/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/drivers/i2c.h>

#define DEV_ADDR   (0x4e)
#define START_ADDR (0x00)
#define RESOLUTION (0.0625f)
#define BIT_SHIFT  (4)

int get_temp(const struct device *dev, float *temp)
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
