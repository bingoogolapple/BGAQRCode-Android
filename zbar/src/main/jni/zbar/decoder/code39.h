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
#ifndef _CODE39_H_
#define _CODE39_H_

/* Code 39 specific decode state */
typedef struct code39_decoder_s {
    unsigned direction : 1;     /* scan direction: 0=fwd, 1=rev */
    unsigned element : 4;       /* element offset 0-8 */
    int character : 12;         /* character position in symbol */
    unsigned s9;                /* current character width */
    unsigned width;             /* last character width */

    unsigned config;
    int configs[NUM_CFGS];      /* int valued configurations */
} code39_decoder_t;

/* reset Code 39 specific state */
static inline void code39_reset (code39_decoder_t *dcode39)
{
    dcode39->direction = 0;
    dcode39->element = 0;
    dcode39->character = -1;
    dcode39->s9 = 0;
}

/* decode Code 39 symbols */
zbar_symbol_type_t _zbar_decode_code39(zbar_decoder_t *dcode);

#endif
