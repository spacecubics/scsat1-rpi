/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(scbus);

#define SOF_MAX_FN       (0x7FF)
#define SOF_FN_OK        (0U)
#define SOF_FN_DROPPED   (2U)
#define SOF_FN_NOCHANGED (3U)

static inline uint16_t scbus_get_sof_fn(void)
{
	return (sys_read32(0x50110048) & SOF_MAX_FN);
}

static bool verify_increment_sof_fn(uint16_t latest, int16_t prev)
{
	if (prev == -1) {
		/* First received SOF */
		return true;
	}

	if ((prev == SOF_MAX_FN) && (latest == 0)) {
		return true;
	}

	if ((latest - prev) == 1) {
		return true;
	}

	return false;
}

void scbus_init(void)
{
	uint32_t reg;

	/* Clock Enable */
	sys_write32(0x000000C0, 0x400080b4);

	/* USB core reset */
	reg = sys_read32(0x4000c000);
	reg |= 0x01000000;

	sys_write32(reg, 0x4000c000);
	reg &= 0xFEFFFFFF;
	sys_write32(reg, 0x4000c000);

	/* Connect to PHY */
	sys_write32(0x00000009, 0x50110074);

	/* VBUS detect */
	sys_write32(0x0000000C, 0x50110078);

	/* USB Enable */
	sys_write32(0x00000001, 0x50110040);

	/* D+ Pullup */
	sys_write32(0x00010000, 0x5011004C);
};

void scbus_verify_sof_fn(void)
{
	uint16_t latest;
	static uint8_t sof_err = SOF_FN_NOCHANGED;
	static uint8_t sof_drop = 0;
	static int16_t previous = -1;
	static uint32_t last_output_ms = 0;
	static bool output = false;
	uint32_t uptime_ms = k_uptime_get_32();

	latest = scbus_get_sof_fn();
	if (latest == previous) {
		goto check;
	}

	if (verify_increment_sof_fn(latest, previous)) {
		if (sof_err != SOF_FN_DROPPED) {
			sof_err = SOF_FN_OK;
		}
	} else {
		sof_err = SOF_FN_DROPPED;
		sof_drop++;
	}
	previous = latest;

check:
	if (last_output_ms == 0) {
		output = true;
	} else if ((uptime_ms - last_output_ms) >= 1000) {
		output = true;
	}

	if (output) {
		printf("%d,%x,%d\n", sof_err, latest, sof_drop);
		last_output_ms = uptime_ms;
		output = false;
		sof_err = SOF_FN_NOCHANGED;
	}

	return;
}
