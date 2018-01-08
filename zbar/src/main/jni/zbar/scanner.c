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

#include <config.h>
#include <stdlib.h>     /* malloc, free, abs */
#include <stddef.h>
#include <string.h>     /* memset */

#include <zbar.h>
#include "svg.h"

#ifdef DEBUG_SCANNER
# define DEBUG_LEVEL (DEBUG_SCANNER)
#endif
#include "debug.h"

#ifndef ZBAR_FIXED
# define ZBAR_FIXED 5
#endif
#define ROUND (1 << (ZBAR_FIXED - 1))

/* FIXME add runtime config API for these */
#ifndef ZBAR_SCANNER_THRESH_MIN
# define ZBAR_SCANNER_THRESH_MIN  4
#endif

#ifndef ZBAR_SCANNER_THRESH_INIT_WEIGHT
# define ZBAR_SCANNER_THRESH_INIT_WEIGHT .44
#endif
#define THRESH_INIT ((unsigned)((ZBAR_SCANNER_THRESH_INIT_WEIGHT       \
                                 * (1 << (ZBAR_FIXED + 1)) + 1) / 2))

#ifndef ZBAR_SCANNER_THRESH_FADE
# define ZBAR_SCANNER_THRESH_FADE 8
#endif

#ifndef ZBAR_SCANNER_EWMA_WEIGHT
# define ZBAR_SCANNER_EWMA_WEIGHT .78
#endif
#define EWMA_WEIGHT ((unsigned)((ZBAR_SCANNER_EWMA_WEIGHT              \
                                 * (1 << (ZBAR_FIXED + 1)) + 1) / 2))

/* scanner state */
struct zbar_scanner_s {
    zbar_decoder_t *decoder; /* associated bar width decoder */
    unsigned y1_min_thresh; /* minimum threshold */

    unsigned x;             /* relative scan position of next sample */
    int y0[4];              /* short circular buffer of average intensities */

    int y1_sign;            /* slope at last crossing */
    unsigned y1_thresh;     /* current slope threshold */

    unsigned cur_edge;      /* interpolated position of tracking edge */
    unsigned last_edge;     /* interpolated position of last located edge */
    unsigned width;         /* last element width */
};

zbar_scanner_t *zbar_scanner_create (zbar_decoder_t *dcode)
{
    zbar_scanner_t *scn = malloc(sizeof(zbar_scanner_t));
    scn->decoder = dcode;
    scn->y1_min_thresh = ZBAR_SCANNER_THRESH_MIN;
    zbar_scanner_reset(scn);
    return(scn);
}

void zbar_scanner_destroy (zbar_scanner_t *scn)
{
    free(scn);
}

zbar_symbol_type_t zbar_scanner_reset (zbar_scanner_t *scn)
{
    memset(&scn->x, 0, sizeof(zbar_scanner_t) - offsetof(zbar_scanner_t, x));
    scn->y1_thresh = scn->y1_min_thresh;
    if(scn->decoder)
        zbar_decoder_reset(scn->decoder);
    return(ZBAR_NONE);
}

unsigned zbar_scanner_get_width (const zbar_scanner_t *scn)
{
    return(scn->width);
}

unsigned zbar_scanner_get_edge (const zbar_scanner_t *scn,
                                unsigned offset,
                                int prec)
{
    unsigned edge = scn->last_edge - offset - (1 << ZBAR_FIXED) - ROUND;
    prec = ZBAR_FIXED - prec;
    if(prec > 0)
        return(edge >> prec);
    else if(!prec)
        return(edge);
    else
        return(edge << -prec);
}

zbar_color_t zbar_scanner_get_color (const zbar_scanner_t *scn)
{
    return((scn->y1_sign <= 0) ? ZBAR_SPACE : ZBAR_BAR);
}

