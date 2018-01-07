/*------------------------------------------------------------------------
 *  Copyright 2008-2010 (c) Jeff Brown <spadix@users.sourceforge.net>
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

#include <config.h>

#include <zbar.h>

#ifdef DEBUG_PDF417
# define DEBUG_LEVEL (DEBUG_PDF417)
#endif
#include "debug.h"
#include "decoder.h"

#include "pdf417_hash.h"

#define PDF417_STOP 0xbff

static inline signed short pdf417_decode8 (zbar_decoder_t *dcode)
{
    /* build edge signature of character
     * from similar edge measurements
     */
    unsigned s = dcode->pdf417.s8;
    dbprintf(2, " s=%d ", s);
    if(s < 8)
        return(-1);

    long sig = 0;
    signed char e;
    unsigned char i;
    for(i = 0; i < 7; i++) {
        if(get_color(dcode) == ZBAR_SPACE)
            e = decode_e(get_width(dcode, i) +
                         get_width(dcode, i + 1), s, 17);
        else
            e = decode_e(get_width(dcode, 7 - i) +
                         get_width(dcode, 6 - i), s, 17);
        dbprintf(4, "%x", e);
        if(e < 0 || e > 8)
            return(-1);
        sig = (sig << 3) ^ e;
    }
    dbprintf(2, " sig=%06lx", sig);

    /* determine cluster number */
    int clst = ((sig & 7) - ((sig >> 3) & 7) +
                ((sig >> 12) & 7) - ((sig >> 15) & 7));
    if(clst < 0)
        clst += 9;
    dbprintf(2, " k=%d", clst);
    zassert(clst >= 0 && clst < 9, -1, "dir=%x sig=%lx k=%x %s\n",
            dcode->pdf417.direction, sig, clst,
            _zbar_decoder_buf_dump(dcode->buf, dcode->pdf417.character));

    if(clst != 0 && clst != 3 && clst != 6) {
        if(get_color(dcode) && clst == 7 && sig == 0x080007)
            return(PDF417_STOP);
        return(-1);
    }

    signed short g[3];
    sig &= 0x3ffff;
    g[0] = pdf417_hash[(sig - (sig >> 10)) & PDF417_HASH_MASK];
    g[1] = pdf417_hash[((sig >> 8) - sig) & PDF417_HASH_MASK];
    g[2] = pdf417_hash[((sig >> 14) - (sig >> 1)) & PDF417_HASH_MASK];
    zassert(g[0] >= 0 && g[1] >= 0 && g[2] >= 0, -1,
            "dir=%x sig=%lx k=%x g0=%03x g1=%03x g2=%03x %s\n",
            dcode->pdf417.direction, sig, clst, g[0], g[1], g[2],
            _zbar_decoder_buf_dump(dcode->buf, dcode->pdf417.character));

    unsigned short c = (g[0] + g[1] + g[2]) & PDF417_HASH_MASK;
    dbprintf(2, " g0=%x g1=%x g2=%x c=%03d(%d)",
             g[0], g[1], g[2], c & 0x3ff, c >> 10);
    return(c);
}

static inline signed char pdf417_decode_start(zbar_decoder_t *dcode)
{
    unsigned s = dcode->pdf417.s8;
    if(s < 8)
        return(0);

    int ei = decode_e(get_width(dcode, 0) + get_width(dcode, 1), s, 17);
    int ex = (get_color(dcode) == ZBAR_SPACE) ? 2 : 6;
    if(ei != ex)
        return(0);

    ei = decode_e(get_width(dcode, 1) + get_width(dcode, 2), s, 17);
    if(ei)
        return(0);

    ei = decode_e(get_width(dcode, 2) + get_width(dcode, 3), s, 17);
    ex = (get_color(dcode) == ZBAR_SPACE) ? 0 : 2;
    if(ei != ex)
        return(0);

    ei = decode_e(get_width(dcode, 3) + get_width(dcode, 4), s, 17);
    ex = (get_color(dcode) == ZBAR_SPACE) ? 0 : 2;
    if(ei != ex)
        return(0);

    ei = decode_e(get_width(dcode, 4) + get_width(dcode, 5), s, 17);
    if(ei)
        return(0);

    ei = decode_e(get_width(dcode, 5) + get_width(dcode, 6), s, 17);
    if(ei)
        return(0);

    ei = decode_e(get_width(dcode, 6) + get_width(dcode, 7), s, 17);
    ex = (get_color(dcode) == ZBAR_SPACE) ? 7 : 1;
    if(ei != ex)
        return(0);

    ei = decode_e(get_width(dcode, 7) + get_width(dcode, 8), s, 17);
    ex = (get_color(dcode) == ZBAR_SPACE) ? 8 : 1;

    if(get_color(dcode) == ZBAR_BAR) {
        /* stop character has extra bar */
        if(ei != 1)
            return(0);
        ei = decode_e(get_width(dcode, 8) + get_width(dcode, 9), s, 17);
    }

    dbprintf(2, "      pdf417[%c]: s=%d",
             (get_color(dcode)) ? '<' : '>', s);

    /* check quiet zone */
    if(ei >= 0 && ei < ex) {
        dbprintf(2, " [invalid quiet]\n");
        return(0);
    }

    /* lock shared resources */
    if(acquire_lock(dcode, ZBAR_PDF417)) {
        dbprintf(2, " [locked %d]\n", dcode->lock);
        return(0);
    }

    pdf417_decoder_t *dcode417 = &dcode->pdf417;
    dcode417->direction = get_color(dcode);
    dcode417->element = 0;
    dcode417->character = 0;

    dbprintf(2, " [valid start]\n");
    return(ZBAR_PARTIAL);
}

zbar_symbol_type_t _zbar_decode_pdf417 (zbar_decoder_t *dcode)
{
    pdf417_decoder_t *dcode417 = &dcode->pdf417;

    /* update latest character width */
    dcode417->s8 -= get_width(dcode, 8);
    dcode417->s8 += get_width(dcode, 0);

    if(dcode417->character < 0) {
        pdf417_decode_start(dcode);
        dbprintf(4, "\n");
        return(0);
    }

    /* process every 8th element of active symbol */
    if(++dcode417->element)
        return(0);
    dcode417->element = 0;

    dbprintf(2, "      pdf417[%c%02d]:",
             (dcode417->direction) ? '<' : '>', dcode417->character);

    if(get_color(dcode) != dcode417->direction) {
        int c = dcode417->character;
        release_lock(dcode, ZBAR_PDF417);
        dcode417->character = -1;
        zassert(get_color(dcode) == dcode417->direction, ZBAR_NONE,
                "color=%x dir=%x char=%d elem=0 %s\n",
                get_color(dcode), dcode417->direction, c,
                _zbar_decoder_buf_dump(dcode->buf, c));
    }

    signed short c = pdf417_decode8(dcode);
    if(c < 0 || size_buf(dcode, dcode417->character + 1)) {
        dbprintf(1, (c < 0) ? " [aborted]\n" : " [overflow]\n");
        release_lock(dcode, ZBAR_PDF417);
        dcode417->character = -1;
        return(0);
    }

    /* FIXME TBD infer dimensions, save codewords */

    if(c == PDF417_STOP) {
        dbprintf(1, " [valid stop]");
        /* FIXME check trailing bar and qz */
        dcode->direction = 1 - 2 * dcode417->direction;
        dcode->modifiers = 0;
        release_lock(dcode, ZBAR_PDF417);
        dcode417->character = -1;
    }

    dbprintf(2, "\n");
    return(0);
}
