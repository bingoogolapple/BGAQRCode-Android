/*------------------------------------------------------------------------
 *  Copyright 2007-2010 (c) Jeff Brown <spadix@users.sourceforge.net>
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
#ifndef _DECODER_H_
#define _DECODER_H_

#include <config.h>
#include <stdlib.h>     /* realloc */
#include <limits.h>

#include <zbar.h>

#include "debug.h"

#define NUM_CFGS (ZBAR_CFG_MAX_LEN - ZBAR_CFG_MIN_LEN + 1)

#ifdef ENABLE_EAN
# include "decoder/ean.h"
#endif
#ifdef ENABLE_I25
# include "decoder/i25.h"
#endif
#ifdef ENABLE_DATABAR
# include "decoder/databar.h"
#endif
#ifdef ENABLE_CODABAR
# include "decoder/codabar.h"
#endif
#ifdef ENABLE_CODE39
# include "decoder/code39.h"
#endif
#ifdef ENABLE_CODE93
# include "decoder/code93.h"
#endif
#ifdef ENABLE_CODE128
# include "decoder/code128.h"
#endif
#ifdef ENABLE_PDF417
# include "decoder/pdf417.h"
#endif
#ifdef ENABLE_QRCODE
# include "decoder/qr_finder.h"
#endif

/* size of bar width history (implementation assumes power of two) */
#ifndef DECODE_WINDOW
# define DECODE_WINDOW  16
#endif

/* initial data buffer allocation */
#ifndef BUFFER_MIN
# define BUFFER_MIN   0x20
#endif

/* maximum data buffer allocation
 * (longer symbols are rejected)
 */
#ifndef BUFFER_MAX
# define BUFFER_MAX  0x100
#endif

/* buffer allocation increment */
#ifndef BUFFER_INCR
# define BUFFER_INCR  0x10
#endif

#define CFG(dcode, cfg) ((dcode).configs[(cfg) - ZBAR_CFG_MIN_LEN])
#define TEST_CFG(config, cfg) (((config) >> (cfg)) & 1)
#define MOD(mod) (1 << (mod))

/* symbology independent decoder state */
struct zbar_decoder_s {
    unsigned char idx;                  /* current width index */
    unsigned w[DECODE_WINDOW];          /* window of last N bar widths */
    zbar_symbol_type_t type;            /* type of last decoded data */
    zbar_symbol_type_t lock;            /* buffer lock */
    unsigned modifiers;                 /* symbology modifier */
    int direction;                      /* direction of last decoded data */
    unsigned s6;                        /* 6-element character width */

    /* everything above here is automatically reset */
    unsigned buf_alloc;                 /* dynamic buffer allocation */
    unsigned buflen;                    /* binary data length */
    unsigned char *buf;                 /* decoded characters */
    void *userdata;                     /* application data */
    zbar_decoder_handler_t *handler;    /* application callback */

    /* symbology specific state */
#ifdef ENABLE_EAN
    ean_decoder_t ean;                  /* EAN/UPC parallel decode attempts */
#endif
#ifdef ENABLE_I25
    i25_decoder_t i25;                  /* Interleaved 2 of 5 decode state */
#endif
#ifdef ENABLE_DATABAR
    databar_decoder_t databar;          /* DataBar decode state */
#endif
#ifdef ENABLE_CODABAR
    codabar_decoder_t codabar;          /* Codabar decode state */
#endif
#ifdef ENABLE_CODE39
    code39_decoder_t code39;            /* Code 39 decode state */
#endif
#ifdef ENABLE_CODE93
    code93_decoder_t code93;            /* Code 93 decode state */
#endif
#ifdef ENABLE_CODE128
    code128_decoder_t code128;          /* Code 128 decode state */
#endif
#ifdef ENABLE_PDF417
    pdf417_decoder_t pdf417;            /* PDF417 decode state */
#endif
#ifdef ENABLE_QRCODE
    qr_finder_t qrf;                    /* QR Code finder state */
#endif
};

/* return current element color */
static inline char get_color (const zbar_decoder_t *dcode)
{
    return(dcode->idx & 1);
}

/* retrieve i-th previous element width */
static inline unsigned get_width (const zbar_decoder_t *dcode,
                                  unsigned char offset)
{
    return(dcode->w[(dcode->idx - offset) & (DECODE_WINDOW - 1)]);
}

/* retrieve bar+space pair width starting at offset i */
static inline unsigned pair_width (const zbar_decoder_t *dcode,
                                   unsigned char offset)
{
    return(get_width(dcode, offset) + get_width(dcode, offset + 1));
}

