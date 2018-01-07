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
#include "image.h"
#include "win.h"
#include <ctype.h>

int _zbar_window_vfw_init(zbar_window_t *w);
int _zbar_window_dib_init(zbar_window_t *w);

int _zbar_window_resize (zbar_window_t *w)
{
    window_state_t *win = w->state;
    int lbw;
    if(w->height * 8 / 10 <= w->width)
        lbw = w->height / 36;
    else
        lbw = w->width * 5 / 144;
    if(lbw < 1)
        lbw = 1;
    win->logo_scale = lbw;
    zprintf(7, "%dx%d scale=%d\n", w->width, w->height, lbw);
    if(win->logo_zbars) {
        DeleteObject(win->logo_zbars);
        win->logo_zbars = NULL;
    }
    if(win->logo_zpen)
        DeleteObject(win->logo_zpen);
    if(win->logo_zbpen)
        DeleteObject(win->logo_zbpen);

    LOGBRUSH lb = { 0, };
    lb.lbStyle = BS_SOLID;
    lb.lbColor = RGB(0xd7, 0x33, 0x33);
    win->logo_zpen = ExtCreatePen(PS_GEOMETRIC | PS_SOLID |
                                  PS_ENDCAP_ROUND | PS_JOIN_ROUND,
                                  lbw * 2, &lb, 0, NULL);

    lb.lbColor = RGB(0xa4, 0x00, 0x00);
    win->logo_zbpen = ExtCreatePen(PS_GEOMETRIC | PS_SOLID |
                                   PS_ENDCAP_ROUND | PS_JOIN_ROUND,
                                   lbw * 2, &lb, 0, NULL);

    int x0 = w->width / 2;
    int y0 = w->height / 2;
    int by0 = y0 - 54 * lbw / 5;
    int bh = 108 * lbw / 5;

    static const int bx[5] = { -6, -3, -1,  2,  5 };
    static const int bw[5] = {  1,  1,  2,  2,  1 };

    int i;
    for(i = 0; i < 5; i++) {
        int x = x0 + lbw * bx[i];
        HRGN bar = CreateRectRgn(x, by0,
                                 x + lbw * bw[i], by0 + bh);
        if(win->logo_zbars) {
            CombineRgn(win->logo_zbars, win->logo_zbars, bar, RGN_OR);
            DeleteObject(bar);
        }
        else
            win->logo_zbars = bar;
    }

    static const int zx[4] = { -7,  7, -7,  7 };
    static const int zy[4] = { -8, -8,  8,  8 };

    for(i = 0; i < 4; i++) {
        win->logo_z[i].x = x0 + lbw * zx[i];
        win->logo_z[i].y = y0 + lbw * zy[i];
    }
    return(0);
}

int _zbar_window_attach (zbar_window_t *w,
                         void *display,
                         unsigned long unused)
{
    window_state_t *win = w->state;
    if(w->display) {
        /* FIXME cleanup existing resources */
        w->display = NULL;
    }

    if(!display) {
        if(win) {
            free(win);
            w->state = NULL;
        }
        return(0);
    }

    if(!win)
        win = w->state = calloc(1, sizeof(window_state_t));

    w->display = display;

    win->bih.biSize = sizeof(win->bih);
    win->bih.biPlanes = 1;

    HDC hdc = GetDC(w->display);
    if(!hdc)
        return(-1/*FIXME*/);
    win->bih.biXPelsPerMeter =
        1000L * GetDeviceCaps(hdc, HORZRES) / GetDeviceCaps(hdc, HORZSIZE);
    win->bih.biYPelsPerMeter =
        1000L * GetDeviceCaps(hdc, VERTRES) / GetDeviceCaps(hdc, VERTSIZE);

    int height = -MulDiv(11, GetDeviceCaps(hdc, LOGPIXELSY), 96);
    HFONT font = CreateFontW(height, 0, 0, 0, 0, 0, 0, 0,
                             ANSI_CHARSET, 0, 0, 0,
                             FF_MODERN | FIXED_PITCH, NULL);

    SelectObject(hdc, font);
    DeleteObject(font);

    TEXTMETRIC tm;
    GetTextMetrics(hdc, &tm);
    win->font_height = tm.tmHeight;

    ReleaseDC(w->display, hdc);

    return(_zbar_window_dib_init(w));
}

int _zbar_window_begin (zbar_window_t *w)
{
    HDC hdc = w->state->hdc = GetDC(w->display);
    if(!hdc || !SaveDC(hdc))
        return(-1/*FIXME*/);
    return(0);
}

int _zbar_window_end (zbar_window_t *w)
{
    HDC hdc = w->state->hdc;
    w->state->hdc = NULL;
    RestoreDC(hdc, -1);
    ReleaseDC(w->display, hdc);
    ValidateRect(w->display, NULL);
    return(0);
}

int _zbar_window_clear (zbar_window_t *w)
{
    HDC hdc = GetDC(w->display);
    if(!hdc)
        return(-1/*FIXME*/);

    RECT r = { 0, 0, w->width, w->height };
    FillRect(hdc, &r, GetStockObject(BLACK_BRUSH));

    ReleaseDC(w->display, hdc);
    ValidateRect(w->display, NULL);
    return(0);
}

