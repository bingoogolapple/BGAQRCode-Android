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
#ifndef _WINDOW_X_H_
#define _WINDOW_X_H_

#include "window.h"

#ifdef HAVE_X
# include <X11/Xlib.h>
# include <X11/Xutil.h>
# ifdef HAVE_X11_EXTENSIONS_XSHM_H
#  include <X11/extensions/XShm.h>
# endif
#ifdef HAVE_X11_EXTENSIONS_XVLIB_H
#  include <X11/extensions/Xvlib.h>
#endif
#endif

struct window_state_s {
    unsigned long colors[8];    /* pre-allocated colors */

    GC gc;                      /* graphics context */
    Region exposed;             /* region to redraw */
    XFontStruct *font;          /* overlay font */

    /* pre-calculated logo geometries */
    int logo_scale;
    unsigned long logo_colors[2];
    Region logo_zbars;
    XPoint logo_z[4];
    XRectangle logo_bars[5];

#ifdef HAVE_X11_EXTENSIONS_XSHM_H
    XShmSegmentInfo shm;        /* shared memory segment */
#endif

    union {
        XImage *x;
#ifdef HAVE_X11_EXTENSIONS_XVLIB_H
        XvImage *xv;
#endif
    } img;

    XID img_port;               /* current format port */
    XID *xv_ports;              /* best port for format */
    int num_xv_adaptors;        /* number of adaptors */
    XID *xv_adaptors;           /* port grabbed for each adaptor */
};

extern int _zbar_window_probe_ximage(zbar_window_t*);
extern int _zbar_window_probe_xshm(zbar_window_t*);
extern int _zbar_window_probe_xv(zbar_window_t*);

#endif
