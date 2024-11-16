/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <csp/csp.h>

struct hwtest_cam_telemetry {
	uint8_t telemetry_id;
	uint16_t frame_count;
} __attribute__((__packed__));

void init_photo_dir_service(uint8_t command_id, csp_packet_t *packet);
void capture_frame_service(uint8_t command_id, csp_packet_t *packet);
void get_frame_count_service(uint8_t command_id, csp_packet_t *packet);
