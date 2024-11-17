/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <csp/csp.h>

#define PORT_HWTEST (11U) /* for HWTEST program */
#define PORT_FILE   (13U) /* for file related command */

#define CSP_COMMAND_ID_OFFSET (1U)

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