static inline unsigned calc_thresh (zbar_scanner_t *scn)
{
    /* threshold 1st to improve noise rejection */
    unsigned dx, thresh = scn->y1_thresh;
    unsigned long t;
    if((thresh <= scn->y1_min_thresh) || !scn->width) {
        dbprintf(1, " tmin=%d", scn->y1_min_thresh);
        return(scn->y1_min_thresh);
    }
    /* slowly return threshold to min */
    dx = (scn->x << ZBAR_FIXED) - scn->last_edge;
    t = thresh * dx;
    t /= scn->width;
    t /= ZBAR_SCANNER_THRESH_FADE;
    dbprintf(1, " thr=%d t=%ld x=%d last=%d.%d (%d)",
             thresh, t, scn->x, scn->last_edge >> ZBAR_FIXED,
             scn->last_edge & ((1 << ZBAR_FIXED) - 1), dx);
    if(thresh > t) {
        thresh -= t;
        if(thresh > scn->y1_min_thresh)
            return(thresh);
    }
    scn->y1_thresh = scn->y1_min_thresh;
    return(scn->y1_min_thresh);
}

static inline zbar_symbol_type_t process_edge (zbar_scanner_t *scn,
                                               int y1)
{
    if(!scn->y1_sign)
        scn->last_edge = scn->cur_edge = (1 << ZBAR_FIXED) + ROUND;
    else if(!scn->last_edge)
        scn->last_edge = scn->cur_edge;

    scn->width = scn->cur_edge - scn->last_edge;
    dbprintf(1, " sgn=%d cur=%d.%d w=%d (%s)\n",
             scn->y1_sign, scn->cur_edge >> ZBAR_FIXED,
             scn->cur_edge & ((1 << ZBAR_FIXED) - 1), scn->width,
             ((y1 > 0) ? "SPACE" : "BAR"));
    scn->last_edge = scn->cur_edge;

#if DEBUG_SVG > 1
    svg_path_moveto(SVG_ABS, scn->last_edge - (1 << ZBAR_FIXED) - ROUND, 0);
#endif

    /* pass to decoder */
    if(scn->decoder)
        return(zbar_decode_width(scn->decoder, scn->width));
    return(ZBAR_PARTIAL);
}

inline zbar_symbol_type_t zbar_scanner_flush (zbar_scanner_t *scn)
{
    unsigned x;
    if(!scn->y1_sign)
        return(ZBAR_NONE);

    x = (scn->x << ZBAR_FIXED) + ROUND;

    if(scn->cur_edge != x || scn->y1_sign > 0) {
        zbar_symbol_type_t edge = process_edge(scn, -scn->y1_sign);
        dbprintf(1, "flush0:");
        scn->cur_edge = x;
        scn->y1_sign = -scn->y1_sign;
        return(edge);
    }

    scn->y1_sign = scn->width = 0;
    if(scn->decoder)
        return(zbar_decode_width(scn->decoder, 0));
    return(ZBAR_PARTIAL);
}

zbar_symbol_type_t zbar_scanner_new_scan (zbar_scanner_t *scn)
{
    zbar_symbol_type_t edge = ZBAR_NONE;
    while(scn->y1_sign) {
        zbar_symbol_type_t tmp = zbar_scanner_flush(scn);
        if(tmp < 0 || tmp > edge)
            edge = tmp;
    }

    /* reset scanner and associated decoder */
    memset(&scn->x, 0, sizeof(zbar_scanner_t) - offsetof(zbar_scanner_t, x));
    scn->y1_thresh = scn->y1_min_thresh;
    if(scn->decoder)
        zbar_decoder_new_scan(scn->decoder);
    return(edge);
}

