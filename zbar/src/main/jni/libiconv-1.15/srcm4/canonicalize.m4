# canonicalize.m4 serial 28

dnl Copyright (C) 2003-2007, 2009-2017 Free Software Foundation, Inc.

dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# Provides canonicalize_file_name and canonicalize_filename_mode, but does
# not provide or fix realpath.
AC_DEFUN([gl_FUNC_CANONICALIZE_FILENAME_MODE],
[
  AC_REQUIRE([gl_USE_SYSTEM_EXTENSIONS])
  AC_CHECK_FUNCS_ONCE([canonicalize_file_name])
  AC_REQUIRE([gl_DOUBLE_SLASH_ROOT])
  AC_REQUIRE([gl_FUNC_REALPATH_WORKS])
  if test $ac_cv_func_canonicalize_file_name = no; then
    HAVE_CANONICALIZE_FILE_NAME=0
  else
    case "$gl_cv_func_realpath_works" in
      *yes) ;;
      *)    REPLACE_CANONICALIZE_FILE_NAME=1 ;;
    esac
  fi
])

# Provides canonicalize_file_name and realpath.
AC_DEFUN([gl_CANONICALIZE_LGPL],
[
  AC_REQUIRE([gl_STDLIB_H_DEFAULTS])
  AC_REQUIRE([gl_CANONICALIZE_LGPL_SEPARATE])
  if test $ac_cv_func_canonicalize_file_name = no; then
    HAVE_CANONICALIZE_FILE_NAME=0
    if test $ac_cv_func_realpath = no; then
      HAVE_REALPATH=0
    else
      case "$gl_cv_func_realpath_works" in
	*yes) ;;
	*)    REPLACE_REALPATH=1 ;;
      esac
    fi
  else
    case "$gl_cv_func_realpath_works" in
      *yes)
        ;;
      *)
        REPLACE_CANONICALIZE_FILE_NAME=1
        REPLACE_REALPATH=1
        ;;
    esac
  fi
])

# Like gl_CANONICALIZE_LGPL, except prepare for separate compilation
# (no REPLACE_CANONICALIZE_FILE_NAME, no REPLACE_REALPATH, no AC_LIBOBJ).
AC_DEFUN([gl_CANONICALIZE_LGPL_SEPARATE],
[
  AC_REQUIRE([gl_USE_SYSTEM_EXTENSIONS])
  AC_CHECK_FUNCS_ONCE([canonicalize_file_name getcwd readlink])
  AC_REQUIRE([gl_DOUBLE_SLASH_ROOT])
  AC_REQUIRE([gl_FUNC_REALPATH_WORKS])
  AC_CHECK_HEADERS_ONCE([sys/param.h])
])

# Check whether realpath works.  Assume that if a platform has both
# realpath and canonicalize_file_name, but the former is broken, then
# so is the latter.
AC_DEFUN([gl_FUNC_REALPATH_WORKS],
[
  AC_CHECK_FUNCS_ONCE([realpath])
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
  AC_CACHE_CHECK([whether realpath works], [gl_cv_func_realpath_works], [
    touch conftest.a
    mkdir conftest.d
    AC_RUN_IFELSE([
      AC_LANG_PROGRAM([[
        ]GL_NOCRASH[
        #include <stdlib.h>
        #include <string.h>
      ]], [[
        int result = 0;
        {
          char *name = realpath ("conftest.a", NULL);
          if (!(name && *name == '/'))
            result |= 1;
          free (name);
        }
        {
          char *name = realpath ("conftest.b/../conftest.a", NULL);
          if (name != NULL)
            result |= 2;
          free (name);
        }
        {
          char *name = realpath ("conftest.a/", NULL);
          if (name != NULL)
            result |= 4;
          free (name);
        }
        {
          char *name1 = realpath (".", NULL);
          char *name2 = realpath ("conftest.d//./..", NULL);
          if (! name1 || ! name2 || strcmp (name1, name2))
            result |= 8;
          free (name1);
          free (name2);
        }
        return result;
      ]])
     ],
     [gl_cv_func_realpath_works=yes],
     [gl_cv_func_realpath_works=no],
     [case "$host_os" in
                       # Guess yes on glibc systems.
        *-gnu* | gnu*) gl_cv_func_realpath_works="guessing yes" ;;
                       # If we don't know, assume the worst.
        *)             gl_cv_func_realpath_works="guessing no" ;;
      esac
     ])
    rm -rf conftest.a conftest.d
  ])
  case "$gl_cv_func_realpath_works" in
    *yes)
      AC_DEFINE([FUNC_REALPATH_WORKS], [1], [Define to 1 if realpath()
        can malloc memory, always gives an absolute path, and handles
        trailing slash correctly.])
      ;;
  esac
])
