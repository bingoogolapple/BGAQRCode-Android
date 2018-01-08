/*------------------------------------------------------------------------
 *  Copyright 2007-2009 (c) Jeff Brown <spadix@users.sourceforge.net>
 *
 *  This file is part of the ZBar Bar Code Reader.
 *
 *  The ZBar Bar Code Reader is free software; you can redistribute it
 *  and/or modify it under the terms of the GNU Lesser Public License as
 *  published by the Free Software Foundation; either version 2.1 of
 *  the License, or (at your option) any later version.
 *
 *  The ZBar Bar Code Reader is distributed in the hope that it will be
 *  useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser Public License
 *  along with the ZBar Bar Code Reader; if not, write to the Free
 *  Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301  USA
 *
 *  http://sourceforge.net/projects/zbar
 *------------------------------------------------------------------------*/
#ifndef _ZBAR_THREAD_H_
#define _ZBAR_THREAD_H_

/* simple platform thread abstraction
 */

#include <config.h>
#include "event.h"

#if defined(_WIN32)

# include <windows.h>
# define HAVE_THREADS
# define ZTHREAD DWORD WINAPI

typedef ZTHREAD (zbar_thread_proc_t)(void*);

typedef DWORD zbar_thread_id_t;

#elif defined(HAVE_LIBPTHREAD)

# include <pthread.h>
# include <signal.h>
# define HAVE_THREADS
# define ZTHREAD void*

typedef ZTHREAD (zbar_thread_proc_t)(void*);

typedef pthread_t zbar_thread_id_t;

#else

# undef HAVE_THREADS
# undef ZTHREAD

typedef void zbar_thread_proc_t;
typedef int zbar_thread_id_t;

#endif


typedef struct zbar_thread_s {
    zbar_thread_id_t tid;
    int started, running;
    zbar_event_t notify, activity;
} zbar_thread_t;


#if defined(_WIN32)

static inline void _zbar_thread_init (zbar_thread_t *thr)
{
    thr->running = 1;
    _zbar_event_trigger(&thr->activity);
}

static inline zbar_thread_id_t _zbar_thread_self ()
{
    return(GetCurrentThreadId());
}

static inline int _zbar_thread_is_self (zbar_thread_id_t tid)
{
    return(tid == GetCurrentThreadId());
}


#elif defined(HAVE_LIBPTHREAD)

static inline void _zbar_thread_init (zbar_thread_t *thr)
{
    sigset_t sigs;
    sigfillset(&sigs);
    pthread_sigmask(SIG_BLOCK, &sigs, NULL);
    thr->running = 1;
    _zbar_event_trigger(&thr->activity);
}

static inline zbar_thread_id_t _zbar_thread_self (void)
{
    return(pthread_self());
}

static inline int _zbar_thread_is_self (zbar_thread_id_t tid)
{
    return(pthread_equal(tid, pthread_self()));
}


#else

# define _zbar_thread_start(...) -1
# define _zbar_thread_stop(...) 0
# define _zbar_thread_self(...) 0
# define _zbar_thread_is_self(...) 1

#endif

#ifdef HAVE_THREADS
extern int _zbar_thread_start(zbar_thread_t*, zbar_thread_proc_t*,
                              void*, zbar_mutex_t*);
extern int _zbar_thread_stop(zbar_thread_t*, zbar_mutex_t*);
#endif

#endif