zbar_symbol_type_t zbar_scan_y (zbar_scanner_t *scn,
                                int y)
{
    /* FIXME calc and clip to max y range... */
    /* retrieve short value history */
    register int x = scn->x;
    register int y0_1 = scn->y0[(x - 1) & 3];
    register int y0_0 = y0_1;
    register int y0_2, y0_3, y1_1, y2_1, y2_2;
    zbar_symbol_type_t edge;
    if(x) {
        /* update weighted moving average */
        y0_0 += ((int)((y - y0_1) * EWMA_WEIGHT)) >> ZBAR_FIXED;
        scn->y0[x & 3] = y0_0;
    }
    else
        y0_0 = y0_1 = scn->y0[0] = scn->y0[1] = scn->y0[2] = scn->y0[3] = y;
    y0_2 = scn->y0[(x - 2) & 3];
    y0_3 = scn->y0[(x - 3) & 3];
    /* 1st differential @ x-1 */
    y1_1 = y0_1 - y0_2;
    {
        register int y1_2 = y0_2 - y0_3;
        if((abs(y1_1) < abs(y1_2)) &&
           ((y1_1 >= 0) == (y1_2 >= 0)))
            y1_1 = y1_2;
    }

    /* 2nd differentials @ x-1 & x-2 */
    y2_1 = y0_0 - (y0_1 * 2) + y0_2;
    y2_2 = y0_1 - (y0_2 * 2) + y0_3;

    dbprintf(1, "scan: x=%d y=%d y0=%d y1=%d y2=%d",
             x, y, y0_1, y1_1, y2_1);

    edge = ZBAR_NONE;
    /* 2nd zero-crossing is 1st local min/max - could be edge */
    if((!y2_1 ||
        ((y2_1 > 0) ? y2_2 < 0 : y2_2 > 0)) &&
       (calc_thresh(scn) <= abs(y1_1)))
    {
        /* check for 1st sign change */
        char y1_rev = (scn->y1_sign > 0) ? y1_1 < 0 : y1_1 > 0;
        if(y1_rev)
            /* intensity change reversal - finalize previous edge */
            edge = process_edge(scn, y1_1);

        if(y1_rev || (abs(scn->y1_sign) < abs(y1_1))) {
            int d;
            scn->y1_sign = y1_1;

            /* adaptive thresholding */
            /* start at multiple of new min/max */
            scn->y1_thresh = (abs(y1_1) * THRESH_INIT + ROUND) >> ZBAR_FIXED;
            dbprintf(1, "\tthr=%d", scn->y1_thresh);
            if(scn->y1_thresh < scn->y1_min_thresh)
                scn->y1_thresh = scn->y1_min_thresh;

            /* update current edge */
            d = y2_1 - y2_2;
            scn->cur_edge = 1 << ZBAR_FIXED;
            if(!d)
                scn->cur_edge >>= 1;
            else if(y2_1)
                /* interpolate zero crossing */
                scn->cur_edge -= ((y2_1 << ZBAR_FIXED) + 1) / d;
            scn->cur_edge += x << ZBAR_FIXED;
            dbprintf(1, "\n");
        }
    }
    else
        dbprintf(1, "\n");
    /* FIXME add fall-thru pass to decoder after heuristic "idle" period
       (eg, 6-8 * last width) */
    scn->x = x + 1;
    return(edge);
}

/* undocumented API for drawing cutesy debug graphics */
void zbar_scanner_get_state (const zbar_scanner_t *scn,
                             unsigned *x,
                             unsigned *cur_edge,
                             unsigned *last_edge,
                             int *y0,
                             int *y1,
                             int *y2,
                             int *y1_thresh)
{
    register int y0_0 = scn->y0[(scn->x - 1) & 3];
    register int y0_1 = scn->y0[(scn->x - 2) & 3];
    register int y0_2 = scn->y0[(scn->x - 3) & 3];
    zbar_scanner_t *mut_scn;
    if(x) *x = scn->x - 1;
    if(last_edge) *last_edge = scn->last_edge;
    if(y0) *y0 = y0_1;
    if(y1) *y1 = y0_1 - y0_2;
    if(y2) *y2 = y0_0 - (y0_1 * 2) + y0_2;
    /* NB not quite accurate (uses updated x) */
    mut_scn = (zbar_scanner_t*)scn;
    if(y1_thresh) *y1_thresh = calc_thresh(mut_scn);
    dbprintf(1, "\n");
}
