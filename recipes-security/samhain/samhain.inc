DESCRIPTION = "Provides file integrity checking and log file monitoring/analysis"
HOMEPAGE    = "http://www.la-samhna.de/samhain/"
LICENSE     = "GPLv2"
LIC_FILES_CHKSUM = "file://LICENSE;md5=8ca43cbc842c2336e835926c2166c28b"


SRC_URI = "http://la-samhna.de/archive/samhain_signed-${PV}.tar.gz \
	   file://${INITSCRIPT_NAME}.init \
	   file://${INITSCRIPT_NAME}.default \
           file://c99_dnmalloc.patch \
	  "

SRC_URI[md5sum] = "d98a55646b14f9419fcedde909d1bf02"
SRC_URI[sha256sum] = "6b2db91fc92b3a9fc2edcc6ee16438156753c05f69c114856289e9f25ba0e50a"

S = "${WORKDIR}/samhain-${PV}"

inherit autotools-brokensep update-rc.d pkgconfig

SAMHAIN_PORT ??= "49777"
SAMHAIN_SERVER ??= "NULL"

INITSCRIPT_NAME = "samhain-${SAMHAIN_MODE}"
INITSCRIPT_PARAMS ?= "defaults"


PACKAGECONFIG ??= ""

do_unpack_samhain() {
    cd ${WORKDIR}
    tar -xzvf samhain-${PV}.tar.gz
}

python do_unpack_append() {
    bb.build.exec_func('do_unpack_samhain', d)
}

do_configure_prepend_arm() {
    export sh_cv___va_copy=yes
}

do_configure_prepend_aarch64() {
    export sh_cv___va_copy=yes
}

# If we use oe_runconf in do_configure() it will by default
# use the prefix --oldincludedir=/usr/include which is not
# recognized by Samhain's configure script and would invariably
# throw back the error "unrecognized option: --oldincludedir=/usr/include"
do_configure_prepend () {
    cat << EOF > ${S}/config-site.${BP}
ssp_cv_lib=no
sh_cv_va_copy=yes
EOF
    export CONFIG_SITE=${S}/config-site.${BP}
}

do_configure () {
	./configure \
	--build=${BUILD_SYS} \
	--host=${HOST_SYS} \
	--target=${TARGET_SYS} \
	--prefix=${prefix} \
	--exec_prefix=${exec_prefix} \
	--bindir=${bindir} \
	--sbindir=${sbindir} \
	--libexecdir=${libexecdir} \
	--datadir=${datadir} \
	--sysconfdir=${sysconfdir} \
	--sharedstatedir=${sharedstatedir} \
	--localstatedir=${localstatedir} \
	--libdir=${libdir} \
	--includedir=${includedir} \
	--infodir=${infodir} \
	--mandir=${mandir} \
	${EXTRA_OECONF}
}

do_compile_prepend_libc-musl () {
	sed -i 's/^#define HAVE_MALLOC_H.*//' ${B}/config.h
}

# Install the init script, it's default file, and the extraneous
# documentation.
do_install_append () {
	cd ${S}
	oe_runmake install DESTDIR='${D}' INSTALL=install-boot
	install -d ${D}${sysconfdir}/init.d
	install -m 755 ${WORKDIR}/${INITSCRIPT_NAME}.init \
		${D}${sysconfdir}/init.d/${INITSCRIPT_NAME}

	install -d ${D}${sysconfdir}/default
	install -m 755 ${WORKDIR}/${INITSCRIPT_NAME}.default \
		${D}${sysconfdir}/default/${INITSCRIPT_NAME}

	install -d ${D}${docdir}/${PN}
	cp -r docs/* ${D}${docdir}/${PN}
	cp -r scripts ${D}${docdir}/${PN}
	install -d -m 755 ${D}/var/samhain
}

FILES_${PN} += "\
    /run \
    "

INSANE_SKIP_${PN} = "already-stripped"
