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
#ifndef _IMG_SCANNER_H_
#define _IMG_SCANNER_H_

#include <zbar.h>

/* internal image scanner APIs for 2D readers */

extern zbar_symbol_t *_zbar_image_scanner_alloc_sym(zbar_image_scanner_t*,
                                                    zbar_symbol_type_t,
                                                    int);
extern void _zbar_image_scanner_add_sym(zbar_image_scanner_t*,
                                        zbar_symbol_t*);
extern void _zbar_image_scanner_recycle_syms(zbar_image_scanner_t*,
                                             zbar_symbol_t*);

#endif
