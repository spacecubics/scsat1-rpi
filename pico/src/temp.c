/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "temp.h"

#include <zephyr/drivers/i2c.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(temp, LOG_LEVEL_DBG);

#define DEV_ADDR   (0x4e)
#define START_ADDR (0x00)
#define RESOLUTION (0.0625f)
#define BIT_SHIFT  (4)

int get_temp(float *temp) {

	const struct device *const temp_sensor = DEVICE_DT_GET_ANY(lm75);
	struct sensor_value temp_val;
	int r;
	int rc;

    	if (!device_is_ready(temp_sensor)) {
		LOG_ERR("Temperature sensor device not ready");
		return -ENODEV;
	}

	if (temp == NULL) {
		return -EINVAL;
	}

	rc = sensor_sample_fetch(temp_sensor);
	if ( rc < 0 ) {
		LOG_ERR("Failed to fetch sensor data, error: %d", rc);
		return rc;
	}

	r = sensor_channel_get(temp_sensor, SENSOR_CHAN_AMBIENT_TEMP, &temp_val);
	if ( r < 0 ) {
		LOG_ERR("Failed to get sensor data, error: %d", r);
		return r;
	}

	*temp = (float)(temp_val.val1 + (float)(temp_val.val2) /1000000.0f);
	LOG_DBG("Temperature: %d.%06d°C", temp_val.val1, temp_val.val2);

    	return 0;
}
