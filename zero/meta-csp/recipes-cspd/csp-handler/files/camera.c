/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "camera.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <systemd/sd-journal.h>
#include <linux/videodev2.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "cspd.h"
#include "utils.h"

#define CAMERA_WIDTH  1920
#define CAMERA_HEIGHT 1080

#define CAM_FRAME_PATH      "/storageA/photo"
#define CAM_FRAME_PREFIX    "frame"
#define CAM_FRAME_EXTENSION "sgrbg10"
#define CAM_PATH            "/dev/video0"
#define MAX_FILENAME_LENGTH 256

static void get_next_filename(char *filename, size_t size, const char *extension)
{
	unsigned max_num = 0;
	struct dirent *entry;
	DIR *dp = opendir(CAM_FRAME_PATH);
	if (!dp) {
		perror("opendir");
		return;
	}

	while ((entry = readdir(dp)) != NULL) {
		unsigned num;
		if (sscanf(entry->d_name, CAM_FRAME_PREFIX "-%u.%*s", &num) == 1) {
			if (num > max_num) {
				max_num = num;
			}
		}
	}
	closedir(dp);

	snprintf(filename, size, "%s/" CAM_FRAME_PREFIX "-%03u.%s", CAM_FRAME_PATH, max_num + 1,
		 extension);
}

static void save_raw_file(uint8_t *buffer, int length)
{
	char filename[MAX_FILENAME_LENGTH];
	get_next_filename(filename, sizeof(filename), CAM_FRAME_EXTENSION);
	FILE *fp = fopen(filename, "wb");
	if (fp) {
		fwrite(buffer, 1, length, fp);
		fclose(fp);
	}
}

static int capture_raw_frame(const char *camera_dev)
{
	int fd = open(camera_dev, O_RDWR);
	if (fd < 0) {
		perror("open");
		return EXIT_FAILURE;
	}

	struct v4l2_format fmt;
	memset(&fmt, 0, sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = CAMERA_WIDTH;
	fmt.fmt.pix.height = CAMERA_HEIGHT;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_SGRBG10;
	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

	if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
		perror("VIDIOC_S_FMT");
		close(fd);
		return EXIT_FAILURE;
	}

	struct v4l2_buffer buf;
	struct v4l2_requestbuffers req;
	uint8_t *buffer;

	memset(&req, 0, sizeof(req));
	req.count = 1;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
		perror("VIDIOC_REQBUFS");
		exit(EXIT_FAILURE);
	}

	memset(&buf, 0, sizeof(buf));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.index = 0;

	if (ioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) {
		perror("VIDIOC_QUERYBUF");
		exit(EXIT_FAILURE);
	}

	buffer = (uint8_t *)mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
				 buf.m.offset);
	if (buffer == MAP_FAILED) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}

	if (ioctl(fd, VIDIOC_STREAMON, &buf.type) < 0) {
		perror("VIDIOC_STREAMON");
		exit(EXIT_FAILURE);
	}

	if (ioctl(fd, VIDIOC_QBUF, &buf) < 0) {
		perror("VIDIOC_QBUF");
		exit(EXIT_FAILURE);
	}

	if (ioctl(fd, VIDIOC_DQBUF, &buf) < 0) {
		perror("VIDIOC_DQBUF");
		exit(EXIT_FAILURE);
	}

	save_raw_file(buffer, buf.bytesused);

	munmap(buffer, buf.length);
	ioctl(fd, VIDIOC_STREAMOFF, &buf.type);

	close(fd);
	return EXIT_SUCCESS;
}

static int init_photo_dir(void)
{
	DIR *dir;
	struct dirent *entry;
	int ret = 0;
	char fname[PATH_MAX];

	dir = opendir(CAM_FRAME_PATH);
	if (dir == NULL) {
		ret = mkdir(CAM_FRAME_PATH, 0644);
		goto end;
	}

	while (true) {
		entry = readdir(dir);
		if (entry == NULL) {
			break;
		} else if (strncmp(entry->d_name, CAM_FRAME_PREFIX, strlen(CAM_FRAME_PREFIX)) ==
			   0) {
			snprintf(fname, sizeof(fname), "%s/%s", CAM_FRAME_PATH, entry->d_name);
			remove(fname);
		}
	}

	closedir(dir);

end:
	return ret;
}

static int capture_frame(void)
{
	return capture_raw_frame(CAM_PATH);
}

static int get_frame_file_count(uint16_t *count)
{
	DIR *dir;
	struct dirent *entry;
	int ret = 0;

	dir = opendir(CAM_FRAME_PATH);
	if (dir == NULL) {
		sd_journal_print(LOG_ERR, "Failed to open dir %s\n", CAM_FRAME_PATH);
		ret = -1;
		goto end;
	}

	while (true) {
		entry = readdir(dir);
		if (entry == NULL) {
			break;
		} else if (strncmp(entry->d_name, CAM_FRAME_PREFIX, strlen(CAM_FRAME_PREFIX)) ==
			   0) {
			(*count)++;
		}
	}

	closedir(dir);

end:
	return ret;
}

void init_photo_dir_service(uint8_t command_id, csp_packet_t *packet)
{
	ARG_UNUSED(command_id);
	ARG_UNUSED(packet);
	(void)init_photo_dir();
}

void capture_frame_service(uint8_t command_id, csp_packet_t *packet)
{
	ARG_UNUSED(command_id);
	ARG_UNUSED(packet);
	(void)capture_frame();
}

void get_frame_count_service(uint8_t command_id, csp_packet_t *packet)
{
	uint16_t count = 0;
	struct hwtest_cam_telemetry tlm;

	(void)get_frame_file_count(&count);

	tlm.telemetry_id = command_id;
	tlm.frame_count = count;
	memcpy(packet->data, &tlm, sizeof(tlm));
	csp_sendto_reply(packet, packet, CSP_O_SAME);
}
