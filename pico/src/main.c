/*
 * Copyright (c) 2023 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <stdio.h>

#define DEV_ADDR   (0x4e)
#define START_ADDR (0x00)
#define RESOLUTION (0.0625)
#define BIT_SHIFT  (4)

static int get_temp(const struct device *dev, float *temp)
{
	int ret;
	uint8_t tmp[2];

	if (temp == NULL) {
		return -EINVAL;
	}

	ret = i2c_burst_read(dev, DEV_ADDR, START_ADDR, tmp, sizeof(tmp));
	if (ret == 0) {
		tmp[1] = tmp[1] >> BIT_SHIFT;
		*temp = (int8_t)tmp[0] + (float)tmp[1] * RESOLUTION;
	}

	return ret;
}

int main(void)
{
	int ret;
	float temp;
	const struct device *i2c = DEVICE_DT_GET(DT_NODELABEL(i2c0));
	const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}

	if (!device_is_ready(i2c)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}

	while (true) {
		ret = get_temp(i2c, &temp);
		if (ret == 0) {
			printf("Temp: %f\n", temp);
		} else {
			printf("Temp error(%d)\n", ret);
		}
		(void)gpio_pin_toggle_dt(&led);
		k_sleep(K_SECONDS(1));
	}

	return 0;
}
