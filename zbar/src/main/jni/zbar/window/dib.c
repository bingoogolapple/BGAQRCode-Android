/*------------------------------------------------------------------------
 *  Copyright 2009 (c) Jeff Brown <spadix@users.sourceforge.net>
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
#include "image.h"
#include "win.h"

static int dib_cleanup (zbar_window_t *w)
{
    return(0);
}

static int dib_init (zbar_window_t *w,
                     zbar_image_t *img,
                     int new_format)
{
    if(new_format)
        _zbar_window_bih_init(w, img);

    window_state_t *win = w->state;
    w->dst_width = win->bih.biWidth = (img->width + 3) & ~3;
    w->dst_height = win->bih.biHeight = img->height;
    return(0);
}

static int dib_draw (zbar_window_t *w,
                     zbar_image_t *img)
{
    StretchDIBits(w->state->hdc,
                  w->scaled_offset.x, w->scaled_offset.y + w->scaled_size.y - 1,
                  w->scaled_size.x, -w->scaled_size.y,
                  0, 0, w->src_width, w->src_height,
                  (void*)img->data, (BITMAPINFO*)&w->state->bih,
                  DIB_RGB_COLORS, SRCCOPY);
    return(0);
}

static uint32_t dib_formats[] = {
    fourcc('B','G','R','3'),
    fourcc('B','G','R','4'),
    fourcc('J','P','E','G'),
    0
};

int _zbar_window_dib_init (zbar_window_t *w)
{
    uint32_t *fmt;
    for(fmt = dib_formats; *fmt; fmt++)
        _zbar_window_add_format(w, *fmt);

    w->init = dib_init;
    w->draw_image = dib_draw;
    w->cleanup = dib_cleanup;
    return(0);
}
