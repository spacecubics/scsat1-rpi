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

EXTRA_OEMAKE = "DESTDIR=${D} LIBDIR=${libdir} INCLUDEDIR=${includedir} BINDIR=${bindir} CFLAGS+='-D ZERO_CSP_ADDR=${ZERO_CSP_ADDRESS} -D PICO_CSP_ADDR=${PICO_CSP_ADDRESS}'"

do_install() {
    oe_runmake install
}
