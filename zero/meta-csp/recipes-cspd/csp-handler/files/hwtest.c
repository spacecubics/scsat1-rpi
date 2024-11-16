/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "hwtest.h"

#include <systemd/sd-journal.h>
#include "cspd.h"
#include "temp.h"
#include "camera.h"

#define HWTEST_CMD_MIN_SIZE (1U)

#define HWTEST_GET_TEMP_CMD        (0U)
#define HWTEST_INIT_PHOTO_DIR_CMD  (1U)
#define HWTEST_CAPTURE_FRAME_CMD   (2U)
#define HWTEST_GET_FRAME_COUNT_CMD (3U)

void hwtest_handler(csp_packet_t *packet)
{
	uint8_t command_id;

	if (packet == NULL) {
		sd_journal_print(LOG_ERR, "Invalid parameter");
		goto end;
	}

	if (packet->length < HWTEST_CMD_MIN_SIZE) {
		sd_journal_print(LOG_ERR, "Invalid command size: %d", packet->length);
		csp_buffer_free(packet);
		goto end;
	}

	command_id = packet->data[CSP_COMMAND_ID_OFFSET];

	switch (command_id) {
	case HWTEST_GET_TEMP_CMD:
		get_temp_service(command_id, packet);
		break;
	case HWTEST_INIT_PHOTO_DIR_CMD:
		init_photo_dir_service(command_id, packet);
		break;
	case HWTEST_CAPTURE_FRAME_CMD:
		capture_frame_service(command_id, packet);
		break;
	case HWTEST_GET_FRAME_COUNT_CMD:
		get_frame_count_service(command_id, packet);
		break;
	default:
		sd_journal_print(LOG_ERR, "Unknown command ID: %d", command_id);
		csp_buffer_free(packet);
		break;
	}

end:
	return;
}
