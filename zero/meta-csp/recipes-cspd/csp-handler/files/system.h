/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <csp/csp.h>

#define SYSTEM_VERSION_STR_SIZE (16U)

struct system_err_telemetry {
	uint8_t telemetry_id;
	uint32_t error_code;
} __attribute__((__packed__));

struct system_version_telemetry {
	uint8_t telemetry_id;
	uint32_t error_code;
	char version[SYSTEM_VERSION_STR_SIZE];
} __attribute__((__packed__));

void system_handler(csp_packet_t *packet);
