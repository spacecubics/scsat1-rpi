/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <csp/csp.h>

void init_photo_dir_service(csp_conn_t *conn);
void capture_frame_service(csp_conn_t *conn);
void get_frame_count_service(csp_conn_t *conn);
