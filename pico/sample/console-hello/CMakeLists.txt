# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(BOARD rpi_pico)

if (USE_UART1)
    list(APPEND EXTRA_DTC_OVERLAY_FILE "uart1.overlay")
endif()

if (ALLOW_DEBUG)
    list(APPEND EXTRA_CONF_FILE "debug.conf")
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(console-hello)

target_sources(app PRIVATE src/main.c)
