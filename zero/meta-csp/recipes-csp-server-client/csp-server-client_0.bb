LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=2915dc85ab8fd26629e560d023ef175c"

TARGET_CC_ARCH += "${LDFLAGS}"

SRC_URI = "file://csp_server_client.c \
           file://csp_server_client_posix.c \
           file://Makefile \
           file://LICENSE"

S = "${WORKDIR}"

DEPENDS:append = "libcsp"

EXTRA_OEMAKE = "DESTDIR=${D} LIBDIR=${libdir} INCLUDEDIR=${includedir} BINDIR=${bindir}"

do_install() {
    oe_runmake install
}
