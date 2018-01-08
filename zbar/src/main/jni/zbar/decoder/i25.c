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
#include <string.h>     /* memmove */

#include <zbar.h>

#ifdef DEBUG_I25
# define DEBUG_LEVEL (DEBUG_I25)
#endif
#include "debug.h"
#include "decoder.h"

static inline unsigned char i25_decode1 (unsigned char enc,
                                         unsigned e,
                                         unsigned s)
{
    unsigned char E = decode_e(e, s, 45);
    if(E > 7)
        return(0xff);
    enc <<= 1;
    if(E > 2)
        enc |= 1;
    return(enc);
}

static inline unsigned char i25_decode10 (zbar_decoder_t *dcode,
                                          unsigned char offset)
{
    i25_decoder_t *dcode25 = &dcode->i25;
    dbprintf(2, " s=%d", dcode25->s10);
    if(dcode25->s10 < 10)
        return(0xff);

    /* threshold bar width ratios */
    unsigned char enc = 0, par = 0;
    signed char i;
    for(i = 8; i >= 0; i -= 2) {
        unsigned char j = offset + ((dcode25->direction) ? i : 8 - i);
        enc = i25_decode1(enc, get_width(dcode, j), dcode25->s10);
        if(enc == 0xff)
            return(0xff);
        if(enc & 1)
            par++;
    }

    dbprintf(2, " enc=%02x par=%x", enc, par);

    /* parity check */
    if(par != 2) {
        dbprintf(2, " [bad parity]");
        return(0xff);
    }

    /* decode binary weights */
    enc &= 0xf;
    if(enc & 8) {
        if(enc == 12)
            enc = 0;
        else if(--enc > 9) {
            dbprintf(2, " [invalid encoding]");
            return(0xff);
        }
    }

    dbprintf(2, " => %x", enc);
    return(enc);
}

static inline signed char i25_decode_start (zbar_decoder_t *dcode)
{
    i25_decoder_t *dcode25 = &dcode->i25;
    if(dcode25->s10 < 10)
        return(ZBAR_NONE);

    unsigned char enc = 0;
    unsigned char i = 10;
    enc = i25_decode1(enc, get_width(dcode, i++), dcode25->s10);
    enc = i25_decode1(enc, get_width(dcode, i++), dcode25->s10);
    enc = i25_decode1(enc, get_width(dcode, i++), dcode25->s10);

    if((get_color(dcode) == ZBAR_BAR)
       ? enc != 4
       : (enc = i25_decode1(enc, get_width(dcode, i++), dcode25->s10))) {
        dbprintf(4, "      i25: s=%d enc=%x [invalid]\n", dcode25->s10, enc);
        return(ZBAR_NONE);
    }

    /* check leading quiet zone - spec is 10n(?)
     * we require 5.25n for w=2n to 6.75n for w=3n
     * (FIXME should really factor in w:n ratio)
     */
    unsigned quiet = get_width(dcode, i);
    if(quiet && quiet < dcode25->s10 * 3 / 8) {
        dbprintf(3, "      i25: s=%d enc=%x q=%d [invalid qz]\n",
                 dcode25->s10, enc, quiet);
        return(ZBAR_NONE);
    }

    dcode25->direction = get_color(dcode);
    dcode25->element = 1;
    dcode25->character = 0;
    return(ZBAR_PARTIAL);
}

static inline int i25_acquire_lock (zbar_decoder_t *dcode)
{
    int i;
    /* lock shared resources */
    if(acquire_lock(dcode, ZBAR_I25)) {
        dcode->i25.character = -1;
        return(1);
    }

    /* copy holding buffer */
    for(i = 4; --i >= 0; )
        dcode->buf[i] = dcode->i25.buf[i];
    return(0);
}

