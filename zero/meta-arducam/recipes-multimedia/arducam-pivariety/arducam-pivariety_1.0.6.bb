SUMMARY = "Arducam pivariety SDK recipe"
SECTION = "libs"
LICENSE = "CLOSED"

SRC_URI = "\
    file://libarducam_pivariety.so.1.0.6 \
    file://arducam_pivariety.hpp \
    file://arducam_pivariety.pc \
"

INHIBIT_PACKAGE_STRIP = "1"
INHIBIT_SYSROOT_STRIP = "1"
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"

RDEPENDS_${PN} += "libgcc zlib"
DEPENDS += " \
          zlib \
          libgcc \
"

do_install () {
    install -d ${D}${libdir}
    install -d ${D}${includedir}/arducam
    install -d ${D}${libdir}/pkgconfig
    install -m 644 ${WORKDIR}/libarducam_pivariety.so.1.0.6 ${D}${libdir}
    install -m 644 ${WORKDIR}/arducam_pivariety.hpp ${D}${includedir}/arducam
    install -m 644 ${WORKDIR}/arducam_pivariety.pc ${D}${libdir}/pkgconfig/
    ln -sf libarducam_pivariety.so.1.0.6 ${D}${libdir}/libarducam_pivariety.so
}

FILES:${PN} = "${libdir}/libarducam_pivariety.so.1.0.6"
FILES:${PN}-dev = "${libdir}/libarducam_pivariety.so \
                   ${libdir}/pkgconfig/arducam_pivariety.pc \
                   ${includedir}/arducam/arducam_pivariety.hpp"
