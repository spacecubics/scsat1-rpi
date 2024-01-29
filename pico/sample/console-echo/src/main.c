/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <errno.h>

#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, CONFIG_MAIN_LOG_LEVEL);

#define CONSOLE_DEVICE DEVICE_DT_GET(DT_CHOSEN(zephyr_shell_uart))
const struct device *uart_dev = CONSOLE_DEVICE;

/* message configuration */
#define MSG_SIZE 32
K_MSGQ_DEFINE(uart_msgq, MSG_SIZE, 10, 4); /* message queue (up to 10 messages) */

/* recieve buffer */
static char rx_buf[MSG_SIZE];
static int rx_buf_pos;

/* call when there is an interruption. */
void serial_callback_handler(const struct device *dev, void *user_data)
{
	int ret = uart_irq_update(uart_dev);
	if (ret < 0) {
		LOG_ERR("can't start interrupt : %d", ret);
		return;
	}
	/* chack rx buffer */
	ret = uart_irq_rx_ready(uart_dev);
	if (ret < 0) {
		return;
	}
	uint8_t c;
	/* read until FIFO empty */
	while (uart_fifo_read(uart_dev, &c, 1) == 1) {
		if ((c == '\n' || c == '\r') && rx_buf_pos > 0) {
			/* terminate string */
			rx_buf[rx_buf_pos] = '\0';

			/* if queue is full, message is silently dropped */
			k_msgq_put(&uart_msgq, &rx_buf, K_NO_WAIT);

			/* reset the buffer (it was copied to the msgq) */
			rx_buf_pos = 0;
		} else if (rx_buf_pos < (sizeof(rx_buf) - 1)) {
			rx_buf[rx_buf_pos++] = c;
		}
		/* else: characters beyond buffer size are dropped */
	}
}

void print_uart(char *buf)
{
	LOG_INF("call print_uart : \"%s\"", buf);
	int msg_len = strlen(buf);

	for (int i = 0; i < msg_len; i++) {
		uart_poll_out(uart_dev, buf[i]);
	}
}

int main(void)
{
	/* enable only has DTR line e.g. USB */
	if (DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart)) {
		uint32_t dtr = 0;
		while (!dtr) {
			int status = uart_line_ctrl_get(uart_dev, UART_LINE_CTRL_DTR, &dtr);
			if (status != 0) {
				/* if returing not 0 then "API is not enable" or
				 * "this function is not implemented"
				 */
				LOG_ERR("Failed to get DTR : %s", strerror(errno));
			}
			k_sleep(K_MSEC(100));
		}
	}

	if (!device_is_ready(uart_dev)) {
		LOG_ERR("UART device not found!");
		return 0;
	}

	/* configure uart IRQ (interrupt request) handler
	 * ref.
	 *   https://docs.zephyrproject.org/latest/hardware/peripherals/uart.html#c.uart_irq_callback_user_data_set
	 */

	int ret = uart_irq_callback_user_data_set(uart_dev, serial_callback_handler, NULL);
	if (ret != 0) {
		switch (ret) {
		case -ENOTSUP:
			LOG_ERR("Interrupt-driven UART API support not enabled");
			return 0;
		case -ENOSYS:
			LOG_ERR("UART device does not support interrupt-driven API");
			return 0;
		default:
			LOG_ERR("Error setting UART callback: %d", ret);
			return 0;
		}
	}
	/* enable rx IRQ */
	uart_irq_rx_enable(uart_dev);

	char tx_buf[MSG_SIZE];

	LOG_INF("application started.");
	print_uart("Tell me something and press enter:\r\n");

	while (k_msgq_get(&uart_msgq, &tx_buf, K_FOREVER) == 0) {
		print_uart("Echo: ");
		print_uart(tx_buf);
		print_uart("\r\n");
	}

	return 0;
}
