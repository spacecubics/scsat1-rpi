FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://resolved.conf"

do_install:append() {
    install -d ${D}${sysconfdir}/systemd
    install -m 0644 ${WORKDIR}/resolved.conf ${D}${sysconfdir}/systemd
}
