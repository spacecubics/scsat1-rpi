LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=86d3f3a95c324c9479bd8986968f4327"

TARGET_CC_ARCH += "${LDFLAGS}"

SRC_URI = "file://main.c \
           file://Makefile \
           file://LICENSE \
           "

S = "${WORKDIR}"

DEPENDS:append = "libgpiod"

EXTRA_OEMAKE = "DESTDIR=${D} LIBDIR=${libdir} INCLUDEDIR=${includedir} BINDIR=${bindir}"

do_install() {
    oe_runmake install
}
