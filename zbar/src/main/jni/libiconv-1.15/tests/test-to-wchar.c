/* Copyright (C) 2009, 2011 Free Software Foundation, Inc.
   This file is part of the GNU LIBICONV Library.

   The GNU LIBICONV Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   The GNU LIBICONV Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU LIBICONV Library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/>.  */

#include "config.h"

#include <stdlib.h>
#include <iconv.h>
#include <errno.h>

/* This test checks that the conversion to wchar_t stops correctly when
   the input is incomplete.  Based on a bug report from
   Tristan Gingold <gingold@adacore.com>.  */

int main ()
{
  iconv_t cd = iconv_open ("wchar_t", "UTF-8");
  if (cd == (iconv_t)(-1)) {
    /* Skip the test on platforms without wchar_t
      (Solaris 2.6, HP-UX 11.00).  */
  } else {
    char inbuf[2] = { 0xc2, 0xa0 };
    wchar_t outbuf[10];

    char *inptr = inbuf;
    size_t inbytesleft = 1;
    char *outptr = (char *) outbuf;
    size_t outbytesleft = sizeof (outbuf);
    size_t r = iconv (cd,
                      (ICONV_CONST char **) &inptr, &inbytesleft,
                      &outptr, &outbytesleft);

    if (!(r == (size_t)(-1) && errno == EINVAL))
      abort ();
  }

  return 0;
}
