/*------------------------------------------------------------------------
 *  Copyright 2011 (c) Jeff Brown <spadix@users.sourceforge.net>
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
#ifndef _CODABAR_H_
#define _CODABAR_H_

/* Codabar specific decode state */
typedef struct codabar_decoder_s {
    unsigned direction : 1;     /* scan direction: 0=fwd, 1=rev */
    unsigned element : 4;       /* element offset 0-7 */
    int character : 12;         /* character position in symbol */
    unsigned s7;                /* current character width */
    unsigned width;             /* last character width */
    unsigned char buf[6];       /* initial scan buffer */

    unsigned config;
    int configs[NUM_CFGS];      /* int valued configurations */
} codabar_decoder_t;

/* reset Codabar specific state */
static inline void codabar_reset (codabar_decoder_t *codabar)
{
    codabar->direction = 0;
    codabar->element = 0;
    codabar->character = -1;
    codabar->s7 = 0;
}

/* decode Codabar symbols */
zbar_symbol_type_t _zbar_decode_codabar(zbar_decoder_t *dcode);

#endif
