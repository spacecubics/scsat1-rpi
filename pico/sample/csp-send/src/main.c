/*
 * Copyright (c) 2023 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <zephyr/kernel.h>
#include <csp/csp.h>
#include <csp/drivers/usart.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, CONFIG_MAIN_LOG_LEVEL);

extern csp_conf_t csp_conf;

#define SERVER_PORT 10

#define CSP_DST_ADDR 13
#define CSP_SRC_ADDR 14

int main(void)
{
	csp_conf.version = 1;
	LOG_INF("start csp_send application");

	const char *kiss_device = "uart@40034000";

	csp_init();

	csp_iface_t *default_iface = NULL;
	csp_usart_conf_t conf = {.device = kiss_device,
				 .baudrate = 115200,
				 .databits = 8,
				 .stopbits = 1,
				 .paritysetting = 0};
	int error = csp_usart_open_and_add_kiss_interface(&conf, CSP_IF_KISS_DEFAULT_NAME,
							  CSP_SRC_ADDR, &default_iface);
	if (error != CSP_ERR_NONE) {
		LOG_ERR("failed to add KISS interface [%s], error: %d", kiss_device, error);
		exit(1);
	}
	default_iface->is_default = 1;

	LOG_DBG("successful add KISS interface [%s]", kiss_device);

	while (1) {
		csp_conn_t *conn =
			csp_connect(CSP_PRIO_NORM, CSP_DST_ADDR, SERVER_PORT, 1000, CSP_O_NONE);
		if (conn == NULL) {
			LOG_ERR("Connection failed");
			exit(1);
		}

		csp_packet_t *packet = csp_buffer_get(100);
		if (packet == NULL) {
			LOG_ERR("Failed to get CSP buffer");
			exit(1);
		}

		char message[] = "Hello World";
		memcpy(packet->data, message, 12);
		packet->length = (strlen((char *)packet->data) + 1); /* include the 0 termination */

		csp_send(conn, packet);
		csp_close(conn);

		LOG_DBG("message (%s) send to [%s]", packet->data, kiss_device);

		k_sleep(K_MSEC(1000));
	}

	return 0;
}
