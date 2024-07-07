/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cspd.h"

#include <pthread.h>
#include <errno.h>

static void *router(void *param)
{
	while (1) {
		csp_route_work();
	}
	return NULL;
}

int start_csp_router(void)
{
	pthread_attr_t attr;
	pthread_t handle;

	if (pthread_attr_init(&attr) != 0) {
		perror("pthread_attr_init: ");
		return -1;
	}

	if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
		perror("pthread_attr_setdetachstate: ");
		return -1;
	}

	if (pthread_create(&handle, &attr, router, NULL) != 0) {
		perror("pthread_create: ");
		return -1;
	}

	if (pthread_attr_destroy(&attr) != 0) {
		perror("pthread_attr_destroy: ");
		return -1;
	}

	return 0;
}
