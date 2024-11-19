/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <csp/csp.h>

#define FILE_NAME_MAX_LEN (64U)

struct upload_open_reply_telemetry {
	uint8_t telemetry_id;
	uint32_t error_code;
	uint16_t session_id;
	char file_name[FILE_NAME_MAX_LEN];
} __attribute__((__packed__));

void upload_handler_init(void);
int file_upload_open_cmd(uint8_t command_id, csp_packet_t *packet);
