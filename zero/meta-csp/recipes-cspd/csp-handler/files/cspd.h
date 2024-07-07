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

/* handler.c */
void *handle_csp_packet(void *param);

/* router.c */
int start_csp_router(void);

/* temp.c */
void get_temp_service(csp_conn_t *conn);
