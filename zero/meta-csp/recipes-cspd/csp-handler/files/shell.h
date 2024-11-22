/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <csp/csp.h>

#define SHELL_RESULT_BUF_SIZE (240U)

struct shell_err_reply_telemetry {
	uint8_t telemetry_id;
	uint32_t error_code;
} __attribute__((__packed__));

struct shell_cmd_reply_telemetry {
	uint8_t telemetry_id;
	uint32_t error_code;
	uint32_t seq_number;
	uint8_t result[SHELL_RESULT_BUF_SIZE];
} __attribute__((__packed__));

void shell_handler_init(void);
void shell_handler(csp_packet_t *packet);
