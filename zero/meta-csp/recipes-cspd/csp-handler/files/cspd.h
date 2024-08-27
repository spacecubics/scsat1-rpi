/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <csp/csp.h>

#define PORT_A (10U) /* for csp_server_client */
#define PORT_T (11U) /* for get temperature */
#define PORT_I (12U) /* for init photo dir */
#define PORT_C (13U) /* for capture frame */
#define PORT_F (14U) /* for get frame count */

#ifndef MAIN_OBC_CAN_ADDR
#define MAIN_OBC_CAN_ADDR (1U)
#endif /* MAIN_OBC_CAN_ADDR */

#ifndef RPI_ZERO_CAN_ADDR
#define RPI_ZERO_CAN_ADDR (2U)
#endif /* RPI_ZERO_CAN_ADDR */

#ifndef RPI_ZERO_UART_ADDR
#define RPI_ZERO_UART_ADDR (3U)
#endif /* RPI_ZERO_UART_ADDR */

#ifndef RPI_PICO_UART_ADDR
#define RPI_PICO_UART_ADDR (4U)
#endif /* RPI_PICO_UART_ADDR */

/* camera.c */
void init_photo_dir_service(csp_conn_t *conn);
void capture_frame_service(csp_conn_t *conn);
void get_frame_count_service(csp_conn_t *conn);
