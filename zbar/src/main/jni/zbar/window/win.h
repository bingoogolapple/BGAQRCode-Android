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
#ifndef _WINDOW_WIN_H_
#define _WINDOW_WIN_H_

#include <windows.h>

struct window_state_s {
    HDC hdc;
    void* hdd;

    BITMAPINFOHEADER bih;

    /* pre-calculated logo geometries */
    int logo_scale;
    HRGN logo_zbars;
    HPEN logo_zpen, logo_zbpen;
    POINT logo_z[4];

    int font_height;
};

extern int _zbar_window_bih_init(zbar_window_t *w,
                                 zbar_image_t *img);

#endif
