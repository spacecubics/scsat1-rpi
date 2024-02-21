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
	LOG_INF("application started.");

	while (1) {
		LOG_INF("Log Hello!");
		printk("Hello World!\n");
		k_sleep(K_MSEC(1000));
	}

	return 0;
}
