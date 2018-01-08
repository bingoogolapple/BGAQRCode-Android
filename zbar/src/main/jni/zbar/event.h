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
#ifndef _ZBAR_EVENT_H_
#define _ZBAR_EVENT_H_

#include <config.h>
#include "mutex.h"
#include "timer.h"

/* platform synchronization "event" abstraction
 */

#if defined(_WIN32)

# include <windows.h>

typedef HANDLE zbar_event_t;


#else

# ifdef HAVE_LIBPTHREAD
#  include <pthread.h>
# endif

typedef struct zbar_event_s {
    int state;
# ifdef HAVE_LIBPTHREAD
    pthread_cond_t cond;
# endif
    int pollfd;
} zbar_event_t;

#endif


extern int _zbar_event_init(zbar_event_t*);
extern void _zbar_event_destroy(zbar_event_t*);
extern void _zbar_event_trigger(zbar_event_t*);
extern int _zbar_event_wait(zbar_event_t*, zbar_mutex_t*, zbar_timer_t*);

#endif
