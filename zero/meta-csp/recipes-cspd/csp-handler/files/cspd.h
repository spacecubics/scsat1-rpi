#ifndef CSPD_H
#define CSPD_H

#include <stdio.h>
#include <stdlib.h>
#include <csp/csp.h>

#define PORT_A 10 /* for csp_server_client */

#ifndef ZERO_CSP_ADDR
#define ZERO_CSP_ADDR 1
#endif

#ifndef PICO_CSP_ADDR
#define PICO_CSP_ADDR 2
#endif

/* handler.c */
void *handle_csp_packet(void *param);

/* router.c */
int start_csp_router(void);

#endif /* CSPD_H */
