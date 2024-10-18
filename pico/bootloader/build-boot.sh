#!/bin/sh
#
# Build the bootloader.
#

ABSPATH=$(readlink -f $0)
BASEDIR=$(dirname "$ABSPATH")

cd $BASEDIR || exit 1

rm -rf build

west build -b rpi_pico ../../../bootloader/mcuboot/boot/zephyr \
           -- -DDTC_OVERLAY_FILE="$BASEDIR"/rpi_pico.overlay \
              -DOVERLAY_CONFIG="$BASEDIR"/rpi_pico.conf
