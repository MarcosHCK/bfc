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
# Subdirectories
#

SUBDIRS=\
	backend \
	$(VOID)

#
# Extras
#

EXTRA_DIST+=\
	dynasm/Readme.md \
	bfd_error.h \
	codegen.h \
	dynasm.h \
	options.h \
	$(VOID)

#
# Binaries and libraries
#

bin_PROGRAMS=bfc

bfc_SOURCES=\
	bfc.c \
	bfcc.c \
	bfl.c \
	codegen.c \
	main.c \
	options.c \
	$(VOID)
bfc_CFLAGS=\
	-DG_LOG_DOMAIN=\"Bfc\" \
	-DABSTOPBUILDDIR=\"${abs_top_builddir}\" \
	-DPKGLIBDIR=\"${pkglibdir}\" \
	$(GIO_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(VOID)
bfc_LDADD=\
	backend/libbundle.la \
	-lbfd \
	$(GIO_LIBS) \
	$(GLIB_LIBS) \
	$(VOID)