static inline signed char i25_decode_end (zbar_decoder_t *dcode)
{
    i25_decoder_t *dcode25 = &dcode->i25;

    /* check trailing quiet zone */
    unsigned quiet = get_width(dcode, 0);
    if((quiet && quiet < dcode25->width * 3 / 8) ||
       decode_e(get_width(dcode, 1), dcode25->width, 45) > 2 ||
       decode_e(get_width(dcode, 2), dcode25->width, 45) > 2) {
        dbprintf(3, "      i25: s=%d q=%d [invalid qz]\n",
                 dcode25->width, quiet);
        return(ZBAR_NONE);
    }

    /* check exit condition */
    unsigned char E = decode_e(get_width(dcode, 3), dcode25->width, 45);
    if((!dcode25->direction)
       ? E - 3 > 4
       : (E > 2 ||
          decode_e(get_width(dcode, 4), dcode25->width, 45) > 2))
        return(ZBAR_NONE);

    if(dcode25->character <= 4 &&
       i25_acquire_lock(dcode))
        return(ZBAR_PARTIAL);

    dcode->direction = 1 - 2 * dcode25->direction;
    if(dcode25->direction) {
        /* reverse buffer */
        dbprintf(2, " (rev)");
        int i;
        for(i = 0; i < dcode25->character / 2; i++) {
            unsigned j = dcode25->character - 1 - i;
            char c = dcode->buf[i];
            dcode->buf[i] = dcode->buf[j];
            dcode->buf[j] = c;
        }
    }

    if(dcode25->character < CFG(*dcode25, ZBAR_CFG_MIN_LEN) ||
       (CFG(*dcode25, ZBAR_CFG_MAX_LEN) > 0 &&
        dcode25->character > CFG(*dcode25, ZBAR_CFG_MAX_LEN))) {
        dbprintf(2, " [invalid len]\n");
        release_lock(dcode, ZBAR_I25);
        dcode25->character = -1;
        return(ZBAR_NONE);
    }

    zassert(dcode25->character < dcode->buf_alloc, ZBAR_NONE, "i=%02x %s\n",
            dcode25->character,
            _zbar_decoder_buf_dump(dcode->buf, dcode25->character));
    dcode->buflen = dcode25->character;
    dcode->buf[dcode25->character] = '\0';
    dcode->modifiers = 0;
    dbprintf(2, " [valid end]\n");
    dcode25->character = -1;
    return(ZBAR_I25);
}

zbar_symbol_type_t _zbar_decode_i25 (zbar_decoder_t *dcode)
{
    i25_decoder_t *dcode25 = &dcode->i25;

    /* update latest character width */
    dcode25->s10 -= get_width(dcode, 10);
    dcode25->s10 += get_width(dcode, 0);

    if(dcode25->character < 0 &&
       !i25_decode_start(dcode))
        return(ZBAR_NONE);

    if(--dcode25->element == 6 - dcode25->direction)
        return(i25_decode_end(dcode));
    else if(dcode25->element)
        return(ZBAR_NONE);

    /* FIXME check current character width against previous */
    dcode25->width = dcode25->s10;

    dbprintf(2, "      i25[%c%02d+%x]",
             (dcode25->direction) ? '<' : '>',
             dcode25->character, dcode25->element);

    if(dcode25->character == 4 && i25_acquire_lock(dcode))
        return(ZBAR_PARTIAL);

    unsigned char c = i25_decode10(dcode, 1);
    dbprintf(2, " c=%x", c);
    if(c > 9) {
        dbprintf(2, " [aborted]\n");
        goto reset;
    }

    if(size_buf(dcode, dcode25->character + 3)) {
        dbprintf(2, " [overflow]\n");
        goto reset;
    }

    unsigned char *buf;
    if(dcode25->character >= 4)
        buf = dcode->buf;
    else
        buf = dcode25->buf;
    buf[dcode25->character++] = c + '0';

    c = i25_decode10(dcode, 0);
    dbprintf(2, " c=%x", c);
    if(c > 9) {
        dbprintf(2, " [aborted]\n");
        goto reset;
    }
    else
        dbprintf(2, "\n");

    buf[dcode25->character++] = c + '0';
    dcode25->element = 10;
    return((dcode25->character == 2) ? ZBAR_PARTIAL : ZBAR_NONE);

reset:
    if(dcode25->character >= 4)
        release_lock(dcode, ZBAR_I25);
    dcode25->character = -1;
    return(ZBAR_NONE);
}
