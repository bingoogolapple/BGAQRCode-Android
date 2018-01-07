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

#include "processor.h"
#include <assert.h>

/* the processor api lock is a recursive mutex with added capabilities
 * to completely drop all lock levels before blocking and atomically
 * unblock a waiting set.  the lock is implemented using a variation
 * of the "specific notification pattern" [cargill], which makes it
 * easy to provide these features across platforms with consistent,
 * predictable semantics.  probably overkill, but additional overhead
 * associated with this mechanism should fall in the noise, as locks
 * are only exchanged O(frame/image)
 *
 * [cargill]
 * http://www.profcon.com/profcon/cargill/jgf/9809/SpecificNotification.html
 */

static inline proc_waiter_t *proc_waiter_queue (zbar_processor_t *proc)
{
    proc_waiter_t *waiter = proc->free_waiter;
    if(waiter) {
        proc->free_waiter = waiter->next;
        waiter->events = 0;
    }
    else {
        waiter = calloc(1, sizeof(proc_waiter_t));
        _zbar_event_init(&waiter->notify);
    }

    waiter->next = NULL;
    waiter->requester = _zbar_thread_self();

    if(proc->wait_head)
        proc->wait_tail->next = waiter;
    else
        proc->wait_head = waiter;
    proc->wait_tail = waiter;
    return(waiter);
}

static inline proc_waiter_t *proc_waiter_dequeue (zbar_processor_t *proc)
{
    proc_waiter_t *prev = proc->wait_next, *waiter;
    if(prev)
        waiter = prev->next;
    else
        waiter = proc->wait_head;
    while(waiter && (waiter->events & EVENTS_PENDING)) {
        prev = waiter;
        proc->wait_next = waiter;
        waiter = waiter->next;
    }

    if(waiter) {
        if(prev)
            prev->next = waiter->next;
        else
            proc->wait_head = waiter->next;
        if(!waiter->next)
            proc->wait_tail = prev;
        waiter->next = NULL;

        proc->lock_level = 1;
        proc->lock_owner = waiter->requester;
    }
    return(waiter);
}

static inline void proc_waiter_release (zbar_processor_t *proc,
                                        proc_waiter_t *waiter)
{
    if(waiter) {
        waiter->next = proc->free_waiter;
        proc->free_waiter = waiter;
    }
}

int _zbar_processor_lock (zbar_processor_t *proc)
{
    if(!proc->lock_level) {
        proc->lock_owner = _zbar_thread_self();
        proc->lock_level = 1;
        return(0);
    }

    if(_zbar_thread_is_self(proc->lock_owner)) {
        proc->lock_level++;
        return(0);
    }

    proc_waiter_t *waiter = proc_waiter_queue(proc);
    _zbar_event_wait(&waiter->notify, &proc->mutex, NULL);

    assert(proc->lock_level == 1);
    assert(_zbar_thread_is_self(proc->lock_owner));

    proc_waiter_release(proc, waiter);
    return(0);
}

int _zbar_processor_unlock (zbar_processor_t *proc,
                            int all)
{
    assert(proc->lock_level > 0);
    assert(_zbar_thread_is_self(proc->lock_owner));

    if(all)
        proc->lock_level = 0;
    else
        proc->lock_level--;

    if(!proc->lock_level) {
        proc_waiter_t *waiter = proc_waiter_dequeue(proc);
        if(waiter)
            _zbar_event_trigger(&waiter->notify);
    }
    return(0);
}

void _zbar_processor_notify (zbar_processor_t *proc,
                             unsigned events)
{
    proc->wait_next = NULL;
    proc_waiter_t *waiter;
    for(waiter = proc->wait_head; waiter; waiter = waiter->next)
        waiter->events = ((waiter->events & ~events) |
                          (events & EVENT_CANCELED));

    if(!proc->lock_level) {
        waiter = proc_waiter_dequeue(proc);
        if(waiter)
            _zbar_event_trigger(&waiter->notify);
    }
}

static inline int proc_wait_unthreaded (zbar_processor_t *proc,
                                        proc_waiter_t *waiter,
                                        zbar_timer_t *timeout)
{
    int blocking = proc->streaming && zbar_video_get_fd(proc->video) < 0;
    _zbar_mutex_unlock(&proc->mutex);

    int rc = 1;
    while(rc > 0 && (waiter->events & EVENTS_PENDING)) {
        /* FIXME lax w/the locking (though shouldn't matter...) */
        if(blocking) {
            zbar_image_t *img = zbar_video_next_image(proc->video);
            if(!img) {
                rc = -1;
                break;
            }

            /* FIXME reacquire API lock! (refactor w/video thread?) */
            _zbar_mutex_lock(&proc->mutex);
            _zbar_process_image(proc, img);
            zbar_image_destroy(img);
            _zbar_mutex_unlock(&proc->mutex);
        }
        int reltime = _zbar_timer_check(timeout);
        if(blocking && (reltime < 0 || reltime > MAX_INPUT_BLOCK))
            reltime = MAX_INPUT_BLOCK;
        rc = _zbar_processor_input_wait(proc, NULL, reltime);
    }
    _zbar_mutex_lock(&proc->mutex);
    return(rc);
}

int _zbar_processor_wait (zbar_processor_t *proc,
                          unsigned events,
                          zbar_timer_t *timeout)
{
    _zbar_mutex_lock(&proc->mutex);
    int save_level = proc->lock_level;
    proc_waiter_t *waiter = proc_waiter_queue(proc);
    waiter->events = events & EVENTS_PENDING;

    _zbar_processor_unlock(proc, 1);
    int rc;
    if(proc->threaded)
        rc = _zbar_event_wait(&waiter->notify, &proc->mutex, timeout);
    else
        rc = proc_wait_unthreaded(proc, waiter, timeout);

    if(rc <= 0 || !proc->threaded) {
        /* reacquire api lock */
        waiter->events &= EVENT_CANCELED;
        proc->wait_next = NULL;
        if(!proc->lock_level) {
            proc_waiter_t *w = proc_waiter_dequeue(proc);
            assert(w == waiter);
        }
        else
            _zbar_event_wait(&waiter->notify, &proc->mutex, NULL);
    }
    if(rc > 0 && (waiter->events & EVENT_CANCELED))
        rc = -1;

    assert(proc->lock_level == 1);
    assert(_zbar_thread_is_self(proc->lock_owner));

    proc->lock_level = save_level;
    proc_waiter_release(proc, waiter);
    _zbar_mutex_unlock(&proc->mutex);
    return(rc);
}
