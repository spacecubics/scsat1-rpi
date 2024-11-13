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
LOG_MODULE_REGISTER(csp, CONFIG_MAIN_LOG_LEVEL);

#define ROUTER_STACK_SIZE (256U)
#define SERVER_STACK_SIZE (1024U)
#define ROUTER_PRIO       (0U)
#define SERVER_PRIO       (0U)

#define PICO_UART0_DEV      "uart@40034000"
#define PICO_UART0_BAUDRATE (115200U)
#define PICO_UART0_DATABIT  (8U)
#define PICO_UART0_STOPBITS (1U)
#define PICO_UART0_PARITY   (0U)
#define PICO_CSP_ADDR       (26U)

extern csp_conf_t csp_conf;

static void server(void);

static void *router_task(void *param)
{
	while (true) {
		csp_route_work();
	}

	return NULL;
}

static void *server_task(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	server();

	return NULL;
}

K_THREAD_DEFINE(router_id, ROUTER_STACK_SIZE, router_task, NULL, NULL, NULL, ROUTER_PRIO, 0,
		K_TICKS_FOREVER);
K_THREAD_DEFINE(server_id, SERVER_STACK_SIZE, server_task, NULL, NULL, NULL, SERVER_PRIO, 0,
		K_TICKS_FOREVER);

static void router_start(void)
{
	k_thread_start(router_id);
}

static void server_start(void)
{
	k_thread_start(server_id);
}

static void server(void)
{
	csp_conn_t *conn;
	csp_socket_t sock = {0};

	LOG_INF("Server task started");

	csp_bind(&sock, CSP_ANY);

	csp_listen(&sock, 10);

	while (true) {
		if ((conn = csp_accept(&sock, 10000)) == NULL) {
			continue;
		}

		csp_packet_t *packet;
		while ((packet = csp_read(conn, 50)) != NULL) {
			switch (csp_conn_dport(conn)) {
			default:
				csp_service_handler(packet);
				break;
			}
		}

		csp_close(conn);
	}

	return;
}

int csp_server_start(void)
{
	int ret = 0;
	csp_conf.version = 1;
	const char *kiss_device = PICO_UART0_DEV;
	csp_iface_t *default_iface = NULL;
	int error;

	csp_init();

	router_start();

	csp_usart_conf_t conf = {.device = kiss_device,
				 .baudrate = PICO_UART0_BAUDRATE,
				 .databits = PICO_UART0_DATABIT,
				 .stopbits = PICO_UART0_STOPBITS,
				 .paritysetting = PICO_UART0_PARITY};
	error = csp_usart_open_and_add_kiss_interface(&conf, CSP_IF_KISS_DEFAULT_NAME,
						      PICO_CSP_ADDR, &default_iface);
	if (error != CSP_ERR_NONE) {
		LOG_ERR("failed to add KISS interface [%s], error: %d", kiss_device, error);
		ret = -1;
		goto end;
	}
	default_iface->is_default = 1;

	server_start();

end:
	return ret;
}
