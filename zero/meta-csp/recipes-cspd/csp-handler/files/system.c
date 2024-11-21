/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0 */

#include "system.h"

#include <systemd/sd-journal.h>
#include <errno.h>
#include "cspd.h"
#include "version.h"

/* Command size */
#define SYSTEM_CMD_MIN_SIZE (1U)

/* Command ID */
#define SYSTEM_GET_VER_CMD (0U)

static void send_system_err_reply(csp_packet_t *packet, uint8_t command_id, int err_code)
{
	struct system_err_telemetry tlm;

	tlm.telemetry_id = command_id;
	tlm.error_code = htole32(err_code);

	memcpy(packet->data, &tlm, sizeof(tlm));
	packet->length = sizeof(tlm);

	csp_sendto_reply(packet, packet, CSP_O_SAME);
}

static void send_system_version_reply(csp_packet_t *packet, uint8_t command_id, int err_code)
{
	struct system_version_telemetry tlm;

	tlm.telemetry_id = command_id;
	tlm.error_code = htole32(err_code);
	strcpy(tlm.version, CSPD_VERSION);

	memcpy(packet->data, &tlm, sizeof(tlm));
	packet->length = sizeof(tlm);

	csp_sendto_reply(packet, packet, CSP_O_SAME);
}

void system_handler(csp_packet_t *packet)
{
	int ret = 0;
	uint8_t command_id;

	if (packet == NULL) {
		command_id = CSP_UNKNOWN_COMMAND_ID;
		goto end;
	}

	if (packet->length < SYSTEM_CMD_MIN_SIZE) {
		sd_journal_print(LOG_ERR, "Invalide command size: %d", packet->length);
		ret = -EINVAL;
		command_id = CSP_UNKNOWN_COMMAND_ID;
		goto reply;
	}

	command_id = packet->data[CSP_COMMAND_ID_OFFSET];

	switch (command_id) {
	case SYSTEM_GET_VER_CMD:
		send_system_version_reply(packet, command_id, 0);
		break;
	default:
		sd_journal_print(LOG_ERR, "Unkown command code: %d", command_id);
		ret = -EINVAL;
		command_id = CSP_UNKNOWN_COMMAND_ID;
		break;
	}

reply:
	if (ret < 0) {
		send_system_err_reply(packet, command_id, ret);
	}

end:
	return;
}
