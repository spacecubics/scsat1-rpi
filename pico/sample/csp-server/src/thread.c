/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <csp/csp.h>

void server(void);

#define ROUTER_STACK_SIZE 256
#define SERVER_STACK_SIZE 1024

#define ROUTER_PRIO 0
#define SERVER_PRIO 0

static void *router_task(void *param)
{
	/* Here there be routing */
	while (1) {
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

void router_start(void)
{
	k_thread_start(router_id);
}

void server_start(void)
{
	k_thread_start(server_id);
}
