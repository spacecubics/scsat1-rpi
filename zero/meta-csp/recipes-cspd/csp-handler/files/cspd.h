#ifndef CSPD_H
#define CSPD_H

#include <stdio.h>
#include <stdlib.h>
#include <csp/csp.h>

#define PORT_A 10 /* for csp_server_client */

#ifndef MAIN_OBC_CAN_ADDR
#define MAIN_OBC_CAN_ADDR 1
#endif /* MAIN_OBC_CAN_ADDR */

#ifndef RPI_ZERO_CAN_ADDR
#define RPI_ZERO_CAN_ADDR 2
#endif /* RPI_ZERO_CAN_ADDR */

#ifndef RPI_ZERO_UART_ADDR
#define RPI_ZERO_UART_ADDR 3
#endif /* RPI_ZERO_UART_ADDR */

#ifndef RPI_PICO_UART_ADDR
#define RPI_PICO_UART_ADDR 4
#endif /* RPI_PICO_UART_ADDR */

/* handler.c */
void *handle_csp_packet(void *param);

/* router.c */
int start_csp_router(void);

#endif /* CSPD_H */
