# sys_types_h.m4 serial 6
dnl Copyright (C) 2011-2017 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN_ONCE([gl_SYS_TYPES_H],
[
  AC_REQUIRE([gl_SYS_TYPES_H_DEFAULTS])
  gl_NEXT_HEADERS([sys/types.h])

  dnl Ensure the type pid_t gets defined.
  AC_REQUIRE([AC_TYPE_PID_T])

  dnl Ensure the type mode_t gets defined.
  AC_REQUIRE([AC_TYPE_MODE_T])

  dnl Whether to override the 'off_t' type.
  AC_REQUIRE([gl_TYPE_OFF_T])
])

AC_DEFUN([gl_SYS_TYPES_H_DEFAULTS],
[
])

# This works around a buggy version in autoconf <= 2.69.
# See <https://lists.gnu.org/archive/html/autoconf/2016-08/msg00014.html>

m4_version_prereq([2.70], [], [

# This is taken from the following Autoconf patch:
# http://git.sv.gnu.org/cgit/autoconf.git/commit/?id=e17a30e98

m4_undefine([AC_HEADER_MAJOR])
AC_DEFUN([AC_HEADER_MAJOR],
[AC_CHECK_HEADERS_ONCE([sys/types.h])
AC_CHECK_HEADER([sys/mkdev.h],
  [AC_DEFINE([MAJOR_IN_MKDEV], [1],
    [Define to 1 if `major', `minor', and `makedev' are declared in
     <mkdev.h>.])])
if test $ac_cv_header_sys_mkdev_h = no; then
  AC_CHECK_HEADER([sys/sysmacros.h],
    [AC_DEFINE([MAJOR_IN_SYSMACROS], [1],
      [Define to 1 if `major', `minor', and `makedev' are declared in
       <sysmacros.h>.])])
fi
])

])
