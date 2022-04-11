# Copyright 2021-2025 MarcosHCK
# This file is part of bfc (BrainFuck Compiler).
#
# bfc (BrainFuck Compiler) is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# bfc (BrainFuck Compiler) is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with bfc (BrainFuck Compiler). If not, see <http://www.gnu.org/licenses/>.
#

#
# Some variables
#

PACKAGE_NAME=@PACKAGE_NAME@
PACKAGE_RELEASE=@PACKAGE_RELEASE@
PACKAGE_VERSION_MAYOR=@PACKAGE_VERSION_MAYOR@
PACKAGE_VERSION_MINOR=@PACKAGE_VERSION_MINOR@
PACKAGE_VERSION_MICRO=@PACKAGE_VERSION_MICRO@
PACKAGE_VERSION_BUILD=@PACKAGE_VERSION_BUILD@

EXTRA_DIST=
CLEANFILES=
VOID=

#
# Extras
#

EXTRA_DIST+=\
	std-i386.h \
	std-x86_64.h \
	$(VOID)

#
# Binaries and libraries`
#

noinst_LTLIBRARIES=libbundle.la
pkglib_LTLIBRARIES=\
	libstd-x86_64.la \
	libstd-i386.la \
	$(VOID)

libbundle_la_SOURCES=\
	i386.c \
	x86_64.c \
  $(VOID)
libbundle_la_CFLAGS=\
	-DG_LOG_DOMAIN=\"Bfc.Backend\" \
  -I${top_builddir}/src/ \
	-D__BACKEND__=1 \
	$(GIO_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(VOID)
libbundle_la_CCASFLAGS=\
  -I${top_builddir}/src/ \
	$(GIO_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(VOID)
libbundle_la_LIBADD=\
	$(GIO_LIBS) \
	$(GLIB_LIBS) \
	$(VOID)
libbundle_la_LDFLAGS=\
	-static \
	$(VOID)

libstd_x86_64_la_SOURCES=\
	std-x86_64.c \
	$(VOID)
libstd_x86_64_la_CFLAGS=\
	$(VOID)
libstd_x86_64_la_LIBADD=\
	$(VOID)
libstd_x86_64_la_LDFLAGS=\
	-rpath ${pkglibdir} \
	-release ${PACKAGE_RELEASE} \
	-version-info ${PACKAGE_VERSION_MAYOR}:${PACKAGE_VERSION_MINOR}:${PACKAGE_VERSION_MICRO} \
	-static \
	-m64 \
	$(VOID)

libstd_i386_la_SOURCES=\
	std-x86_64.c \
	$(VOID)
libstd_i386_la_CFLAGS=\
	$(VOID)
libstd_i386_la_LIBADD=\
	$(VOID)
libstd_i386_la_LDFLAGS=\
	-rpath ${pkglibdir} \
	-release ${PACKAGE_RELEASE} \
	-version-info ${PACKAGE_VERSION_MAYOR}:${PACKAGE_VERSION_MINOR}:${PACKAGE_VERSION_MICRO} \
	-static \
	-m32 \
	$(VOID)

#
# Custom extensions
#

SUFFIXES=.dasc.c .c

DYNASM_CFLAGS=\
	-D DEBUG=\"${DEBUG}\" \
	-D DEVELOPER=\"${DEVELOPER}\" \
	$(VOID)

.dasc.c.c:
	$(DYNASM) $(DYNASM_CFLAGS) -o $@ $<
