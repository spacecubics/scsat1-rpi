FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://wpa_supplicant-wlan0.conf"

inherit systemd
SYSTEMD_AUTO_ENABLE = "enable"
SYSTEMD_SERVICE:${PN} = "wpa_supplicant@wlan0.service"
FILES:${PN} += "${systemd_unitdir}/system/"

do_compile:append() {
    sed -i -e "s/WiFi-SSID/${DEBUG_WIFI_SSID}/g" ${WORKDIR}/wpa_supplicant-wlan0.conf
    sed -i -e "s/WiFi-PASS/${DEBUG_WIFI_PASS}/g" ${WORKDIR}/wpa_supplicant-wlan0.conf
}

do_install:append() {
    install -d ${D}${sysconfdir}/wpa_supplicant/
    install -m 600 ${WORKDIR}/wpa_supplicant-wlan0.conf ${D}${sysconfdir}/wpa_supplicant/

    install -d ${D}${sysconfdir}/systemd/system/multi-user.target.wants/
    ln -s -r ${D}${systemd_unitdir}/system/wpa_supplicant@.service ${D}${sysconfdir}/systemd/system/multi-user.target.wants/wpa_supplicant@wlan0.service
}
