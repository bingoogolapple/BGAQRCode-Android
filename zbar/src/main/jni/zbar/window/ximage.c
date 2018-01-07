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
#include "x.h"
#include "image.h"

static int ximage_cleanup (zbar_window_t *w)
{
    window_state_t *x = w->state;
    if(x->img.x)
        free(x->img.x);
    x->img.x = NULL;
    return(0);
}

static inline int ximage_init (zbar_window_t *w,
                               zbar_image_t *img,
                               int format_change)
{
    ximage_cleanup(w);
    XImage *ximg = w->state->img.x = calloc(1, sizeof(XImage));
    ximg->width = img->width;
    ximg->height = img->height;
    ximg->format = ZPixmap;
    ximg->byte_order = LSBFirst;
    ximg->bitmap_unit = 8;
    ximg->bitmap_bit_order = MSBFirst;
    ximg->bitmap_pad = 8;

    const zbar_format_def_t *fmt = _zbar_format_lookup(w->format);
    if(fmt->group == ZBAR_FMT_RGB_PACKED) {
        ximg->depth = ximg->bits_per_pixel = fmt->p.rgb.bpp << 3;
        ximg->red_mask =
            (0xff >> RGB_SIZE(fmt->p.rgb.red))
            << RGB_OFFSET(fmt->p.rgb.red);
        ximg->green_mask =
            (0xff >> RGB_SIZE(fmt->p.rgb.green))
            << RGB_OFFSET(fmt->p.rgb.green);
        ximg->blue_mask =
            (0xff >> RGB_SIZE(fmt->p.rgb.blue))
            << RGB_OFFSET(fmt->p.rgb.blue);
    }
    else {
        ximg->depth = ximg->bits_per_pixel = 8;
        ximg->red_mask = ximg->green_mask = ximg->blue_mask = 0xff;
    }

    if(!XInitImage(ximg))
        return(err_capture_int(w, SEV_ERROR, ZBAR_ERR_XPROTO, __func__,
                               "unable to init XImage for format %x",
                               w->format));

    w->dst_width = img->width;
    w->dst_height = img->height;

    /* FIXME implement some basic scaling */
    w->scale_num = w->scale_den = 1;

    zprintf(3, "new XImage %.4s(%08" PRIx32 ") %dx%d"
            " from %.4s(%08" PRIx32 ") %dx%d\n",
            (char*)&w->format, w->format, ximg->width, ximg->height,
            (char*)&img->format, img->format, img->width, img->height);
    zprintf(4, "    masks: %08lx %08lx %08lx\n",
            ximg->red_mask, ximg->green_mask, ximg->blue_mask);
    return(0);
}

static int ximage_draw (zbar_window_t *w,
                        zbar_image_t *img)
{
    window_state_t *x = w->state;
    XImage *ximg = x->img.x;
    assert(ximg);
    ximg->data = (void*)img->data;

    point_t src = { 0, 0 };
    point_t dst = w->scaled_offset;
    if(dst.x < 0) {
        src.x = -dst.x;
        dst.x = 0;
    }
    if(dst.y < 0) {
        src.y = -dst.y;
        dst.y = 0;
    }
    point_t size = w->scaled_size;
    if(size.x > w->width)
        size.x = w->width;
    if(size.y > w->height)
        size.y = w->height;

    XPutImage(w->display, w->xwin, x->gc, ximg,
              src.x, src.y, dst.x, dst.y, size.x, size.y);
    ximg->data = NULL;
    return(0);
}

static uint32_t ximage_formats[4][5] = {
    {   /* 8bpp */
        /* FIXME fourcc('Y','8','0','0'), */
        fourcc('R','G','B','1'),
        fourcc('B','G','R','1'),
        0
    },
    {   /* 16bpp */
        fourcc('R','G','B','P'), fourcc('R','G','B','O'),
        fourcc('R','G','B','R'), fourcc('R','G','B','Q'),
        0
    },
    {   /* 24bpp */
        fourcc('R','G','B','3'),
        fourcc('B','G','R','3'),
        0
    },
    {   /* 32bpp */
        fourcc('R','G','B','4'),
        fourcc('B','G','R','4'),
        0
    },
};

static int ximage_probe_format (zbar_window_t *w,
                                uint32_t format)
{
    const zbar_format_def_t *fmt = _zbar_format_lookup(format);

    XVisualInfo visreq, *visuals = NULL;
    memset(&visreq, 0, sizeof(XVisualInfo));

    visreq.depth = fmt->p.rgb.bpp << 3;
    visreq.red_mask =
        (0xff >> RGB_SIZE(fmt->p.rgb.red)) << RGB_OFFSET(fmt->p.rgb.red);
    visreq.green_mask =
        (0xff >> RGB_SIZE(fmt->p.rgb.green)) << RGB_OFFSET(fmt->p.rgb.green);
    visreq.blue_mask =
        (0xff >> RGB_SIZE(fmt->p.rgb.blue)) << RGB_OFFSET(fmt->p.rgb.blue);

    int n;
    visuals = XGetVisualInfo(w->display,
                             VisualDepthMask | VisualRedMaskMask |
                             VisualGreenMaskMask | VisualBlueMaskMask,
                             &visreq, &n);

    zprintf(8, "bits=%d r=%08lx g=%08lx b=%08lx: n=%d visuals=%p\n",
            visreq.depth, visreq.red_mask, visreq.green_mask,
            visreq.blue_mask, n, visuals);
    if(!visuals)
        return(1);
    XFree(visuals);
    if(!n)
        return(-1);

    return(0);
}

int _zbar_window_probe_ximage (zbar_window_t *w)
{
    /* FIXME determine supported formats/depths */
    int n;
    XPixmapFormatValues *formats = XListPixmapFormats(w->display, &n);
    if(!formats)
        return(err_capture(w, SEV_ERROR, ZBAR_ERR_XPROTO, __func__,
                           "unable to query XImage formats"));

    int i;
    for(i = 0; i < n; i++) {
        if(formats[i].depth & 0x7 ||
           formats[i].depth > 0x20) {
            zprintf(2, "    [%d] depth=%d bpp=%d: not supported\n",
                    i, formats[i].depth, formats[i].bits_per_pixel);
            continue;
        }
        int fmtidx = formats[i].depth / 8 - 1;
        int j, n = 0;
        for(j = 0; ximage_formats[fmtidx][j]; j++)
            if(!ximage_probe_format(w, ximage_formats[fmtidx][j])) {
                zprintf(2, "    [%d] depth=%d bpp=%d: %.4s(%08" PRIx32 ")\n",
                        i, formats[i].depth, formats[i].bits_per_pixel,
                        (char*)&ximage_formats[fmtidx][j],
                        ximage_formats[fmtidx][j]);
                _zbar_window_add_format(w, ximage_formats[fmtidx][j]);
                n++;
            }
        if(!n)
            zprintf(2, "    [%d] depth=%d bpp=%d: no visuals\n",
                    i, formats[i].depth, formats[i].bits_per_pixel);
    }
    XFree(formats);

    if(!w->formats || !w->formats[0])
        return(err_capture(w, SEV_ERROR, ZBAR_ERR_UNSUPPORTED, __func__,
                           "no usable XImage formats found"));

    w->init = ximage_init;
    w->draw_image = ximage_draw;
    w->cleanup = ximage_cleanup;
    return(0);
}
