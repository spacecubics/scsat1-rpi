/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/types.h>
#include <dirent.h>
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
#include <jpeglib.h>
#include <unistd.h>

#include "cspd.h"
#include "yuyv_to_rgb.h"

#define CAMERA_WIDTH  1920
#define CAMERA_HEIGHT 1080
#define JPEG_QUALITY 90
#define MMAP_COUNT    2

#define CAM_FRAME_PATH   "/storageA/photo"
#define CAM_FRAME_PREFIX "frame"

static int xioctl(int fd, int request, void *arg)
{
	for (;;) {
		int ret = ioctl(fd, request, arg);
		if (ret < 0) {
			if (errno == EINTR) {
				continue;
			}
			return -errno;
		}
		break;
	}

	return 0;
}

static void get_next_filename(char *filename, size_t size)
{
	unsigned num = 0;
	while (1) {
		snprintf(filename, size, "%s/frame-%03d.jpg", CAM_FRAME_PATH, num);
		if (access(filename, F_OK) != 0) {
			return;
		}
		num++;
	}
}

static void jpeg_write_file(uint8_t *prgb, int width, int height)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	char filename[256];
	get_next_filename(filename, sizeof(filename));

	FILE *fp = fopen(filename, "wb");
	if (!fp) {
		perror("fopen");
		return;
	}

	jpeg_stdio_dest(&cinfo, fp);

	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = 3; // RGB
	cinfo.in_color_space = JCS_RGB;

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, JPEG_QUALITY, TRUE);
	jpeg_start_compress(&cinfo, TRUE);

	for (int y = 0; y < height; y++) {
		JSAMPROW row_pointer[1];               // Pointer to a single row
		row_pointer[0] = &prgb[y * width * 3]; // RGB data
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	fclose(fp);

	jpeg_destroy_compress(&cinfo);
}

// Function to capture images
static int capture_jpeg_frame(const char *camera_dev)
{
	int fd, width, height, length, ret;
	struct v4l2_format fmt;
	struct v4l2_requestbuffers req;
	struct v4l2_buffer buf;
	enum v4l2_buf_type type;
	void *mmap_p[MMAP_COUNT];
	__u32 mmap_l[MMAP_COUNT];
	uint8_t *yuyvbuf, *rgbbuf;

	fd = open(camera_dev, O_RDWR, 0);
	if (fd < 0) {
		perror("open");
		return -1;
	}

	memset(&fmt, 0, sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = CAMERA_WIDTH;
	fmt.fmt.pix.height = CAMERA_HEIGHT;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	ret = xioctl(fd, VIDIOC_S_FMT, &fmt);
	if (ret < 0 || fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_YUYV || fmt.fmt.pix.width <= 0 ||
	    fmt.fmt.pix.height <= 0) {
		perror("ioctl(VIDIOC_S_FMT)");
		return -1;
	}
	width = fmt.fmt.pix.width;
	height = fmt.fmt.pix.height;
	length = width * height;

	yuyvbuf = malloc(2 * length);
	if (!yuyvbuf) {
		perror("malloc");
		return -1;
	}

	memset(&req, 0, sizeof(req));
	req.count = MMAP_COUNT;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	ret = xioctl(fd, VIDIOC_REQBUFS, &req);
	if (ret < 0) {
		perror("ioctl(VIDIOC_REQBUFS)");
		return -1;
	}

	for (unsigned i = 0; i < req.count; i++) {
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		ret = xioctl(fd, VIDIOC_QUERYBUF, &buf);
		if (ret < 0) {
			perror("ioctl(VIDIOC_QUERYBUF)");
			return -1;
		}

		mmap_p[i] = mmap(NULL, buf.length, PROT_READ, MAP_SHARED, fd, buf.m.offset);
		if (mmap_p[i] == MAP_FAILED) {
			perror("mmap");
			return -1;
		}
		mmap_l[i] = buf.length;
	}

	for (unsigned i = 0; i < req.count; i++) {
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		ret = xioctl(fd, VIDIOC_QBUF, &buf);
		if (ret < 0) {
			perror("ioctl(VIDIOC_QBUF)");
			return -1;
		}
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = xioctl(fd, VIDIOC_STREAMON, &type);
	if (ret < 0) {
		perror("ioctl(VIDIOC_STREAMON)");
		return -1;
	}

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	while (1) {
		ret = select(fd + 1, &fds, NULL, NULL, NULL);
		if (ret < 0 && errno != EINTR) {
			perror("select");
			return -1;
		}
		if (FD_ISSET(fd, &fds)) {
			memset(&buf, 0, sizeof(buf));
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			ret = xioctl(fd, VIDIOC_DQBUF, &buf);
			if (ret < 0 || buf.bytesused < (__u32)(2 * length)) {
				perror("ioctl(VIDIOC_DQBUF)");
				return -1;
			}
			memcpy(yuyvbuf, mmap_p[buf.index], 2 * length);
			ret = xioctl(fd, VIDIOC_QBUF, &buf);
			if (ret < 0) {
				perror("ioctl(VIDIOC_QBUF)");
				return -1;
			}
			break;
		}
	}

	xioctl(fd, VIDIOC_STREAMOFF, &type);
	for (unsigned i = 0; i < req.count; i++) {
		munmap(mmap_p[i], mmap_l[i]);
	}
	close(fd);

	rgbbuf = malloc(3 * length);
	if (!rgbbuf) {
		perror("malloc");
		return -1;
	}

	yuyv_to_rgb(yuyvbuf, rgbbuf, length);

	jpeg_write_file(rgbbuf, width, height);

	free(yuyvbuf);
	free(rgbbuf);
	return 0;
}

int camera_jpeg(const char *camera_dev)
{
	if (capture_jpeg_frame(camera_dev) < 0) {
		csp_print("Image capture failed!\n");
		return 1;
	}
	return 0;
}

static int init_photo_dir(void)
{
	DIR *dir;
	struct dirent *entry;
	int ret = 0;
	char fname[128];

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
	static uint16_t seq = 1;
	char fname[128];
	char cam_cmd[256];
	int ret;

	snprintf(fname, sizeof(fname), "%s/frame-%03d.bin", CAM_FRAME_PATH, seq);
	snprintf(cam_cmd, sizeof(cam_cmd), "cam -c 1 --capture=1 --file=%s &",
		 fname);

	ret = system(cam_cmd);

	if (ret == 0) {
		seq++;
	}

	return ret;
}

static int get_frame_file_count(uint16_t *count)
{
	DIR *dir;
	struct dirent *entry;
	int ret = 0;

	dir = opendir(CAM_FRAME_PATH);
	if (dir == NULL) {
		csp_print("Failed to open dir %s\n", CAM_FRAME_PATH);
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

void init_photo_dir_service(csp_conn_t *conn)
{
	(void)init_photo_dir();
}

void capture_frame_service(csp_conn_t *conn)
{
	(void)capture_frame();
}

void capture_jpeg_service(csp_conn_t *conn)
{
	(void)camera_jpeg("/dev/video0");
}

void get_frame_count_service(csp_conn_t *conn)
{
	int ret;
	uint16_t count = 0;
	csp_packet_t *send_packet = csp_buffer_get(0);

	(void)get_frame_file_count(&count);
	memcpy(send_packet->data, &count, sizeof(count));
	send_packet->length = sizeof(count);
	csp_send(conn, send_packet);
}
