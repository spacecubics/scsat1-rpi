/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>

#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, CONFIG_MAIN_LOG_LEVEL);

#define CONSOLE_DEVICE DEVICE_DT_GET(DT_CHOSEN(zephyr_console))

int main(void)
{
	const struct device *dev = CONSOLE_DEVICE;

	/* enable only has DTR line */
	if (DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart)) {
		uint32_t dtr = 0;
		while (!dtr) {
			int status = uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
			if (status != 0) {
				/* if returing not 0 then "API is not enable" or
				 * "this function is not implemented"
				 */
				LOG_ERR("Failed to get DTR : %s", strerror(errno));
			}
			k_sleep(K_MSEC(100));
		}
	}

	LOG_INF("application started.");

	while (1) {
		LOG_INF("Log Hello!");
		printk("Hello World!\n");
		k_sleep(K_MSEC(1000));
	}

	return 0;
}
