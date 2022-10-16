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

AC_PREREQ([2.69])

#
# Take version information
# Extract mayor, minor and micro version
# numbers from version string
# Also define release type
#

m4_define([v_VERSION], m4_esyscmd_s([git describe]))
m4_define([v_MAJOR], m4_esyscmd_s([git describe | sed -e 's/^v\([0-9]*\)\.\([0-9]*\)\.\([0-9]*\)\-\([a-z]*\).*/\1/']))
m4_define([v_MINOR], m4_esyscmd_s([git describe | sed -e 's/^v\([0-9]*\)\.\([0-9]*\)\.\([0-9]*\)\-\([a-z]*\).*/\2/']))
m4_define([v_MICRO], m4_esyscmd_s([git describe | sed -e 's/^v\([0-9]*\)\.\([0-9]*\)\.\([0-9]*\)\-\([a-z]*\).*/\3/']))
m4_define([v_STAGE], m4_esyscmd_s([git describe | sed -e 's/^v\([0-9]*\)\.\([0-9]*\)\.\([0-9]*\)\-\([a-z]*\).*/\4/']))

#
# autoconf
#

AC_INIT([bfc], [v_VERSION], [bfc.bugs@hcktm.com], [bfc], [https://github.com/MarcosHCK/bfc/])
AC_CONFIG_SRCDIR([src/bfc.c])
AC_LANG([C])

#
# aclocal
#

AC_CONFIG_MACRO_DIRS([m4])

#
# Setup default cross-compiling
# environment mechanics
#

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

#
# automake
#

AM_INIT_AUTOMAKE([subdir-objects])

#
# Custom build options
#

AC_ARG_ENABLE(
  [debug],
  [AS_HELP_STRING(
    [--enable-debug],
    [Enable debug object output @<:@default=no@:>@])],
  [AM_CONDITIONAL([DEBUG], [test "x$enableval" != "xno"])
   if test "x$enableval" != "xno"; then
    AC_DEFINE([DEBUG], [1], [Debugging enabled])
    AC_SUBST([DEBUG], [1])
   else
    AC_DEFINE([DEBUG], [0], [Debugging disabled])
    AC_SUBST([DEBUG], [0])
   fi],
  [ AC_DEFINE([DEBUG], [0], [Debugging disabled])
    AC_SUBST([DEBUG], [0])])

AC_ARG_ENABLE(
  [developer],
  [AS_HELP_STRING(
    [--enable-developer],
    [Enable developer features @<:@default=no@:>@])],
  [AM_CONDITIONAL([DEVELOPER], [test "x$enableval" != "xno"])
   if test "x$enableval" != "xno"; then
    AC_DEFINE([DEVELOPER], [1], [Developer features enabled])
    AC_SUBST([DEVELOPER], [1])
   else
    AC_DEFINE([DEVELOPER], [0], [Developer features disabled])
    AC_SUBST([DEVELOPER], [0])
   fi],
  [ AC_DEFINE([DEVELOPER], [0], [Developer features disabled])
    AC_SUBST([DEVELOPER], [0])])

AC_SUBST([PACKAGE_VERSION_MAJOR], [v_MAJOR])
AC_DEFINE_UNQUOTED([PACKAGE_VERSION_MAJOR], [v_MAJOR], [Version mayor number])
AC_SUBST([PACKAGE_VERSION_MINOR], [v_MINOR])
AC_DEFINE_UNQUOTED([PACKAGE_VERSION_MINOR], [v_MINOR], [Version minor number])
AC_SUBST([PACKAGE_VERSION_MICRO], [v_MICRO])
AC_DEFINE_UNQUOTED([PACKAGE_VERSION_MICRO], [v_MICRO], [Version micro number])
AC_SUBST([PACKAGE_RELEASE], ["v_STAGE"])
AC_DEFINE([PACKAGE_RELEASE], ["v_STAGE"], [Release stage])

#
#  Checks for programs we'll
# need to build
#

LT_PREREQ([2.4.6])
LT_INIT

AC_PROG_AWK
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

PKG_PROG_PKG_CONFIG

AC_CHECK_TOOL([LLVM_CONFIG], [llvm-config-14], [no])
if test "x$LLVM_CONFIG" = "xno"; then
  AC_CHECK_TOOL([LLVM_CONFIG], [llvm-config], [no])
  if test "x$LLVM_CONFIG" = "xno"; then
    AC_MSG_FAILURE([llvm-config not found our your system])
  else
    LLVM_CFLAGS=`$LLVM_CONFIG --cflags`
    AC_SUBST([LLVM_CFLAGS], [])
    LLVM_LIBS=`$LLVM_CONFIG -- libs`
    AC_SUBST([LLVM_LIBS], [])
  fi
else
  LLVM_CFLAGS=`$LLVM_CONFIG --cflags`
  AC_SUBST([LLVM_CFLAGS], [])
  LLVM_LIBS=`$LLVM_CONFIG --libs`
  AC_SUBST([LLVM_LIBS], [])
fi

#
# Custom build options
#

debugging="no"
developer="no"

AC_ARG_ENABLE([debug],
              [AS_HELP_STRING([--enable-debug], [Enable debug object output @<:@default=no@:>@])],
              [if test "x$enableval" != "xno"; then
                debugging="yes"
               fi
              ])

AM_CONDITIONAL([DEBUG], [test "x$debugging" = "xyes"])
if test "x$debugging" = "xno"; then
  AC_DEFINE([DEBUG], [0], [Debugging disabled])
  AC_SUBST([DEBUG], [0])
else
  AC_DEFINE([DEBUG], [1], [Debugging enabled])
  AC_SUBST([DEBUG], [1])
fi

AC_ARG_ENABLE([developer],
              [AS_HELP_STRING([--enable-developer], [Enable developer features @<:@default=no@:>@])],
              [if test "x$enableval" != "xno"; then
                developer="yes"
               fi
              ])

AM_CONDITIONAL([DEVELOPER], [test "x$developer" = "xyes"])
if test "x$developer" = "xno"; then
  AC_DEFINE([DEVELOPER], [0], [Developer feature disabled])
  AC_SUBST([DEVELOPER], [0])
else
  AC_DEFINE([DEVELOPER], [1], [Developer feature enabled])
  AC_SUBST([DEVELOPER], [1])
fi

#
# Check for libraries using pkg-config
# Note: keep sorted alphabetically 
#

PKG_CHECK_MODULES([GIO], [gio-2.0])
PKG_CHECK_MODULES([GLIB], [glib-2.0])
AC_CHECK_LIB([bfd], [bfd_get_error])

#
# Check for libraries
#

#
# Checks for header files.
#

#
# Checks for typedefs, structures, and compiler characteristics.
#

AC_TYPE_SIZE_T
AC_CHECK_TYPES([ptrdiff_t])

#
# Checks for library functions.
#

AC_FUNC_REALLOC
AC_CHECK_FUNCS([memcpy])
AC_CHECK_FUNCS([memset])

#
# Prepare output
#

AC_CONFIG_FILES([
  src/Makefile
  Makefile
])

AC_CONFIG_HEADERS([config.h])
AC_OUTPUT
