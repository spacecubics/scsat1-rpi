/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "cspd.h"
#include "utils.h"

#define CAM_FRAME_PATH   "/storageA/photo"
#define CAM_FRAME_PREFIX "frame"

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
	ARG_UNUSED(conn);
	(void)init_photo_dir();
}

void capture_frame_service(csp_conn_t *conn)
{
	ARG_UNUSED(conn);
	(void)capture_frame();
}

void get_frame_count_service(csp_conn_t *conn)
{
	uint16_t count = 0;
	csp_packet_t *send_packet = csp_buffer_get(0);

	(void)get_frame_file_count(&count);
	memcpy(send_packet->data, &count, sizeof(count));
	send_packet->length = sizeof(count);
	csp_send(conn, send_packet);
}
