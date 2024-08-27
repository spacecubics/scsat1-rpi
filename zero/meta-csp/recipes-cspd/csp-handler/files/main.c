/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <csp/csp_rtable.h>
#include <csp/drivers/can_socketcan.h>
#include <csp/drivers/usart.h>
#include <csp/csp_id.h>
#include "cspd.h"
#include "handler.h"
#include "router.h"

extern csp_conf_t csp_conf;

int main()
{
	csp_conf.version = 1;
	csp_init();

	/* CAN interface */
	csp_iface_t *can_iface = csp_can_socketcan_init("can0", RPI_ZERO_CAN_ADDR, 1000000, true);
	if (can_iface == NULL) {
		csp_print("failed to add CAN interface [can0]");
		exit(1);
	}
	can_iface->is_default = 1;
	can_iface->netmask = csp_id_get_host_bits();

	/* UART interface */
	csp_iface_t *usart_iface;
	csp_usart_conf_t usart_conf = {
		.device = "/dev/ttyS0",
		.baudrate = 115200,
		.databits = 8,
		.stopbits = 1,
		.paritysetting = 0,
	};
	int error = csp_usart_open_and_add_kiss_interface(&usart_conf, CSP_IF_KISS_DEFAULT_NAME,
							  RPI_ZERO_UART_ADDR, &usart_iface);
	if (error != CSP_ERR_NONE) {
		csp_print("failed to add KISS interface [%s], error: %d\n", usart_conf.device,
			  error);
		exit(1);
	}
	usart_iface->netmask = csp_id_get_host_bits();

	csp_rtable_set(RPI_PICO_UART_ADDR, csp_id_get_host_bits(), usart_iface, CSP_NO_VIA_ADDRESS);
	csp_rtable_set(MAIN_OBC_CAN_ADDR, csp_id_get_host_bits(), can_iface, CSP_NO_VIA_ADDRESS);

	start_csp_router();
	handle_csp_packet(NULL);

	return 0;
}
