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
#include "cspd.h"
#include "utils.h"
#include "camera/capture_raw.h"

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
	return captureRawImage();
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
