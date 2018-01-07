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
#ifndef _I25_H_
#define _I25_H_

/* interleaved 2 of 5 specific decode state */
typedef struct i25_decoder_s {
    unsigned direction : 1;     /* scan direction: 0=fwd/space, 1=rev/bar */
    unsigned element : 4;       /* element offset 0-8 */
    int character : 12;         /* character position in symbol */
    unsigned s10;               /* current character width */
    unsigned width;             /* last character width */
    unsigned char buf[4];       /* initial scan buffer */

    unsigned config;
    int configs[NUM_CFGS];      /* int valued configurations */
} i25_decoder_t;

/* reset interleaved 2 of 5 specific state */
static inline void i25_reset (i25_decoder_t *i25)
{
    i25->direction = 0;
    i25->element = 0;
    i25->character = -1;
    i25->s10 = 0;
}

/* decode interleaved 2 of 5 symbols */
zbar_symbol_type_t _zbar_decode_i25(zbar_decoder_t *dcode);

#endif