static inline void win_set_rgb (HDC hdc,
                                uint32_t rgb)
{
    SelectObject(hdc, GetStockObject(DC_PEN));
    SetDCPenColor(hdc, RGB((rgb & 4) * 0x33,
                           (rgb & 2) * 0x66,
                           (rgb & 1) * 0xcc));
}

int _zbar_window_draw_polygon (zbar_window_t *w,
                               uint32_t rgb,
                               const point_t *pts,
                               int npts)
{
    HDC hdc = w->state->hdc;
    win_set_rgb(hdc, rgb);

    point_t org = w->scaled_offset;
    POINT gdipts[npts + 1];
    int i;
    for(i = 0; i < npts; i++) {
        point_t p = window_scale_pt(w, pts[i]);
        gdipts[i].x = p.x + org.x;
        gdipts[i].y = p.y + org.y;
    }
    gdipts[npts] = gdipts[0];

    Polyline(hdc, gdipts, npts + 1);
    return(0);
}

int _zbar_window_draw_marker (zbar_window_t *w,
                              uint32_t rgb,
                              point_t p)
{
    HDC hdc = w->state->hdc;
    win_set_rgb(hdc, rgb);

    static const DWORD npolys[3] = { 5, 2, 2 };
    POINT polys[9] = {
        { p.x - 2, p.y - 2 },
        { p.x - 2, p.y + 2 },
        { p.x + 2, p.y + 2 },
        { p.x + 2, p.y - 2 },
        { p.x - 2, p.y - 2 },

        { p.x - 3, p.y },
        { p.x + 4, p.y },

        { p.x, p.y - 3 },
        { p.x, p.y + 4 },
    };

    PolyPolyline(hdc, polys, npolys, 3);
    return(0);
}

int _zbar_window_draw_text (zbar_window_t *w,
                            uint32_t rgb,
                            point_t p,
                            const char *text)
{
    HDC hdc = w->state->hdc;
    SetTextColor(hdc, RGB((rgb & 4) * 0x33,
                          (rgb & 2) * 0x66,
                          (rgb & 1) * 0xcc));
    SetBkMode(hdc, TRANSPARENT);

    int n = 0;
    while(n < 32 && text[n] && isprint(text[n]))
        n++;

    if(p.x >= 0)
        SetTextAlign(hdc, TA_BASELINE | TA_CENTER);
    else {
        SetTextAlign(hdc, TA_BASELINE | TA_RIGHT);
        p.x += w->width;
    }

    if(p.y < 0)
        p.y = w->height + p.y * w->state->font_height * 5 / 4;

    TextOut(hdc, p.x, p.y, text, n);
    return(0);
}

int _zbar_window_fill_rect (zbar_window_t *w,
                            uint32_t rgb,
                            point_t org,
                            point_t size)
{
    HDC hdc = w->state->hdc;
    SetDCBrushColor(hdc, RGB((rgb & 4) * 0x33,
                             (rgb & 2) * 0x66,
                             (rgb & 1) * 0xcc));

    RECT r = { org.x, org.y, org.x + size.x, org.y + size.y };

    FillRect(hdc, &r, GetStockObject(DC_BRUSH));
    return(0);
}

int _zbar_window_draw_logo (zbar_window_t *w)
{
    HDC hdc = w->state->hdc;

    window_state_t *win = w->state;

    /* FIXME buffer offscreen */
    HRGN rgn = CreateRectRgn(0, 0, w->width, w->height);
    CombineRgn(rgn, rgn, win->logo_zbars, RGN_DIFF);
    FillRgn(hdc, rgn, GetStockObject(WHITE_BRUSH));
    DeleteObject(rgn);

    FillRgn(hdc, win->logo_zbars, GetStockObject(BLACK_BRUSH));

    SelectObject(hdc, win->logo_zpen);
    Polyline(hdc, win->logo_z, 4);

    ExtSelectClipRgn(hdc, win->logo_zbars, RGN_AND);
    SelectObject(hdc, win->logo_zbpen);
    Polyline(hdc, win->logo_z, 4);
    return(0);
}

int _zbar_window_bih_init (zbar_window_t *w,
                           zbar_image_t *img)
{
    window_state_t *win = w->state;
    switch(w->format) {
    case fourcc('J','P','E','G'): {
        win->bih.biBitCount = 0;
        win->bih.biCompression = BI_JPEG;
        break;
    }
    case fourcc('B','G','R','3'): {
        win->bih.biBitCount = 24;
        win->bih.biCompression = BI_RGB;
        break;
    }
    case fourcc('B','G','R','4'): {
        win->bih.biBitCount = 32;
        win->bih.biCompression = BI_RGB;
        break;
    }
    default:
        assert(0);
        /* FIXME PNG? */
    }
    win->bih.biSizeImage = img->datalen;

    zprintf(20, "biCompression=%d biBitCount=%d\n",
            (int)win->bih.biCompression, win->bih.biBitCount);

    return(0);
}
