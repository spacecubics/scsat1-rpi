LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=b234ee4d69f5fce4486a80fdaf4a4263"

inherit module

SRC_URI = "file://Makefile \
           file://arducam-pivariety.c \
           file://arducam-pivariety.h \
           file://COPYING"

S = "${WORKDIR}"
RPROVIDES_${PN} += "kernel-module-arducam-pivariety"
