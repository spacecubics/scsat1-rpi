SRC_URI += "file://disable-wifi-overlay.dts;subdir=git/arch/${ARCH}/boot/dts/overlays"

FILESEXTRAPATHS:append := "${THISDIR}/files:"

PACKAGE_ARCH = "${MACHINE_ARCH}"
