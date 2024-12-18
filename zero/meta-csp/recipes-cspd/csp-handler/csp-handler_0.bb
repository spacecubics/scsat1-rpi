LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=86d3f3a95c324c9479bd8986968f4327"

TARGET_CC_ARCH += "${LDFLAGS}"

SRC_URI = "file://main.c \
           file://utils.h \
           file://handler.c \
           file://handler.h \
           file://router.c \
           file://router.h \
           file://temp.c \
           file://temp.h \
           file://camera.c \
           file://camera.h \
           file://hwtest.c \
           file://hwtest.h \
           file://file.c \
           file://file.h \
           file://upload.c \
           file://upload.h \
           file://shell.c \
           file://shell.h \
           file://system.c \
           file://system.h \
           file://version.h \
           file://cspd.h \
           file://Makefile \
           file://LICENSE \
           file://camera/dng_writer.cpp \
           file://camera/dng_writer.h \
           file://camera/frame_plane_mapper.cpp \
           file://camera/frame_plane_mapper.h \
           file://camera/capture_raw.cpp \
           file://camera/capture_raw.h"

S = "${WORKDIR}"

DEPENDS:append = "libcsp systemd glib-2.0 arducam-pivariety libcamera-arducam tiff"

EXTRA_OEMAKE = "DESTDIR=${D} LIBDIR=${libdir} INCLUDEDIR=${includedir} BINDIR=${bindir} \
                CFLAGS+='-D MAIN_OBC_CAN_ADDR=${MAIN_OBC_CAN_ADDRESS} -D RPI_ZERO_CAN_ADDR=${RPI_ZERO_CAN_ADDRESS} -D RPI_ZERO_UART_ADDR=${RPI_ZERO_UART_ADDRESS} -D RPI_PICO_UART_ADDR=${RPI_PICO_UART_ADDRESS}'"

do_install() {
    oe_runmake install
}

inherit pkgconfig
