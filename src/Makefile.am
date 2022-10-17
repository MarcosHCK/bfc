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

VOID=

bin_PROGRAMS=\
	bfc \
	$(VOID)

noinst_HEADERS=\
	bfc.h \
	codegen.hpp \
	collect.h \
	stream.hpp \
	$(VOID)

bfc_SOURCES=\
	bfc.c \
	codegen.cpp \
	collect.c \
	stream.cpp \
	$(VOID)
bfc_CFLAGS=\
	-DG_LOG_DOMAIN=\"Bfc\" \
	$(GIO_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(LLVM_CFLAGS) \
	$(VOID)
bfc_CXXFLAGS=\
	-DG_LOG_DOMAIN=\"Bfc\" \
	$(GIO_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(LLVM_CXXFLAGS) \
	$(VOID)
bfc_LDADD=\
	$(GIO_LIBS) \
	$(GLIB_LIBS) \
	$(GOBJECT_LIBS) \
	$(LLVM_LIBS) \
	$(VOID)
