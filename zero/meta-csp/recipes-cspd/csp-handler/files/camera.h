/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <csp/csp.h>

#define CAMERA_WIDTH  1920
#define CAMERA_HEIGHT 1200
#define CAMERA_DEFAULT_BRIGTHNESS 1.0

#define CAM_FRAME_PATH      "/storageA/photo"
#define CAM_FRAME_PREFIX    "frame"
#define CAM_FRAME_EXTENSION "dng"
#define CAM_FRAME_TIMEOUT   4

struct hwtest_cam_telemetry {
	uint8_t telemetry_id;
	uint16_t frame_count;
} __attribute__((__packed__));

void init_photo_dir_service(uint8_t command_id, csp_packet_t *packet);
void capture_frame_service(uint8_t command_id, csp_packet_t *packet);
void get_frame_count_service(uint8_t command_id, csp_packet_t *packet);
