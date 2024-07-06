# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(BOARD rpi_pico)

# build options
set(OPTION_PATH "${CMAKE_CURRENT_SOURCE_DIR}/options")

## -DCONSOLE=uart1 or usb
if (CONSOLE STREQUAL uart1)
    list(APPEND EXTRA_DTC_OVERLAY_FILE "${OPTION_PATH}/uart1.overlay")
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(scsat1-rpi-pico)

target_sources(app PRIVATE
	src/main.c
	src/temp.c
	src/csp.c
)