/* calculate total character width "s"
 *   - start of character identified by context sensitive offset
 *     (<= DECODE_WINDOW - n)
 *   - size of character is n elements
 */
static inline unsigned calc_s (const zbar_decoder_t *dcode,
                               unsigned char offset,
                               unsigned char n)
{
    /* FIXME check that this gets unrolled for constant n */
    unsigned s = 0;
    while(n--)
        s += get_width(dcode, offset++);
    return(s);
}

/* fixed character width decode assist
 * bar+space width are compared as a fraction of the reference dimension "x"
 *   - +/- 1/2 x tolerance
 *   - measured total character width (s) compared to symbology baseline (n)
 *     (n = 7 for EAN/UPC, 11 for Code 128)
 *   - bar+space *pair width* "e" is used to factor out bad "exposures"
 *     ("blooming" or "swelling" of dark or light areas)
 *     => using like-edge measurements avoids these issues
 *   - n should be > 3
 */
static inline int decode_e (unsigned e,
                            unsigned s,
                            unsigned n)
{
    /* result is encoded number of units - 2
     * (for use as zero based index)
     * or -1 if invalid
     */
    unsigned char E = ((e * n * 2 + 1) / s - 3) / 2;
    return((E >= n - 3) ? -1 : E);
}

/* sort three like-colored elements and return ordering
 */
static inline unsigned decode_sort3 (zbar_decoder_t *dcode,
                                     int i0)
{
    unsigned w0 = get_width(dcode, i0);
    unsigned w2 = get_width(dcode, i0 + 2);
    unsigned w4 = get_width(dcode, i0 + 4);
    if(w0 < w2) {
        if(w2 < w4)
            return((i0 << 8) | ((i0 + 2) << 4) | (i0 + 4));
        if(w0 < w4)
            return((i0 << 8) | ((i0 + 4) << 4) | (i0 + 2));
        return(((i0 + 4) << 8) | (i0 << 4) | (i0 + 2));
    }
    if(w4 < w2)
        return(((i0 + 4) << 8) | ((i0 + 2) << 4) | i0);
    if(w0 < w4)
        return(((i0 + 2) << 8) | (i0 << 4) | (i0 + 4));
    return(((i0 + 2) << 8) | ((i0 + 4) << 4) | i0);
}

/* sort N like-colored elements and return ordering
 */
static inline unsigned decode_sortn (zbar_decoder_t *dcode,
                                     int n,
                                     int i0)
{
    unsigned mask = 0, sort = 0;
    int i;
    for(i = n - 1; i >= 0; i--) {
        unsigned wmin = UINT_MAX;
        int jmin = -1, j;
        for(j = n - 1; j >= 0; j--) {
            if((mask >> j) & 1)
                continue;
            unsigned w = get_width(dcode, i0 + j * 2);
            if(wmin >= w) {
                wmin = w;
                jmin = j;
            }
        }
        zassert(jmin >= 0, 0, "sortn(%d,%d) jmin=%d",
                n, i0, jmin);
        sort <<= 4;
        mask |= 1 << jmin;
        sort |= i0 + jmin * 2;
    }
    return(sort);
}

/* acquire shared state lock */
static inline char acquire_lock (zbar_decoder_t *dcode,
                                 zbar_symbol_type_t req)
{
    if(dcode->lock) {
        dbprintf(2, " [locked %d]\n", dcode->lock);
        return(1);
    }
    dcode->lock = req;
    return(0);
}

/* check and release shared state lock */
static inline char release_lock (zbar_decoder_t *dcode,
                                 zbar_symbol_type_t req)
{
    zassert(dcode->lock == req, 1, "lock=%d req=%d\n",
            dcode->lock, req);
    dcode->lock = 0;
    return(0);
}

/* ensure output buffer has sufficient allocation for request */
static inline char size_buf (zbar_decoder_t *dcode,
                             unsigned len)
{
    unsigned char *buf;
    if(len <= BUFFER_MIN)
        return(0);
    if(len < dcode->buf_alloc)
        /* FIXME size reduction heuristic? */
        return(0);
    if(len > BUFFER_MAX)
        return(1);
    if(len < dcode->buf_alloc + BUFFER_INCR) {
        len = dcode->buf_alloc + BUFFER_INCR;
        if(len > BUFFER_MAX)
            len = BUFFER_MAX;
    }
    buf = realloc(dcode->buf, len);
    if(!buf)
        return(1);
    dcode->buf = buf;
    dcode->buf_alloc = len;
    return(0);
}

extern const char *_zbar_decoder_buf_dump (unsigned char *buf,
                                           unsigned int buflen);

#endif
