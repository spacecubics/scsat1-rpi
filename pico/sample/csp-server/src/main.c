/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <zephyr/kernel.h>
#include <csp/csp.h>
#include <csp/drivers/usart.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, CONFIG_MAIN_LOG_LEVEL);

#define SERVER_PORT 10

#define CSP_SRC_ADDR 26

extern csp_conf_t csp_conf;

int router_start(void);
int server_start(void);

void server(void)
{
	LOG_INF("Server task started\n");

	/* Create socket with no specific socket options, e.g. accepts CRC32, HMAC, etc. if enabled
	 * during compilation */
	csp_socket_t sock = {0};

	/* Bind socket to all ports, e.g. all incoming connections will be handled here */
	csp_bind(&sock, CSP_ANY);

	/* Create a backlog of 10 connections, i.e. up to 10 new connections can be queued */
	csp_listen(&sock, 10);

	/* Wait for connections and then process packets on the connection */
	while (1) {

		/* Wait for a new connection, 10000 mS timeout */
		csp_conn_t *conn;
		if ((conn = csp_accept(&sock, 10000)) == NULL) {
			/* timeout */
			continue;
		}

		/* Read packets on connection, timout is 100 mS */
		csp_packet_t *packet;
		while ((packet = csp_read(conn, 50)) != NULL) {
			switch (csp_conn_dport(conn)) {
			case SERVER_PORT:
				/* Process packet here */
				LOG_INF("Packet received on MY_SERVER_PORT: %s\n",
					(char *)packet->data);
				csp_buffer_free(packet);
				break;

			default:
				/* Call the default CSP service handler, handle pings, buffer use,
				 * etc. */
				csp_service_handler(packet);
				break;
			}
		}

		/* Close current connection */
		csp_close(conn);
	}

	return;
}

int main(void)
{
	csp_conf.version = 1;
	const char *kiss_device = "uart@40034000";

	csp_init();
	router_start();

	csp_iface_t *default_iface = NULL;
	csp_usart_conf_t conf = {.device = kiss_device,
				 .baudrate = 115200,
				 .databits = 8,
				 .stopbits = 1,
				 .paritysetting = 0};
	int error = csp_usart_open_and_add_kiss_interface(&conf, CSP_IF_KISS_DEFAULT_NAME,
							  CSP_SRC_ADDR, &default_iface);
	if (error != CSP_ERR_NONE) {
		LOG_ERR("failed to add KISS interface [%s], error: %d\n", kiss_device, error);
		exit(1);
	}
	default_iface->is_default = 1;

	server_start();

	while (1) {
		k_sleep(K_MSEC(10));
	}

	return 0;
}
