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

#include "window.h"
#include "processor.h"
#include "posix.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

static inline int x_handle_event (zbar_processor_t *proc)
{
    XEvent ev;
    XNextEvent(proc->display, &ev);

    switch(ev.type) {
    case Expose: {
        /* FIXME ignore when running(?) */
        XExposeEvent *exp = (XExposeEvent*)&ev;
        while(1) {
            assert(ev.type == Expose);
            _zbar_window_expose(proc->window, exp->x, exp->y,
                                exp->width, exp->height);
            if(!exp->count)
                break;
            XNextEvent(proc->display, &ev);
        }
        zbar_window_redraw(proc->window);
        break;
    }

    case ConfigureNotify:
        zprintf(3, "resized to %d x %d\n",
                ev.xconfigure.width, ev.xconfigure.height);
        zbar_window_resize(proc->window,
                           ev.xconfigure.width, ev.xconfigure.height);
        _zbar_processor_invalidate(proc);
        break;

    case ClientMessage:
        if((ev.xclient.message_type ==
            XInternAtom(proc->display, "WM_PROTOCOLS", 0)) &&
           ev.xclient.format == 32 &&
           (ev.xclient.data.l[0] ==
            XInternAtom(proc->display, "WM_DELETE_WINDOW", 0))) {
            zprintf(3, "WM_DELETE_WINDOW\n");
            return(_zbar_processor_handle_input(proc, -1));
        }

    case KeyPress: {
        KeySym key = XLookupKeysym(&ev.xkey, 0);
        if(IsModifierKey(key))
            break;
        if((key & 0xff00) == 0xff00)
            key &= 0x00ff;
        zprintf(16, "KeyPress(%04lx)\n", key);
        /* FIXME this doesn't really work... */
        return(_zbar_processor_handle_input(proc, key & 0xffff));
    }
    case ButtonPress: {
        zprintf(16, "ButtonPress(%d)\n", ev.xbutton.button);
        int idx = 1;
        switch(ev.xbutton.button) {
        case Button2: idx = 2; break;
        case Button3: idx = 3; break;
        case Button4: idx = 4; break;
        case Button5: idx = 5; break;
        }
        return(_zbar_processor_handle_input(proc, idx));
    }

    case DestroyNotify:
        zprintf(16, "DestroyNotify\n");
        zbar_window_attach(proc->window, NULL, 0);
        proc->xwin = 0;
        return(0);

    default:
        /* ignored */;
    }
    return(0);
}

static int x_handle_events (zbar_processor_t *proc)
{
    int rc = 0;
    while(rc >= 0 && XPending(proc->display))
        rc = x_handle_event(proc);
    return(rc);
}

static int x_connection_handler (zbar_processor_t *proc,
                                 int i)
{
    _zbar_mutex_lock(&proc->mutex);
    _zbar_processor_lock(proc);
    _zbar_mutex_unlock(&proc->mutex);

    x_handle_events(proc);

    _zbar_mutex_lock(&proc->mutex);
    _zbar_processor_unlock(proc, 0);
    _zbar_mutex_unlock(&proc->mutex);
    return(0);
}

static int x_internal_handler (zbar_processor_t *proc,
                               int i)
{
    XProcessInternalConnection(proc->display, proc->state->polling.fds[i].fd);
    x_connection_handler(proc, i);
    return(0);
}

static void x_internal_watcher (Display *display,
                                XPointer client_arg,
                                int fd,
                                Bool opening,
                                XPointer *watch_arg)
{
    zbar_processor_t *proc = (void*)client_arg;
    if(opening)
        add_poll(proc, fd, x_internal_handler);
    else
        remove_poll(proc, fd);
}

int _zbar_processor_open (zbar_processor_t *proc,
                          char *title,
                          unsigned width,
                          unsigned height)
{
    proc->display = XOpenDisplay(NULL);
    if(!proc->display)
        return(err_capture_str(proc, SEV_ERROR, ZBAR_ERR_XDISPLAY, __func__,
                               "unable to open X display",
                               XDisplayName(NULL)));

    add_poll(proc, ConnectionNumber(proc->display), x_connection_handler);
    XAddConnectionWatch(proc->display, x_internal_watcher, (void*)proc);
    /* must also flush X event queue before polling */
    proc->state->pre_poll_handler = x_connection_handler;

    int screen = DefaultScreen(proc->display);
    XSetWindowAttributes attr;
    attr.event_mask = (ExposureMask | StructureNotifyMask |
                       KeyPressMask | ButtonPressMask);

    proc->xwin = XCreateWindow(proc->display,
                               RootWindow(proc->display, screen),
                               0, 0, width, height, 0,
                               CopyFromParent, InputOutput,
                               CopyFromParent, CWEventMask, &attr);
    if(!proc->xwin) {
        XCloseDisplay(proc->display);
        return(err_capture(proc, SEV_ERROR, ZBAR_ERR_XPROTO, __func__,
                           "creating window"));
    }

    XStoreName(proc->display, proc->xwin, title);

    XClassHint *class_hint = XAllocClassHint();
    class_hint->res_name = "zbar";
    class_hint->res_class = "zbar";
    XSetClassHint(proc->display, proc->xwin, class_hint);
    XFree(class_hint);
    class_hint = NULL;

    Atom wm_delete_window = XInternAtom(proc->display, "WM_DELETE_WINDOW", 0);
    if(wm_delete_window)
        XSetWMProtocols(proc->display, proc->xwin, &wm_delete_window, 1);

    if(zbar_window_attach(proc->window, proc->display, proc->xwin))
        return(err_copy(proc, proc->window));
    return(0);
}

int _zbar_processor_close (zbar_processor_t *proc)
{
    if(proc->window)
        zbar_window_attach(proc->window, NULL, 0);

    if(proc->display) {
        if(proc->xwin) {
            XDestroyWindow(proc->display, proc->xwin);
            proc->xwin = 0;
        }
        proc->state->pre_poll_handler = NULL;
        remove_poll(proc, ConnectionNumber(proc->display));
        XCloseDisplay(proc->display);
        proc->display = NULL;
    }
    return(0);
}

int _zbar_processor_invalidate (zbar_processor_t *proc)
{
    if(!proc->display || !proc->xwin)
        return(0);
    XClearArea(proc->display, proc->xwin, 0, 0, 0, 0, 1);
    XFlush(proc->display);
    return(0);
}

int _zbar_processor_set_size (zbar_processor_t *proc,
                              unsigned width,
                              unsigned height)
{
    if(!proc->display || !proc->xwin)
        return(0);

    /* refuse to resize greater than (default) screen size */
    XWindowAttributes attr;
    XGetWindowAttributes(proc->display, proc->xwin, &attr);

    int maxw = WidthOfScreen(attr.screen);
    int maxh = HeightOfScreen(attr.screen);
    int w, h;
    if(width > maxw) {
        h = (maxw * height + width - 1) / width;
        w = maxw;
    }
    else {
        w = width;
        h = height;
    }
    if(h > maxh) {
        w = (maxh * width + height - 1) / height;
        h = maxh;
    }
    assert(w <= maxw);
    assert(h <= maxh);

    XResizeWindow(proc->display, proc->xwin, w, h);
    XFlush(proc->display);
    return(0);
}

int _zbar_processor_set_visible (zbar_processor_t *proc,
                                 int visible)
{
    if(visible)
        XMapRaised(proc->display, proc->xwin);
    else
        XUnmapWindow(proc->display, proc->xwin);
    XFlush(proc->display);
    return(0);
}
