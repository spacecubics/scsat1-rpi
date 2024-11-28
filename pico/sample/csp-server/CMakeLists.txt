# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(BOARD rpi_pico)

# build options
set(OPTION_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../options")

## -DCONSOLE=uart1 or usb
if (CONSOLE STREQUAL uart1)
    list(APPEND EXTRA_DTC_OVERLAY_FILE "${OPTION_PATH}/uart1.overlay")
elseif (CONSOLE STREQUAL usb)
    list(APPEND EXTRA_DTC_OVERLAY_FILE "${OPTION_PATH}/usb.overlay")
    list(APPEND EXTRA_CONF_FILE "${OPTION_PATH}/usb.conf")
endif()

## -DALLOW_DEBUG=y
if (ALLOW_DEBUG)
	list(APPEND EXTRA_CONF_FILE "${OPTION_PATH}/debug.conf")
	
	if (CONSOLE STREQUAL usb)
        list(APPEND EXTRA_CONF_FILE "${OPTION_PATH}/usb-debug.conf")
	endif()
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(csp-server)

target_sources(app PRIVATE src/main.c src/thread.c)
