DESCRIPTION = "Management library for 32-bit MCUs"
HOMEPAGE = "https://github.com/apache/mynewt-mcumgr"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

GO_IMPORT = "github.com/apache/mynewt-mcumgr-cli/mcumgr"
SRC_URI = "git://github.com/apache/mynewt-mcumgr-cli;protocol=https;branch=master"
SRCREV = "5c56bd24066c780aad5836429bfa2ecc4f9a944c"

inherit go
do_compile[network] = "1"

do_compile() {
    cd ${S}/src/${GO_IMPORT}/mcumgr
    mkdir -p ${B}/${GO_BUILD_BINDIR}
    ${GO} version
    ${GO} build -o ${B}/${GO_BUILD_BINDIR}/mcumgr mcumgr.go
    chmod u+w -R ${B}
}

RDEPENDS:${PN}-dev += "bash"
RDEPENDS:${PN}-staticdev += "bash"
