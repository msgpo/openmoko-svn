SUBDIRS = libbb libopkg src tests utils

HOST_CPU=@host_cpu@
BUILD_CPU=@build_cpu@
OPKGLIBDIR=@opkglibdir@ 
ALL_CFLAGS=-g -O -Wall -DHOST_CPU_STR=\"@host_cpu@\" -DBUILD_CPU=@build_cpu@ -DLIBDIR=\"@libdir@\" -DOPKGLIBDIR=\"@opkglibdir@\" -DDATADIR=\"@datadir@\"

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libopkg.pc



interceptdir = $(datadir)/opkg/intercept
intercept_DATA = intercept/ldconfig intercept/depmod intercept/update-modules

install-data-hook:
	chmod +x $(DESTDIR)$(datadir)/opkg/intercept/*

EXTRA_DIST = $(intercept_DATA)

MAINTAINERCLEANFILES= \
	configure \
	Makefile.in \
	config.guess \
	config.sub \
	ltmain.sh \
	.Makefile.am.swp \
	aclocal.m4

package: all-recursive
	STRIPPROG=$(STRIP) INSTALL=$$PWD/install-sh  binary-arch
