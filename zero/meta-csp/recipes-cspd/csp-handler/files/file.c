/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0 */

#include "file.h"

#include <glib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <systemd/sd-journal.h>
#include "utils.h"

#define FILE_WORK_THREAD_SLEEP_USEC (10000U)

static GQueue file_work_queue;
static pthread_t file_work;

static void *file_work_thread(void *arg)
{
	csp_packet_t *packet;

	ARG_UNUSED(arg);

	while (true) {
		packet = (csp_packet_t *)g_queue_pop_head(&file_work_queue);
		if (packet == NULL) {
			usleep(FILE_WORK_THREAD_SLEEP_USEC);
			continue;
		}
		csp_buffer_free(packet);
	}

	pthread_exit(NULL);
}

void file_handler(csp_packet_t *packet)
{
	g_queue_push_tail(&file_work_queue, packet);
}

void file_handler_init(void)
{
	int ret;

	g_queue_init(&file_work_queue);

	ret = pthread_create(&file_work, NULL, file_work_thread, NULL);
	if (ret < 0) {
		sd_journal_print(LOG_ERR, "Unable to create file work thread");
	} else {
		sd_journal_print(LOG_INFO, "Start the file work thread");
	}
}
