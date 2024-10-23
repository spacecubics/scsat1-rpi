SRC_URI += "file://disable-usb-overlay.dts;subdir=git/arch/${ARCH}/boot/dts/overlays"

FILESEXTRAPATHS:append := "${THISDIR}/files:"

PACKAGE_ARCH = "${MACHINE_ARCH}"
