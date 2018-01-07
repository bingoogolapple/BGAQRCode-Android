/*------------------------------------------------------------------------
 *  Copyright 2010 (c) Jeff Brown <spadix@users.sourceforge.net>
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
#ifndef _CODE93_H_
#define _CODE93_H_

/* Code 93 specific decode state */
typedef struct code93_decoder_s {
    unsigned direction : 1;     /* scan direction: 0=fwd/space, 1=rev/bar */
    unsigned element : 3;       /* element offset 0-5 */
    int character : 12;         /* character position in symbol */
    unsigned width;             /* last character width */
    unsigned char buf;          /* first character */

    unsigned config;
    int configs[NUM_CFGS];      /* int valued configurations */
} code93_decoder_t;

/* reset Code 93 specific state */
static inline void code93_reset (code93_decoder_t *dcode93)
{
    dcode93->direction = 0;
    dcode93->element = 0;
    dcode93->character = -1;
}

/* decode Code 93 symbols */
zbar_symbol_type_t _zbar_decode_code93(zbar_decoder_t *dcode);

#endif
