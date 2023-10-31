LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=86d3f3a95c324c9479bd8986968f4327"

SRC_URI = "file://cspd.service \
           file://LICENSE"

FILES:${PN} += "${systemd_unitdir}/system/cspd.service"
S = "${WORKDIR}"

DEPENDS:append = "csp-handler"

inherit systemd
SYSTEMD_AUTO_ENABLE = "enable"
SYSTEMD_SERVICE:${PN} = "cspd.service"

do_install:append() {
  install -d ${D}/${systemd_unitdir}/system
  install -m 0644 ${WORKDIR}/cspd.service ${D}/${systemd_unitdir}/system
}
