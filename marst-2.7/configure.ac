dnl Process this file with autoconf to produce a configure script.

AC_INIT([MARST],[2.7],[bug-marst@gnu.org])
AC_CONFIG_SRCDIR([marst.c])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE

dnl Check for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

dnl Check for libraries.
AC_CHECK_LIB([m],[sqrt])

dnl Disable unnecessary libtool tests
define([AC_LIBTOOL_LANG_CXX_CONFIG],[:])
define([AC_LIBTOOL_LANG_F77_CONFIG],[:])
define([AC_LIBTOOL_LANG_GCJ_CONFIG],[:])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
