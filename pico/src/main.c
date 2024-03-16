/*
 * Copyright (c) 2023 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <stdio.h>

#include "scbus.h"

int main(void)
{
	scbus_init();

	while (true) {
		scbus_verify_sof_fn();
	}

	return 0;
}
