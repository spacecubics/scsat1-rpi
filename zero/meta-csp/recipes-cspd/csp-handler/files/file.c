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
#include <sys/stat.h>
#include "cspd.h"
#include "utils.h"
#include "upload.h"

#define FILE_WORK_THREAD_SLEEP_USEC (10000U)
#define UPLOAD_MAX_SESSION          (5U)

/* Command size */
#define FILE_CMD_MIN_SIZE (1U)

/* Command ID */
#define FILE_UPLOAD_OPEN_CMD (2U)

static GQueue file_work_queue;
static pthread_t file_work;

static void csp_file_work(csp_packet_t *packet)
{
	int ret = 0;
	uint8_t command_id;

	if (packet == NULL) {
		ret = -EINVAL;
		command_id = CSP_UNKNOWN_COMMAND_ID;
		goto end;
	}

	if (packet->length < FILE_CMD_MIN_SIZE) {
		sd_journal_print(LOG_ERR, "Invalide command size: %d", packet->length);
		ret = -EINVAL;
		command_id = CSP_UNKNOWN_COMMAND_ID;
		goto free;
	}

	command_id = packet->data[CSP_COMMAND_ID_OFFSET];

	switch (command_id) {
	case FILE_UPLOAD_OPEN_CMD:
		file_upload_open_cmd(command_id, packet);
		break;
	default:
		sd_journal_print(LOG_ERR, "Unkown command code: %d", command_id);
		ret = -EINVAL;
		command_id = CSP_UNKNOWN_COMMAND_ID;
		break;
	}

free:
	if (ret < 0) {
	}

end:
	return;
}

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
		csp_file_work(packet);
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

	upload_handler_init();
}
