/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0 */

#include "upload.h"

#include <glib.h>
#include <systemd/sd-journal.h>
#include <sys/stat.h>
#include "cspd.h"
#include "utils.h"

#define UPLOAD_MAX_SESSION (5U)

/* Command size */
#define UPLOAD_OPEN_CMD_SIZE (3U) + FILE_NAME_MAX_LEN

/* Command ID */
#define FILE_UPLOAD_OPEN_CMD (2U)

/* Command argument offset */
#define UPLOAD_SID_OFFSET   (1U)
#define UPLOAD_FNAME_OFFSET (3U)

struct session_entry {
	uint16_t id;
	FILE *file;
	char fname[FILE_NAME_MAX_LEN];
};

static GSList *session_used;
static GSList *session_unused;
static struct session_entry sessions[UPLOAD_MAX_SESSION];

struct session_entry *search_used_session(uint16_t session_id)
{
	struct session_entry *session;

	for (GSList *l = session_used; l != NULL; l = l->next) {
		session = (struct session_entry *)l->data;
		if (session->id == session_id) {
			return session;
		}
	}

	return NULL;
}

struct session_entry *get_unused_session(void)
{
	struct session_entry *session;

	for (GSList *l = session_unused; l != NULL; l = l->next) {
		session = (struct session_entry *)l->data;
		if (session != NULL) {
			session_unused = g_slist_remove(session_unused, session);
			session_used = g_slist_append(session_used, session);
			return session;
		}
	}

	return NULL;
}

static void release_session(struct session_entry *session)
{
	session_used = g_slist_remove(session_used, session);
	session_unused = g_slist_append(session_unused, session);
}

static void send_upload_open_reply(csp_packet_t *packet, uint8_t command_id, int err_code,
				   uint16_t session_id, const char *fname)
{
	struct upload_open_reply_telemetry tlm;

	tlm.telemetry_id = command_id;
	tlm.error_code = htole32(err_code);
	tlm.session_id = htole16(session_id);
	strncpy(tlm.file_name, fname, FILE_NAME_MAX_LEN);
	tlm.file_name[FILE_NAME_MAX_LEN - 1] = '\0';

	memcpy(packet->data, &tlm, sizeof(tlm));
	packet->length = sizeof(tlm);

	csp_sendto_reply(packet, packet, CSP_O_SAME);
}

static int open_upload_file(uint16_t session_id, const char *fname)
{
	int ret = 0;
	struct session_entry *session;

	session = search_used_session(session_id);
	if (session != NULL) {
		sd_journal_print(LOG_ERR, "This session ID is already used in (%s)",
				 session->fname);
		ret = -EEXIST;
		goto end;
	}

	session = get_unused_session();
	if (session == NULL) {
		sd_journal_print(LOG_ERR, "No more sessions are available.");
		ret = -EMFILE;
		goto end;
	}

	session->file = fopen(fname, "r+b");
	if (session->file == NULL) {
		session->file = fopen(fname, "w+b");
	}
	if (session->file == NULL) {
		sd_journal_print(LOG_ERR, "Faild to open the upload file %s", fname);
		release_session(session);
	}

	session->id = session_id;
	strcpy(session->fname, fname);

end:
	return ret;
}

int file_upload_open_cmd(uint8_t command_id, csp_packet_t *packet)
{
	int ret = 0;
	uint16_t session_id = 0;
	char fname[FILE_NAME_MAX_LEN] = {0};

	if (packet->length != UPLOAD_OPEN_CMD_SIZE) {
		sd_journal_print(LOG_ERR, "Invalide command size: %d", packet->length);
		ret = -EINVAL;
		goto end;
	}

	session_id = le16toh(*(unsigned short *)&packet->data[UPLOAD_SID_OFFSET]);
	strcpy(fname, (const char *)&packet->data[UPLOAD_FNAME_OFFSET]);

	sd_journal_print(LOG_INFO, "Upload (OPEN) command (session_id: %d) (fname: %s)", session_id,
			 fname);

	ret = open_upload_file(session_id, fname);

end:
	send_upload_open_reply(packet, command_id, ret, session_id, fname);
	return ret;
}

void upload_handler_init(void)
{
	for (uint8_t i = 0; i < UPLOAD_MAX_SESSION; i++) {
		session_unused = g_slist_append(session_unused, &sessions[i]);
	}
}
