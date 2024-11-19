/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "handler.h"

#include <csp/drivers/can_socketcan.h>
#include <systemd/sd-journal.h>
#include "cspd.h"
#include "utils.h"
#include "hwtest.h"
#include "file.h"

void *handle_csp_packet(void *param)
{
	ARG_UNUSED(param);
	csp_socket_t sock = {0};
	csp_bind(&sock, CSP_ANY);
	csp_listen(&sock, 10);

	while (1) {
		csp_conn_t *conn;
		if ((conn = csp_accept(&sock, 10000)) == NULL) {
			continue;
		}

		csp_packet_t *packet;
		while ((packet = csp_read(conn, 50)) != NULL) {
			switch (csp_conn_dport(conn)) {
			case PORT_HWTEST:
				hwtest_handler(packet);
				break;
			case PORT_FILE:
				file_handler(packet);
				break;
			default:
				csp_service_handler(packet);
				break;
			}
		}
		csp_close(conn);
	}
}
