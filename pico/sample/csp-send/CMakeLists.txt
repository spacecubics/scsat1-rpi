# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(BOARD rpi_pico)

if (ALLOW_DEBUG)
    list(APPEND EXTRA_CONF_FILE "debug.conf")
endif()

if (CONSOLE STREQUAL uart1)
    list(APPEND EXTRA_DTC_OVERLAY_FILE "uart1.overlay")
elseif(CONSOLE STREQUAL usb)
    list(APPEND EXTRA_DTC_OVERLAY_FILE "usb.overlay")
    list(APPEND EXTRA_CONF_FILE "usb.conf")
    
    if (ALLOW_DEBUG)
        list(APPEND EXTRA_CONF_FILE "usb-debug.conf")
    endif()
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(csp-send)

target_sources(app PRIVATE src/main.c)
