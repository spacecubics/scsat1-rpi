#include "cspd.h"

/* uint8_t csp_dbg_packet_print = 1; */

extern csp_conf_t csp_conf;

int main()
{
	csp_conf.version = 1;
	csp_init();

	start_csp_router();
	handle_csp_packet(NULL);

	return 0;
}
