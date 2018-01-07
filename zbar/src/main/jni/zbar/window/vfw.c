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
#include <vfw.h>

extern int _zbar_window_bih_init(zbar_window_t *w,
                                 zbar_image_t *img);

static int vfw_cleanup (zbar_window_t *w)
{
    if(w->hdd) {
        DrawDibClose(w->hdd);
        w->hdd = NULL;
    }
    return(0);
}

static int vfw_init (zbar_window_t *w,
                     zbar_image_t *img,
                     int new_format)
{
    if(new_format)
        _zbar_window_bih_init(w, img);

    w->dst_width = w->bih.biWidth = (img->width + 3) & ~3;
    w->dst_height = w->bih.biHeight = img->height;

    HDC hdc = GetDC(w->hwnd);
    if(!hdc)
        return(-1/*FIXME*/);

    if(!DrawDibBegin(w->hdd, hdc, w->width, w->height,
                     &w->bih, img->width, img->height, 0))
        return(-1/*FIXME*/);

    ReleaseDC(w->hwnd, hdc);
    return(0);
}

static int vfw_draw (zbar_window_t *w,
                     zbar_image_t *img)
{
    HDC hdc = GetDC(w->hwnd);
    if(!hdc)
        return(-1/*FIXME*/);

    zprintf(24, "DrawDibDraw(%dx%d -> %dx%d)\n",
            img->width, img->height, w->width, w->height);

    DrawDibDraw(w->hdd, hdc,
                0, 0, w->width, w->height,
                &w->bih, (void*)img->data,
                0, 0, w->src_width, w->src_height,
                DDF_SAME_DRAW);

    ValidateRect(w->hwnd, NULL);
    ReleaseDC(w->hwnd, hdc);
    return(0);
}

static uint32_t vfw_formats[] = {
    fourcc('B','G','R','3'),
    fourcc('B','G','R','4'),
    fourcc('J','P','E','G'),
    0
};

int _zbar_window_vfw_init (zbar_window_t *w)
{
    w->hdd = DrawDibOpen();
    if(!w->hdd)
        return(err_capture(w, SEV_ERROR, ZBAR_ERR_UNSUPPORTED, __func__,
                           "unable to initialize DrawDib"));

    uint32_t *fmt;
    for(fmt = vfw_formats; *fmt; fmt++)
        _zbar_window_add_format(w, *fmt);

    w->init = vfw_init;
    w->draw_image = vfw_draw;
    w->cleanup = vfw_cleanup;
    return(0);
}
