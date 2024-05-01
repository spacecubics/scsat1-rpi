LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=86d3f3a95c324c9479bd8986968f4327"

TARGET_CC_ARCH += "${LDFLAGS}"

SRC_URI = "file://main.c \
           file://handler.c \
           file://router.c \
           file://cspd.h \
           file://Makefile \
           file://LICENSE"

S = "${WORKDIR}"

DEPENDS:append = "libcsp"

EXTRA_OEMAKE = "DESTDIR=${D} LIBDIR=${libdir} INCLUDEDIR=${includedir} BINDIR=${bindir} \
                CFLAGS+='-D MAIN_OBC_CAN_ADDR=${MAIN_OBC_CAN_ADDRESS} -D RPI_ZERO_CAN_ADDR=${RPI_ZERO_CAN_ADDRESS} -D RPI_ZERO_UART_ADDR=${RPI_ZERO_UART_ADDRESS} -D RPI_PICO_UART_ADDR=${RPI_PICO_UART_ADDRESS}'"

do_install() {
    oe_runmake install
}
