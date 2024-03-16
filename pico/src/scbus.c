/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(scbus);

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
