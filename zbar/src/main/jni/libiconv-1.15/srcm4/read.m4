# read.m4 serial 4
dnl Copyright (C) 2011-2017 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_READ],
[
  AC_REQUIRE([gl_UNISTD_H_DEFAULTS])
  AC_REQUIRE([gl_MSVC_INVAL])
  if test $HAVE_MSVC_INVALID_PARAMETER_HANDLER = 1; then
    REPLACE_READ=1
  fi
  dnl This ifdef is just an optimization, to avoid performing a configure
  dnl check whose result is not used. It does not make the test of
  dnl GNULIB_UNISTD_H_NONBLOCKING or GNULIB_NONBLOCKING redundant.
  m4_ifdef([gl_NONBLOCKING_IO], [
    gl_NONBLOCKING_IO
    if test $gl_cv_have_nonblocking != yes; then
      REPLACE_READ=1
    fi
  ])
])

# Prerequisites of lib/read.c.
AC_DEFUN([gl_PREREQ_READ], [:])
