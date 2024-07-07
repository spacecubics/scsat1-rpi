/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cspd.h"

#include <csp/drivers/can_socketcan.h>

void *handle_csp_packet(void *param)
{
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
			case PORT_A:
				csp_print("recived: %s\n", (char *)packet->data);
				csp_buffer_free(packet);
				break;

			case PORT_T:
				get_temp_service(conn);
				csp_buffer_free(packet);
				break;

			case PORT_I:
				init_photo_dir_service(conn);
				csp_buffer_free(packet);
				break;

			case PORT_C:
				capture_frame_service(conn);
				csp_buffer_free(packet);
				break;

			case PORT_F:
				get_frame_count_service(conn);
				csp_buffer_free(packet);
				break;

			default:
				csp_service_handler(packet);
				break;
			}
		}
		csp_close(conn);
	}
}
