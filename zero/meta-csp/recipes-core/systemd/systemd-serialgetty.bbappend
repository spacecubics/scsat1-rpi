do_install:append() {
    rm ${D}${systemd_system_unitdir}/serial-getty@.service
}

FILES:${PN} += "${systemd_system_unitdir}"
