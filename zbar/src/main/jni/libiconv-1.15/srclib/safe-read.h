/* An interface to read() that retries after interrupts.
   Copyright (C) 2002, 2006, 2009-2017 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* Some system calls may be interrupted and fail with errno = EINTR in the
   following situations:
     - The process is stopped and restarted (signal SIGSTOP and SIGCONT, user
       types Ctrl-Z) on some platforms: Mac OS X.
     - The process receives a signal for which a signal handler was installed
       with sigaction() with an sa_flags field that does not contain
       SA_RESTART.
     - The process receives a signal for which a signal handler was installed
       with signal() and for which no call to siginterrupt(sig,0) was done,
       on some platforms: AIX, HP-UX, IRIX, OSF/1, Solaris.

   This module provides a wrapper around read() that handles EINTR.  */

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


#define SAFE_READ_ERROR ((size_t) -1)

/* Read up to COUNT bytes at BUF from descriptor FD, retrying if interrupted.
   Return the actual number of bytes read, zero for EOF, or SAFE_READ_ERROR
   upon error.  */
extern size_t safe_read (int fd, void *buf, size_t count);


#ifdef __cplusplus
}
#endif
