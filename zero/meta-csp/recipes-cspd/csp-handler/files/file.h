/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <csp/csp.h>

struct file_err_reply_telemetry {
	uint8_t telemetry_id;
	uint32_t error_code;
} __attribute__((__packed__));

void file_handler_init(void);
void file_handler(csp_packet_t *packet);
