/*
 * Copyright (c) 2024 Space Cubics, LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <gpiod.h>

#define PICO_RESET_PIN (0U)

#define LINE_MODE_OUTPUT (0U)
#define LINE_MODE_INPUT  (1U)

#define LINE_OUTPUT_LOW  (0U)
#define LINE_OUTPUT_HIGH (1U)

#define PICO_RESET_APP_NAME     "pico_reset"
#define PICO_RESET_CHIP_NAME    "gpiochip0"
#define PICO_RESET_LOW_DURATION (1U)

int main(void)
{
	int ret;
	struct gpiod_chip *chip;
	struct gpiod_line *line;

	chip = gpiod_chip_open_by_name(PICO_RESET_CHIP_NAME);
	if (!chip) {
		perror("Failed to open a gpiochip");
		ret = EXIT_FAILURE;
		goto end;
	}

	line = gpiod_chip_get_line(chip, PICO_RESET_PIN);
	if (!line) {
		perror("Failed to get the handle to the GPIO line");
		ret = EXIT_FAILURE;
		goto close;
	}

	ret = gpiod_line_request_output(line, PICO_RESET_APP_NAME, LINE_MODE_OUTPUT);
	if (ret < 0) {
		perror("Failed to reserve a single line");
		ret = EXIT_FAILURE;
		goto release;
	}

	ret = gpiod_line_set_value(line, LINE_OUTPUT_LOW);
	if (ret < 0) {
		perror("Failed to set the value of a single GPIO line (OUTPUT_LOW)");
		ret = EXIT_FAILURE;
		goto release;
	}

	sleep(PICO_RESET_LOW_DURATION);

	ret = gpiod_line_set_value(line, LINE_OUTPUT_HIGH);
	if (ret < 0) {
		perror("Failed to set the value of a single GPIO line (OUTPUT_HIGH)");
		ret = EXIT_FAILURE;
	} else {
		ret = EXIT_SUCCESS;
	}

release:
	gpiod_line_release(line);

close:
	gpiod_chip_close(chip);

end:
	return ret;
}
