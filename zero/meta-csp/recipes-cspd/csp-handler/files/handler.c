#include "cspd.h"

#include <csp/drivers/can_socketcan.h>

void *handle_csp_packet(void *param)
{
	uint8_t address = ZERO_CSP_ADDR;

	csp_iface_t *iface;
	if ((iface = csp_can_socketcan_init("can0", address, 1000000, true)) == NULL) {
		fprintf(stderr, "can't initialize socketcan\n");
		exit(-1);
	}
	iface->is_default = 1;

	/* csp_iflist_print(); */

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

			default:
				csp_service_handler(packet);
				break;
			}
		}
		csp_close(conn);
	}
}
