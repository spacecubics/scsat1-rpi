/* * Copyright (c) 2024 Space Cubics, LLC.  *
 * SPDX-License-Identifier: Apache-2.0 */

#include "shell.h"

#include <glib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <systemd/sd-journal.h>
#include <sys/stat.h>
#include <csp/csp_buffer.h>
#include "cspd.h"
#include "utils.h"
#include "upload.h"

#define SHELL_WORK_THREAD_SLEEP_SEC  (1U)
#define SHELL_CMD_REPLY_INTERVAL_SEC (0.2)
#define SHELL_CMD_STR_MAX_SIZE       (200U)
#define SHELL_CMD_REPLY_TLM_MAX_CNT  (50U)

/* Command size */
#define SHELL_CMD_MIN_SIZE      (1U)
#define SHELL_EXEC_CMD_MIN_SIZE (3U) + SHELL_CMD_STR_MAX_SIZE

/* Command ID */
#define SHELL_CMD (0U)

/* Command argument offset */
#define SHELL_CMD_TIMEOUT_OFFSET (1U)
#define SHELL_CMD_STR_OFFSET     (3U)

static GQueue shell_work_queue;
static pthread_t shell_work;

void send_cmd_reply(csp_packet_t *packet, uint8_t command_id, int err_code, uint8_t *result)
{
	struct shell_cmd_reply_telemetry tlm;
	csp_packet_t *clone;

	/*
	 * This reply telemetry might be divided and sent in multiple parts. Since using
	 * `send_cmd_reply()` would release the packet, so clone it before sending.
	 */
	clone = csp_buffer_clone(packet);

	tlm.telemetry_id = command_id;
	tlm.error_code = err_code;
	if (result == NULL) {
		memset(tlm.result, 0, SHELL_RESULT_BUF_SIZE);
	} else {
		memcpy(tlm.result, result, SHELL_RESULT_BUF_SIZE);
	}

	memcpy(clone->data, &tlm, sizeof(tlm));
	clone->length = sizeof(tlm);

	csp_sendto_reply(clone, clone, CSP_O_SAME);
}

void send_shell_err_reply(csp_packet_t *packet, uint8_t command_id, int err_code)
{
	struct shell_err_reply_telemetry tlm;

	tlm.telemetry_id = command_id;
	tlm.error_code = htole32(err_code);

	memcpy(packet->data, &tlm, sizeof(tlm));
	packet->length = sizeof(tlm);

	csp_sendto_reply(packet, packet, CSP_O_SAME);
}

static void shell_cmd(uint8_t command_id, csp_packet_t *packet)
{
	int ret;
	FILE *fp;
	char cmd[SHELL_CMD_STR_MAX_SIZE];
	char result[SHELL_RESULT_BUF_SIZE] = {0};
	uint16_t timeout_sec;
	struct timeval timeout;
	fd_set readfds;
	int fd;
	int nfds;
	size_t rsize;
	uint8_t reply_count = 0;

	if (packet->length != SHELL_EXEC_CMD_MIN_SIZE) {
		sd_journal_print(LOG_ERR, "Invalide command size: %d", packet->length);
		ret = -EINVAL;
		goto end;
	}

	strcpy(cmd, (const char *)&packet->data[SHELL_CMD_STR_OFFSET]);
	timeout_sec = le16toh(*(unsigned short *)&packet->data[SHELL_CMD_TIMEOUT_OFFSET]);

	sd_journal_print(LOG_INFO, "Shell command: %s (timeout: %d sec)", cmd, timeout_sec);

	fp = popen(cmd, "r");
	if (fp == NULL) {
		sd_journal_print(LOG_ERR, "Unable to popen for shell command");
		ret = -EIO;
		goto end;
	}

	fd = fileno(fp);
	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);
	nfds = fd + 1;

	timeout.tv_sec = timeout_sec;
	timeout.tv_usec = 0;

	ret = select(nfds, &readfds, NULL, NULL, &timeout);
	if (ret < 0) {
		sd_journal_print(LOG_ERR, "Unable to select (%d)", ret);
		goto close;
	} else if (ret == 0) {
		sd_journal_print(LOG_ERR, "Timeout occurred");
		ret = -ETIMEDOUT;
		goto close;
	}

	while (reply_count < SHELL_CMD_REPLY_TLM_MAX_CNT) {
		rsize = fread(result, 1, sizeof(result), fp);
		if (rsize <= 0) {
			if (reply_count == 0) {
				strcpy(result, "No output");
				send_cmd_reply(packet, command_id, rsize, (uint8_t *)result);
				sd_journal_print(LOG_DEBUG, "Send reply (No output)");
			}
			break;
		}
		send_cmd_reply(packet, command_id, 0, (uint8_t *)result);
		memset(result, 0, sizeof(result));
		reply_count++;
		sd_journal_print(LOG_DEBUG, "Send reply %d times", reply_count);
		sleep(SHELL_CMD_REPLY_INTERVAL_SEC);
	}

close:
	pclose(fp);

end:
	if (ret < 0) {
		send_cmd_reply(packet, command_id, ret, NULL);
	}

	/*
	 * Since the reply uses a clone for the response, the original is released manually.
	 */
	csp_buffer_free(packet);

	return;
}

static void csp_shell_work(csp_packet_t *packet)
{
	int ret = 0;
	uint8_t command_id;

	if (packet == NULL) {
		ret = -EINVAL;
		command_id = CSP_UNKNOWN_COMMAND_ID;
		goto end;
	}

	if (packet->length < SHELL_CMD_MIN_SIZE) {
		sd_journal_print(LOG_ERR, "Invalide command size: %d", packet->length);
		ret = -EINVAL;
		command_id = CSP_UNKNOWN_COMMAND_ID;
		goto reply;
	}

	command_id = packet->data[CSP_COMMAND_ID_OFFSET];

	switch (command_id) {
	case SHELL_CMD:
		shell_cmd(command_id, packet);
		break;
	default:
		sd_journal_print(LOG_ERR, "Unkown command code: %d", command_id);
		ret = -EINVAL;
		command_id = CSP_UNKNOWN_COMMAND_ID;
		break;
	}

reply:
	if (ret < 0) {
		send_shell_err_reply(packet, command_id, ret);
	}

end:
	return;
}

static void *shell_work_thread(void *arg)
{
	csp_packet_t *packet;

	ARG_UNUSED(arg);

	while (true) {
		packet = (csp_packet_t *)g_queue_pop_head(&shell_work_queue);
		if (packet == NULL) {
			sleep(SHELL_WORK_THREAD_SLEEP_SEC);
			continue;
		}
		csp_shell_work(packet);
	}

	pthread_exit(NULL);
}

void shell_handler(csp_packet_t *packet)
{
	g_queue_push_tail(&shell_work_queue, packet);
}

void shell_handler_init(void)
{
	int ret;

	g_queue_init(&shell_work_queue);

	ret = pthread_create(&shell_work, NULL, shell_work_thread, NULL);
	if (ret < 0) {
		sd_journal_print(LOG_ERR, "Unable to create shell work thread");
	} else {
		sd_journal_print(LOG_INFO, "Start the shell work thread");
	}
}
