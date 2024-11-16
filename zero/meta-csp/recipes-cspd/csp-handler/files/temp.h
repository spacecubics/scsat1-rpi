/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <csp/csp.h>

struct hwtest_temp_telemetry {
	uint8_t telemetry_id;
	uint16_t temp_raw;
} __attribute__((__packed__));

void get_temp_service(uint8_t command_id, csp_packet_t *packet);
