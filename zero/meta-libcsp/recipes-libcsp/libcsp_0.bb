LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=2915dc85ab8fd26629e560d023ef175c"

SRCREV = "${AUTOREV}"
SRCBRANCH = "develop"
SRCREV = "1653d5b340cdf97ceb97dbf84b2dcbbfe8623e8c"
SRC_URI = "git://github.com/libcsp/libcsp.git;protocol=https;branch=${SRCBRANCH};"
SRC_URI += "file://0001-drivers-usart-Remove-exit-code-from-Linux-driver.patch"

DEPENDS += "libsocketcan"

PACKAGES = "${PN} ${PN}-dbg ${PN}-dev"

FILES:${PN}-dbg += "${libdir}/.debug"
FILES:${PN}     += "${libdir}/libcsp.so"
FILES:${PN}-dev += "${includedir}/csp"

S = "${WORKDIR}/git"

inherit cmake pkgconfig

EXTRA_OECMAKE += "-DCSP_USE_RTABLE=ON"

do_install:append() {
    chmod 644 ${D}${libdir}/libcsp.so
}
