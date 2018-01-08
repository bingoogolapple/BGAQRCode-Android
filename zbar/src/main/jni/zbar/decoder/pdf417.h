/*------------------------------------------------------------------------
 *  Copyright 2008-2009 (c) Jeff Brown <spadix@users.sourceforge.net>
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
#ifndef _PDF417_H_
#define _PDF417_H_

/* PDF417 specific decode state */
typedef struct pdf417_decoder_s {
    unsigned direction : 1;     /* scan direction: 0=fwd/space, 1=rev/bar */
    unsigned element : 3;       /* element offset 0-7 */
    int character : 12;         /* character position in symbol */
    unsigned s8;                /* character width */

    unsigned config;
    int configs[NUM_CFGS];      /* int valued configurations */
} pdf417_decoder_t;

/* reset PDF417 specific state */
static inline void pdf417_reset (pdf417_decoder_t *pdf417)
{
    pdf417->direction = 0;
    pdf417->element = 0;
    pdf417->character = -1;
    pdf417->s8 = 0;
}

/* decode PDF417 symbols */
zbar_symbol_type_t _zbar_decode_pdf417(zbar_decoder_t *dcode);

#endif
