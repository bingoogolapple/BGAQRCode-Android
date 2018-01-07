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

#include <config.h>
#include <zbar.h>

#ifdef DEBUG_DATABAR
# define DEBUG_LEVEL (DEBUG_DATABAR)
#endif
#include "debug.h"
#include "decoder.h"

#define GS ('\035')

enum { SCH_NUM, SCH_ALNUM, SCH_ISO646 };

static const signed char finder_hash[0x20] = {
    0x16, 0x1f, 0x02, 0x00, 0x03, 0x00, 0x06, 0x0b,
    0x1f, 0x0e, 0x17, 0x0c, 0x0b, 0x14, 0x11, 0x0c,
    0x1f, 0x03, 0x13, 0x08, 0x00, 0x0a,   -1, 0x16,
    0x0c, 0x09,   -1, 0x1a, 0x1f, 0x1c, 0x00,   -1,
};

/* DataBar character encoding groups */
struct group_s {
    unsigned short sum;
    unsigned char wmax;
    unsigned char todd;
    unsigned char teven;
} groups[] = {
    /* (17,4) DataBar Expanded character groups */
    {    0, 7,  87,   4 },
    {  348, 5,  52,  20 },
    { 1388, 4,  30,  52 },
    { 2948, 3,  10, 104 },
    { 3988, 1,   1, 204 },

    /* (16,4) DataBar outer character groups */
    {    0, 8, 161,   1 },
    {  161, 6,  80,  10 },
    {  961, 4,  31,  34 },
    { 2015, 3,  10,  70 },
    { 2715, 1,   1, 126 },

    /* (15,4) DataBar inner character groups */
    { 1516, 8,  81,   1 },
    { 1036, 6,  48,  10 },
    {  336, 4,  20,  35 },
    {    0, 2,   4,  84 },
};

static const unsigned char exp_sequences[] = {
    /* sequence Group 1 */
    0x01,
    0x23,
    0x25, 0x07,
    0x29, 0x47,
    0x29, 0x67, 0x0b,
    0x29, 0x87, 0xab,
    /* sequence Group 2 */
    0x21, 0x43, 0x65, 0x07,
    0x21, 0x43, 0x65, 0x89,
    0x21, 0x43, 0x65, 0xa9, 0x0b,
    0x21, 0x43, 0x67, 0x89, 0xab
};

/* DataBar expanded checksum multipliers */
static const unsigned char exp_checksums[] = {
    1, 189, 62, 113, 46, 43, 109, 134, 6, 79, 161, 45
};

static inline void
append_check14 (unsigned char *buf)
{
    unsigned char chk = 0, d;
    int i;
    for(i = 13; --i >= 0; ) {
        d = *(buf++) - '0';
        chk += d;
        if(!(i & 1))
            chk += d << 1;
    }
    chk %= 10;
    if(chk)
        chk = 10 - chk;
    *buf = chk + '0';
}

static inline void
decode10 (unsigned char *buf,
          unsigned long n,
          int i)
{
    buf += i;
    while(--i >= 0) {
        unsigned char d = n % 10;
        n /= 10;
        *--buf = '0' + d;
    }
}

#define VAR_MAX(l, i) ((((l) * 12 + (i)) * 2 + 6) / 7)

#define FEED_BITS(b)                         \
    while(i < (b) && len) {                  \
        d = (d << 12) | (*(data++) & 0xfff); \
        i += 12;                             \
        len--;                               \
        dbprintf(2, " %03lx", d & 0xfff);     \
    }

#define PUSH_CHAR(c) \
    *(buf++) = (c)

#define PUSH_CHAR4(c0, c1, c2, c3) do { \
        PUSH_CHAR(c0);                  \
        PUSH_CHAR(c1);                  \
        PUSH_CHAR(c2);                  \
        PUSH_CHAR(c3);                  \
    } while(0);

static inline int
databar_postprocess_exp (zbar_decoder_t *dcode,
                         int *data)
{
    int i = 0, enc;
    unsigned n;
    unsigned char *buf;
    unsigned long d = *(data++);
    int len = d / 211 + 4, buflen;

    /* grok encodation method */
    d = *(data++);
    dbprintf(2, "\n    len=%d %03lx", len, d & 0xfff);
    n = (d >> 4) & 0x7f;
    if(n >= 0x40) {
        i = 10;
        enc = 1;
        buflen = 2 + 14 + VAR_MAX(len, 10 - 2 - 44 + 6) + 2;
    }
    else if(n >= 0x38) {
        i = 4;
        enc = 6 + (n & 7);
        buflen = 2 + 14 + 4 + 6 + 2 + 6 + 2;
    }
    else if(n >= 0x30) {
        i = 6;
        enc = 2 + ((n >> 2) & 1);
        buflen = 2 + 14 + 4 + 3 + VAR_MAX(len, 6 - 2 - 44 - 2 - 10) + 2;
    }
    else if(n >= 0x20) {
        i = 7;
        enc = 4 + ((n >> 3) & 1);
        buflen = 2 + 14 + 4 + 6;
    }
    else {
        i = 9;
        enc = 0;
        buflen = VAR_MAX(len, 9 - 2) + 2;
    }
    dbprintf(2, " buflen=%d enc=%d", buflen, enc);
    zassert(buflen > 2, -1, "buflen=%d\n", buflen);

    if(enc < 4) {
        /* grok variable length symbol bit field */
        if((len ^ (d >> (--i))) & 1)
            /* even/odd length mismatch */
            return(-1);
        if(((d >> (--i)) & 1) != (len > 14))
            /* size group mismatch */
            return(-1);
    }
    len -= 2;
    dbprintf(2, " [%d+%d]", i, len);

    if(size_buf(dcode, buflen))
        return(-1);
    buf = dcode->buf;

    /* handle compressed fields */
    if(enc) {
        PUSH_CHAR('0');
        PUSH_CHAR('1');
    }

    if(enc == 1) {
        i -= 4;
        n = (d >> i) & 0xf;
        if(i >= 10)
            return(-1);
        PUSH_CHAR('0' + n);
    }
    else if(enc)
        PUSH_CHAR('9');

    if(enc) {
        int j;
        for(j = 0; j < 4; j++) {
            FEED_BITS(10);
            i -= 10;
            n = (d >> i) & 0x3ff;
            if(n >= 1000)
                return(-1);
            decode10(buf, n, 3);
            buf += 3;
        }
        append_check14(buf - 13);
        buf++;
    }

    switch(enc)
    {
    case 2: /* 01100: AI 392x */
        FEED_BITS(2);
        i -= 2;
        n = (d >> i) & 0x3;
        PUSH_CHAR4('3', '9', '2', '0' + n);
        break;

    case 3: /* 01101: AI 393x */
        FEED_BITS(12);
        i -= 2;
        n = (d >> i) & 0x3;
        PUSH_CHAR4('3', '9', '3', '0' + n);
        i -= 10;
        n = (d >> i) & 0x3ff;
        if(n >= 1000)
            return(-1);
        decode10(buf, n, 3);
        buf += 3;
        break;

    case 4: /* 0100: AI 3103 */
        FEED_BITS(15);
        i -= 15;
        n = (d >> i) & 0x7fff;
        PUSH_CHAR4('3', '1', '0', '3');
        decode10(buf, n, 6);
        buf += 6;
        break;

    case 5: /* 0101: AI 3202/3203 */
        FEED_BITS(15);
        i -= 15;
        n = (d >> i) & 0x7fff;
        dbprintf(2, " v=%d", n);
        PUSH_CHAR4('3', '2', '0', (n >= 10000) ? '3' : '2' );
        if(n >= 10000)
            n -= 10000;
        decode10(buf, n, 6);
        buf += 6;
        break;
    }
    if(enc >= 6) {
        /* 0111000 - 0111111: AI 310x/320x + AI 11/13/15/17 */
        PUSH_CHAR4('3', '1' + (enc & 1), '0', 'x');
        FEED_BITS(20);
        i -= 20;
        n = (d >> i) & 0xfffff;
        dbprintf(2, " [%d+%d] %d", i, len, n);
        if(n >= 1000000)
            return(-1);
        decode10(buf, n, 6);
        *(buf - 1) = *buf;
        *buf = '0';
        buf += 6;

        FEED_BITS(16);
        i -= 16;
        n = (d >> i) & 0xffff;
        if(n < 38400) {
            int dd, mm, yy;
            dd = n % 32;
            n /= 32;
            mm = n % 12 + 1;
            n /= 12;
            yy = n;
            PUSH_CHAR('1');
            PUSH_CHAR('0' + ((enc - 6) | 1));
            decode10(buf, yy, 2);
            buf += 2;
            decode10(buf, mm, 2);
            buf += 2;
            decode10(buf, dd, 2);
            buf += 2;
        }
        else if(n > 38400)
            return(-1);
    }

    if(enc < 4) {
        /* remainder is general-purpose data compaction */
        int scheme = SCH_NUM;
        while(i > 0 || len > 0) {
            FEED_BITS(8);
            dbprintf(2, " [%d+%d]", i, len);

            if(scheme == SCH_NUM) {
                int n1;
                i -= 4;
                if(i < 0)
                    break;
                if(!((d >> i) & 0xf)) {
                    scheme = SCH_ALNUM;
                    dbprintf(2, ">A");
                    continue;
                }
                if(!len && i < 3) {
                    /* special case last digit */
                    n = ((d >> i) & 0xf) - 1;
                    if(n > 9)
                        return(-1);
                    *(buf++) = '0' + n;
                    break;
                }
                i -= 3;
                zassert(i >= 0, -1, "\n");
                n = ((d >> i) & 0x7f) - 8;
                n1 = n % 11;
                n = n / 11;
                dbprintf(2, "N%d%d", n, n1);
                *(buf++) = (n < 10) ? '0' + n : GS;
                *(buf++) = (n1 < 10) ? '0' + n1 : GS;
            }
            else  {
                unsigned c = 0;
                i -= 3;
                if(i < 0)
                    break;
                if(!((d >> i) & 0x7)) {
                    scheme = SCH_NUM;
                    continue;
                }
                i -= 2;
                if(i < 0)
                    break;
                n = (d >> i) & 0x1f;
                if(n == 0x04) {
                    scheme ^= 0x3;
                    dbprintf(2, ">%d", scheme);
                }
                else if(n == 0x0f)
                    c = GS;
                else if(n < 0x0f)
                    c = 43 + n;
                else if(scheme == SCH_ALNUM) {
                    i--;
                    if(i < 0)
                        return(-1);
                    n = (d >> i) & 0x1f;
                    if(n < 0x1a)
                        c = 'A' + n;
                    else if(n == 0x1a)
                        c = '*';
                    else if(n < 0x1f)
                        c = ',' + n - 0x1b;
                    else
                        return(-1);
                }
                else if(scheme == SCH_ISO646 && n < 0x1d) {
                    i -= 2;
                    if(i < 0)
                        return(-1);
                    n = (d >> i) & 0x3f;
                    if(n < 0x1a)
                        c = 'A' + n;
                    else if(n < 0x34)
                        c = 'a' + n - 0x1a;
                    else
                        return(-1);
                }
                else if(scheme == SCH_ISO646) {
                    i -= 3;
                    if(i < 0)
                        return(-1);
                    n = ((d >> i) & 0x1f);
                    dbprintf(2, "(%02x)", n);
                    if(n < 0xa)
                        c = '!' + n - 8;
                    else if(n < 0x15)
                        c = '%' + n - 0xa;
                    else if(n < 0x1b)
                        c = ':' + n - 0x15;
                    else if(n == 0x1b)
                        c = '_';
                    else if(n == 0x1c)
                        c = ' ';
                    else
                        return(-1);
                }
                else
                    return(-1);

                if(c) {
                    dbprintf(2, "%d%c", scheme, c);
                    *(buf++) = c;
                }
            }
        }
        /* FIXME check pad? */
    }

    i = buf - dcode->buf;
    zassert(i < dcode->buf_alloc, -1, "i=%02x %s\n", i,
            _zbar_decoder_buf_dump(dcode->buf, i));

    *buf = 0;
    dcode->buflen = i;
    if(i && *--buf == GS) {
        *buf = 0;
        dcode->buflen--;
    }

    dbprintf(2, "\n    %s", _zbar_decoder_buf_dump(dcode->buf, dcode->buflen));
    return(0);
}
#undef FEED_BITS

/* convert from heterogeneous base {1597,2841}
 * to base 10 character representation
 */
static inline void
databar_postprocess (zbar_decoder_t *dcode,
                     unsigned d[4])
{
    databar_decoder_t *db = &dcode->databar;
    int i;
    unsigned c, chk = 0;
    unsigned char *buf = dcode->buf;
    *(buf++) = '0';
    *(buf++) = '1';
    buf += 15;
    *--buf = '\0';
    *--buf = '\0';

    dbprintf(2, "\n    d={%d,%d,%d,%d}", d[0], d[1], d[2], d[3]);
    unsigned long r = d[0] * 1597 + d[1];
    d[1] = r / 10000;
    r %= 10000;
    r = r * 2841 + d[2];
    d[2] = r / 10000;
    r %= 10000;
    r = r * 1597 + d[3];
    d[3] = r / 10000;
    dbprintf(2, " r=%ld", r);

    for(i = 4; --i >= 0; ) {
        c = r % 10;
        chk += c;
        if(i & 1)
            chk += c << 1;
        *--buf = c + '0';
        if(i)
            r /= 10;
    }

    dbprintf(2, " d={%d,%d,%d}", d[1], d[2], d[3]);
    r = d[1] * 2841 + d[2];
    d[2] = r / 10000;
    r %= 10000;
    r = r * 1597 + d[3];
    d[3] = r / 10000;
    dbprintf(2, " r=%ld", r);

    for(i = 4; --i >= 0; ) {
        c = r % 10;
        chk += c;
        if(i & 1)
            chk += c << 1;
        *--buf = c + '0';
        if(i)
            r /= 10;
    }

    r = d[2] * 1597 + d[3];
    dbprintf(2, " d={%d,%d} r=%ld", d[2], d[3], r);

    for(i = 5; --i >= 0; ) {
        c = r % 10;
        chk += c;
        if(!(i & 1))
            chk += c << 1;
        *--buf = c + '0';
        if(i)
            r /= 10;
    }

    /* NB linkage flag not supported */
    if(TEST_CFG(db->config, ZBAR_CFG_EMIT_CHECK)) {
        chk %= 10;
        if(chk)
            chk = 10 - chk;
        buf[13] = chk + '0';
        dcode->buflen = buf - dcode->buf + 14;
    }
    else
        dcode->buflen = buf - dcode->buf + 13;

    dbprintf(2, "\n    %s", _zbar_decoder_buf_dump(dcode->buf, 16));
}

static inline int
check_width (unsigned wf,
             unsigned wd,
             unsigned n)
{
    unsigned dwf = wf * 3;
    wd *= 14;
    wf *= n;
    return(wf - dwf <= wd && wd <= wf + dwf);
}

static inline void
merge_segment (databar_decoder_t *db,
               databar_segment_t *seg)
{
    unsigned csegs = db->csegs;
    int i;
    for(i = 0; i < csegs; i++) {
        databar_segment_t *s = db->segs + i;
        if(s != seg && s->finder == seg->finder && s->exp == seg->exp &&
           s->color == seg->color && s->side == seg->side &&
           s->data == seg->data && s->check == seg->check &&
           check_width(seg->width, s->width, 14)) {
            /* merge with existing segment */
            unsigned cnt = s->count;
            if(cnt < 0x7f)
                cnt++;
            seg->count = cnt;
            seg->partial &= s->partial;
            seg->width = (3 * seg->width + s->width + 2) / 4;
            s->finder = -1;
            dbprintf(2, " dup@%d(%d,%d)",
                     i, cnt, (db->epoch - seg->epoch) & 0xff);
        }
        else if(s->finder >= 0) {
            unsigned age = (db->epoch - s->epoch) & 0xff;
            if(age >= 248 || (age >= 128 && s->count < 2))
                s->finder = -1;
        }
    }
}

static inline zbar_symbol_type_t
match_segment (zbar_decoder_t *dcode,
               databar_segment_t *seg)
{
    databar_decoder_t *db = &dcode->databar;
    unsigned csegs = db->csegs, maxage = 0xfff;
    int i0, i1, i2, maxcnt = 0;
    databar_segment_t *smax[3] = { NULL, };

    if(seg->partial && seg->count < 4)
        return(ZBAR_PARTIAL);

    for(i0 = 0; i0 < csegs; i0++) {
        databar_segment_t *s0 = db->segs + i0;
        if(s0 == seg || s0->finder != seg->finder || s0->exp ||
           s0->color != seg->color || s0->side == seg->side ||
           (s0->partial && s0->count < 4) ||
           !check_width(seg->width, s0->width, 14))
            continue;

        for(i1 = 0; i1 < csegs; i1++) {
            databar_segment_t *s1 = db->segs + i1;
            int chkf, chks, chk;
            unsigned age1;
            if(i1 == i0 || s1->finder < 0 || s1->exp ||
               s1->color == seg->color ||
               (s1->partial && s1->count < 4) ||
               !check_width(seg->width, s1->width, 14))
                continue;
            dbprintf(2, "\n\t[%d,%d] f=%d(0%xx)/%d(%x%x%x)",
                     i0, i1, seg->finder, seg->color,
                     s1->finder, s1->exp, s1->color, s1->side);

            if(seg->color)
                chkf = seg->finder + s1->finder * 9;
            else
                chkf = s1->finder + seg->finder * 9;
            if(chkf > 72)
                chkf--;
            if(chkf > 8)
                chkf--;

            chks = (seg->check + s0->check + s1->check) % 79;

            if(chkf >= chks)
                chk = chkf - chks;
            else
                chk = 79 + chkf - chks;

            dbprintf(2, " chk=(%d,%d) => %d", chkf, chks, chk);
            age1 = (((db->epoch - s0->epoch) & 0xff) +
                    ((db->epoch - s1->epoch) & 0xff));

            for(i2 = i1 + 1; i2 < csegs; i2++) {
                databar_segment_t *s2 = db->segs + i2;
                unsigned cnt, age2, age;
                if(i2 == i0 || s2->finder != s1->finder || s2->exp ||
                   s2->color != s1->color || s2->side == s1->side ||
                   s2->check != chk ||
                   (s2->partial && s2->count < 4) ||
                   !check_width(seg->width, s2->width, 14))
                    continue;
                age2 = (db->epoch - s2->epoch) & 0xff;
                age = age1 + age2;
                cnt = s0->count + s1->count + s2->count;
                dbprintf(2, " [%d] MATCH cnt=%d age=%d", i2, cnt, age);
                if(maxcnt < cnt ||
                   (maxcnt == cnt && maxage > age)) {
                    maxcnt = cnt;
                    maxage = age;
                    smax[0] = s0;
                    smax[1] = s1;
                    smax[2] = s2;
                }
            }
        }
    }

    if(!smax[0])
        return(ZBAR_PARTIAL);

    unsigned d[4];
    d[(seg->color << 1) | seg->side] = seg->data;
    for(i0 = 0; i0 < 3; i0++) {
        d[(smax[i0]->color << 1) | smax[i0]->side] = smax[i0]->data;
        if(!--(smax[i0]->count))
            smax[i0]->finder = -1;
    }
    seg->finder = -1;

    if(size_buf(dcode, 18))
        return(ZBAR_PARTIAL);

    if(acquire_lock(dcode, ZBAR_DATABAR))
        return(ZBAR_PARTIAL);

    databar_postprocess(dcode, d);
    dcode->modifiers = MOD(ZBAR_MOD_GS1);
    dcode->direction = 1 - 2 * (seg->side ^ seg->color ^ 1);
    return(ZBAR_DATABAR);
}

static inline unsigned
lookup_sequence (databar_segment_t *seg,
                 int fixed,
                 int seq[22])
{
    unsigned n = seg->data / 211, i;
    const unsigned char *p;
    i = (n + 1) / 2 + 1;
    n += 4;
    i = (i * i) / 4;
    dbprintf(2, " {%d,%d:", i, n);
    p = exp_sequences + i;

    fixed >>= 1;
    seq[0] = 0;
    seq[1] = 1;
    for(i = 2; i < n; ) {
        int s = *p;
        if(!(i & 2)) {
            p++;
            s >>= 4;
        }
        else
            s &= 0xf;
        if(s == fixed)
            fixed = -1;
        s <<= 1;
        dbprintf(2, "%x", s);
        seq[i++] = s++;
        seq[i++] = s;
    }
    dbprintf(2, "}");
    seq[n] = -1;
    return(fixed < 1);
}

#define IDX(s) \
    (((s)->finder << 2) | ((s)->color << 1) | ((s)->color ^ (s)->side))

static inline zbar_symbol_type_t
match_segment_exp (zbar_decoder_t *dcode,
                   databar_segment_t *seg,
                   int dir)
{
    databar_decoder_t *db = &dcode->databar;
    int bestsegs[22], i = 0, segs[22], seq[22];
    int ifixed = seg - db->segs, fixed = IDX(seg), maxcnt = 0;
    int iseg[DATABAR_MAX_SEGMENTS];
    unsigned csegs = db->csegs, width = seg->width, maxage = 0x7fff;

    bestsegs[0] = segs[0] = seq[1] = -1;
    seq[0] = 0;

    dbprintf(2, "\n    fixed=%d@%d: ", fixed, ifixed);
    for(i = csegs, seg = db->segs + csegs - 1; --i >= 0; seg--) {
        if(seg->exp && seg->finder >= 0 &&
           (!seg->partial || seg->count >= 4))
            iseg[i] = IDX(seg);
        else
            iseg[i] = -1;
        dbprintf(2, " %d", iseg[i]);
    }

    for(i = 0; ; i--) {
        if(!i)
            dbprintf(2, "\n   ");
        for(; i >= 0 && seq[i] >= 0; i--) {
            int j;
            dbprintf(2, " [%d]%d", i, seq[i]);

            if(seq[i] == fixed) {
                seg = db->segs + ifixed;
                if(segs[i] < 0 && check_width(width, seg->width, 14)) {
                    dbprintf(2, "*");
                    j = ifixed;
                }
                else
                    continue;
            }
            else {
                for(j = segs[i] + 1; j < csegs; j++) {
                    if(iseg[j] == seq[i] &&
                       (!i || check_width(width, db->segs[j].width, 14))) {
                        seg = db->segs + j;
                        break;
                    }
                }
                if(j == csegs)
                    continue;
            }

            if(!i) {
                if(!lookup_sequence(seg, fixed, seq)) {
                    dbprintf(2, "[nf]");
                    continue;
                }
                width = seg->width;
                dbprintf(2, " A00@%d", j);
            }
            else {
                width = (width + seg->width) / 2;
                dbprintf(2, " %c%x%x@%d",
                         'A' + seg->finder, seg->color, seg->side, j);
            }
            segs[i++] = j;
            segs[i++] = -1;
        }
        if(i < 0)
            break;

        seg = db->segs + segs[0];
        unsigned cnt = 0, chk = 0, age = (db->epoch - seg->epoch) & 0xff;
        for(i = 1; segs[i] >= 0; i++) {
            seg = db->segs + segs[i];
            chk += seg->check;
            cnt += seg->count;
            age += (db->epoch - seg->epoch) & 0xff;
        }

        unsigned data0 = db->segs[segs[0]].data;
        unsigned chk0 = data0 % 211;
        chk %= 211;

        dbprintf(2, " chk=%d ?= %d", chk, chk0);
        if(chk != chk0)
            continue;

        dbprintf(2, " cnt=%d age=%d", cnt, age);
        if(maxcnt > cnt || (maxcnt == cnt && maxage <= age))
            continue;

        dbprintf(2, " !");
        maxcnt = cnt;
        maxage = age;
        for(i = 0; segs[i] >= 0; i++)
            bestsegs[i] = segs[i];
        bestsegs[i] = -1;
    }

    if(bestsegs[0] < 0)
        return(ZBAR_PARTIAL);

    if(acquire_lock(dcode, ZBAR_DATABAR_EXP))
        return(ZBAR_PARTIAL);

    for(i = 0; bestsegs[i] >= 0; i++)
        segs[i] = db->segs[bestsegs[i]].data;

    if(databar_postprocess_exp(dcode, segs)) {
        release_lock(dcode, ZBAR_DATABAR_EXP);
        return(ZBAR_PARTIAL);
    }

    for(i = 0; bestsegs[i] >= 0; i++)
        if(bestsegs[i] != ifixed) {
            seg = db->segs + bestsegs[i];
            if(!--seg->count)
                seg->finder = -1;
        }

    /* FIXME stacked rows are frequently reversed,
     * so direction is impossible to determine at this level
     */
    dcode->direction = (1 - 2 * (seg->side ^ seg->color)) * dir;
    dcode->modifiers = MOD(ZBAR_MOD_GS1);
    return(ZBAR_DATABAR_EXP);
}
#undef IDX

static inline unsigned
calc_check (unsigned sig0,
            unsigned sig1,
            unsigned side,
            unsigned mod)
{
    unsigned chk = 0;
    int i;
    for(i = 4; --i >= 0; ) {
        chk = (chk * 3 + (sig1 & 0xf) + 1) * 3 + (sig0 & 0xf) + 1;
        sig1 >>= 4;
        sig0 >>= 4;
        if(!(i & 1))
            chk %= mod;
    }
    dbprintf(2, " chk=%d", chk);

    if(side)
        chk = (chk * (6561 % mod)) % mod;
    return(chk);
}

static inline int
calc_value4 (unsigned sig,
             unsigned n,
             unsigned wmax,
             unsigned nonarrow)
{
    unsigned v = 0;
    n--;

    unsigned w0 = (sig >> 12) & 0xf;
    if(w0 > 1) {
        if(w0 > wmax)
            return(-1);
        unsigned n0 = n - w0;
        unsigned sk20 = (n - 1) * n * (2 * n - 1);
        unsigned sk21 = n0 * (n0 + 1) * (2 * n0 + 1);
        v = sk20 - sk21 - 3 * (w0 - 1) * (2 * n - w0);

        if(!nonarrow && w0 > 2 && n > 4) {
            unsigned k = (n - 2) * (n - 1) * (2 * n - 3) - sk21;
            k -= 3 * (w0 - 2) * (14 * n - 7 * w0 - 31);
            v -= k;
        }

        if(n - 2 > wmax) {
            unsigned wm20 = 2 * wmax * (wmax + 1);
            unsigned wm21 = (2 * wmax + 1);
            unsigned k = sk20;
            if(n0 > wmax) {
                k -= sk21;
                k += 3 * (w0 - 1) * (wm20 - wm21 * (2 * n - w0));
            }
            else {
                k -= (wmax + 1) * (wmax + 2) * (2 * wmax + 3);
                k += 3 * (n - wmax - 2) * (wm20 - wm21 * (n + wmax + 1));
            }
            k *= 3;
            v -= k;
        }
        v /= 12;
    }
    else
        nonarrow = 1;
    n -= w0;

    unsigned w1 = (sig >> 8) & 0xf;
    if(w1 > 1) {
        if(w1 > wmax)
            return(-1);
        v += (2 * n - w1) * (w1 - 1) / 2;
        if(!nonarrow && w1 > 2 && n > 3)
            v -= (2 * n - w1 - 5) * (w1 - 2) / 2;
        if(n - 1 > wmax) {
            if(n - w1 > wmax)
                v -= (w1 - 1) * (2 * n - w1 - 2 * wmax);
            else
                v -= (n - wmax) * (n - wmax - 1);
        }
    }
    else
        nonarrow = 1;
    n -= w1;

    unsigned w2 = (sig >> 4) & 0xf;
    if(w2 > 1) {
        if(w2 > wmax)
            return(-1);
        v += w2 - 1;
        if(!nonarrow && w2 > 2 && n > 2)
            v -= n - 2;
        if(n > wmax)
            v -= n - wmax;
    }
    else
        nonarrow = 1;

    unsigned w3 = sig & 0xf;
    if(w3 == 1)
        nonarrow = 1;
    else if(w3 > wmax)
        return(-1);

    if(!nonarrow)
        return(-1);

    return(v);
}

static inline zbar_symbol_type_t
decode_char (zbar_decoder_t *dcode,
             databar_segment_t *seg,
             int off,
             int dir)
{
    databar_decoder_t *db = &dcode->databar;
    unsigned s = calc_s(dcode, (dir > 0) ? off : off - 6, 8);
    int n, i, emin[2] = { 0, }, sum = 0;
    unsigned sig0 = 0, sig1 = 0;

    if(seg->exp)
        n = 17;
    else if(seg->side)
        n = 15;
    else
        n = 16;
    emin[1] = -n;

    dbprintf(2, "\n        char[%c%d]: n=%d s=%d w=%d sig=",
             (dir < 0) ? '>' : '<', off, n, s, seg->width);
    if(s < 13 || !check_width(seg->width, s, n))
        return(ZBAR_NONE);

    for(i = 4; --i >= 0; ) {
        int e = decode_e(pair_width(dcode, off), s, n);
        if(e < 0)
            return(ZBAR_NONE);
        dbprintf(2, "%d", e);
        sum = e - sum;
        off += dir;
        sig1 <<= 4;
        if(emin[1] < -sum)
            emin[1] = -sum;
        sig1 += sum;
        if(!i)
            break;

        e = decode_e(pair_width(dcode, off), s, n);
        if(e < 0)
            return(ZBAR_NONE);
        dbprintf(2, "%d", e);
        sum = e - sum;
        off += dir;
        sig0 <<= 4;
        if(emin[0] > sum)
            emin[0] = sum;
        sig0 += sum;
    }

    int diff = emin[~n & 1];
    diff = diff + (diff << 4);
    diff = diff + (diff << 8);

    sig0 -= diff;
    sig1 += diff;

    dbprintf(2, " emin=%d,%d el=%04x/%04x", emin[0], emin[1], sig0, sig1);

    unsigned sum0 = sig0 + (sig0 >> 8);
    unsigned sum1 = sig1 + (sig1 >> 8);
    sum0 += sum0 >> 4;
    sum1 += sum1 >> 4;
    sum0 &= 0xf;
    sum1 &= 0xf;

    dbprintf(2, " sum=%d/%d", sum0, sum1);

    if(sum0 + sum1 + 8 != n) {
        dbprintf(2, " [SUM]");
        return(ZBAR_NONE);
    }

    if(((sum0 ^ (n >> 1)) | (sum1 ^ (n >> 1) ^ n)) & 1) {
        dbprintf(2, " [ODD]");
        return(ZBAR_NONE);
    }

    i = ((n & 0x3) ^ 1) * 5 + (sum1 >> 1);
    zassert(i < sizeof(groups) / sizeof(*groups), -1,
            "n=%d sum=%d/%d sig=%04x/%04x g=%d",
            n, sum0, sum1, sig0, sig1, i);
    struct group_s *g = groups + i;
    dbprintf(2, "\n            g=%d(%d,%d,%d/%d)",
             i, g->sum, g->wmax, g->todd, g->teven);

    int vodd = calc_value4(sig0 + 0x1111, sum0 + 4, g->wmax, ~n & 1);
    dbprintf(2, " v=%d", vodd);
    if(vodd < 0 || vodd > g->todd)
        return(ZBAR_NONE);

    int veven = calc_value4(sig1 + 0x1111, sum1 + 4, 9 - g->wmax, n & 1);
    dbprintf(2, "/%d", veven);
    if(veven < 0 || veven > g->teven)
        return(ZBAR_NONE);

    int v = g->sum;
    if(n & 2)
        v += vodd + veven * g->todd;
    else
        v += veven + vodd * g->teven;

    dbprintf(2, " f=%d(%x%x%x)", seg->finder, seg->exp, seg->color, seg->side);

    unsigned chk = 0;
    if(seg->exp) {
        unsigned side = seg->color ^ seg->side ^ 1;
        if(v >= 4096)
            return(ZBAR_NONE);
        /* skip A1 left */
        chk = calc_check(sig0, sig1, side, 211);
        if(seg->finder || seg->color || seg->side) {
            i = (seg->finder << 1) - side + seg->color;
            zassert(i >= 0 && i < 12, ZBAR_NONE,
                    "f=%d(%x%x%x) side=%d i=%d\n",
                    seg->finder, seg->exp, seg->color, seg->side, side, i);
            chk = (chk * exp_checksums[i]) % 211;
        }
        else if(v >= 4009)
            return(ZBAR_NONE);
        else
            chk = 0;
    }
    else {
        chk = calc_check(sig0, sig1, seg->side, 79);
        if(seg->color)
            chk = (chk * 16) % 79;
    }
    dbprintf(2, " => %d val=%d", chk, v);

    seg->check = chk;
    seg->data = v;

    merge_segment(db, seg);

    if(seg->exp)
        return(match_segment_exp(dcode, seg, dir));
    else if(dir > 0)
        return(match_segment(dcode, seg));
    return(ZBAR_PARTIAL);
}

static inline int
alloc_segment (databar_decoder_t *db)
{
    unsigned maxage = 0, csegs = db->csegs;
    int i, old = -1;
    for(i = 0; i < csegs; i++) {
        databar_segment_t *seg = db->segs + i;
        unsigned age;
        if(seg->finder < 0) {
            dbprintf(2, " free@%d", i);
            return(i);
        }
        age = (db->epoch - seg->epoch) & 0xff;
        if(age >= 128 && seg->count < 2) {
            seg->finder = -1;
            dbprintf(2, " stale@%d (%d - %d = %d)",
                     i, db->epoch, seg->epoch, age);
            return(i);
        }

        /* score based on both age and count */
        if(age > seg->count)
            age = age - seg->count + 1;
        else
            age = 1;

        if(maxage < age) {
            maxage = age;
            old = i;
            dbprintf(2, " old@%d(%u)", i, age);
        }
    }

    if(csegs < DATABAR_MAX_SEGMENTS) {
        dbprintf(2, " new@%d", i);
        i = csegs;
        csegs *= 2;
        if(csegs > DATABAR_MAX_SEGMENTS)
            csegs = DATABAR_MAX_SEGMENTS;
        if(csegs != db->csegs) {
            databar_segment_t *seg;
            db->segs = realloc(db->segs, csegs * sizeof(*db->segs));
            db->csegs = csegs;
            seg = db->segs + csegs;
            while(--seg, --csegs >= i) {
                seg->finder = -1;
                seg->exp = 0;
                seg->color = 0;
                seg->side = 0;
                seg->partial = 0;
                seg->count = 0;
                seg->epoch = 0;
                seg->check = 0;
            }
            return(i);
        }
    }
    zassert(old >= 0, -1, "\n");

    db->segs[old].finder = -1;
    return(old);
}

static inline zbar_symbol_type_t
decode_finder (zbar_decoder_t *dcode)
{
    databar_decoder_t *db = &dcode->databar;
    databar_segment_t *seg;
    unsigned e0 = pair_width(dcode, 1);
    unsigned e2 = pair_width(dcode, 3);
    unsigned e1, e3, s, finder, dir;
    int sig, iseg;
    dbprintf(2, "      databar: e0=%d e2=%d", e0, e2);
    if(e0 < e2) {
        unsigned e = e2 * 4;
        if(e < 15 * e0 || e > 34 * e0)
            return(ZBAR_NONE);
        dir = 0;
        e3 = pair_width(dcode, 4);
    }
    else {
        unsigned e = e0 * 4;
        if(e < 15 * e2 || e > 34 * e2)
            return(ZBAR_NONE);
        dir = 1;
        e2 = e0;
        e3 = pair_width(dcode, 0);
    }
    e1 = pair_width(dcode, 2);

    s = e1 + e3;
    dbprintf(2, " e1=%d e3=%d dir=%d s=%d", e1, e3, dir, s);
    if(s < 12)
        return(ZBAR_NONE);

    sig = ((decode_e(e3, s, 14) << 8) | (decode_e(e2, s, 14) << 4) |
           decode_e(e1, s, 14));
    dbprintf(2, " sig=%04x", sig & 0xfff);
    if(sig < 0 ||
       ((sig >> 4) & 0xf) < 8 ||
       ((sig >> 4) & 0xf) > 10 ||
       (sig & 0xf) >= 10 ||
       ((sig >> 8) & 0xf) >= 10 ||
       (((sig >> 8) + sig) & 0xf) != 10)
        return(ZBAR_NONE);

    finder = (finder_hash[(sig - (sig >> 5)) & 0x1f] +
              finder_hash[(sig >> 1) & 0x1f]) & 0x1f;
    dbprintf(2, " finder=%d", finder);
    if(finder == 0x1f ||
       !TEST_CFG((finder < 9) ? db->config : db->config_exp, ZBAR_CFG_ENABLE))
        return(ZBAR_NONE);

    zassert(finder >= 0, ZBAR_NONE, "dir=%d sig=%04x f=%d\n",
            dir, sig & 0xfff, finder);

    iseg = alloc_segment(db);
    if(iseg < 0)
        return(ZBAR_NONE);

    seg = db->segs + iseg;
    seg->finder = (finder >= 9) ? finder - 9 : finder;
    seg->exp = (finder >= 9);
    seg->color = get_color(dcode) ^ dir ^ 1;
    seg->side = dir;
    seg->partial = 0;
    seg->count = 1;
    seg->width = s;
    seg->epoch = db->epoch;

    int rc = decode_char(dcode, seg, 12 - dir, -1);
    if(!rc)
        seg->partial = 1;
    else
        db->epoch++;

    int i = (dcode->idx + 8 + dir) & 0xf;
    zassert(db->chars[i] == -1, ZBAR_NONE, "\n");
    db->chars[i] = iseg;
    return(rc);
}

zbar_symbol_type_t
_zbar_decode_databar (zbar_decoder_t *dcode)
{
    databar_decoder_t *db = &dcode->databar;
    databar_segment_t *seg, *pair;
    zbar_symbol_type_t sym;
    int iseg, i = dcode->idx & 0xf;

    sym = decode_finder(dcode);
    dbprintf(2, "\n");

    iseg = db->chars[i];
    if(iseg < 0)
        return(sym);

    db->chars[i] = -1;
    seg = db->segs + iseg;
    dbprintf(2, "        databar: i=%d part=%d f=%d(%x%x%x)",
             iseg, seg->partial, seg->finder, seg->exp, seg->color, seg->side);
    zassert(seg->finder >= 0, ZBAR_NONE, "i=%d f=%d(%x%x%x) part=%x\n",
            iseg, seg->finder, seg->exp, seg->color, seg->side, seg->partial);

    if(seg->partial) {
        pair = NULL;
        seg->side = !seg->side;
    }
    else {
        int jseg = alloc_segment(db);
        pair = db->segs + iseg;
        seg = db->segs + jseg;
        seg->finder = pair->finder;
        seg->exp = pair->exp;
        seg->color = pair->color;
        seg->side = !pair->side;
        seg->partial = 0;
        seg->count = 1;
        seg->width = pair->width;
        seg->epoch = db->epoch;
    }

    sym = decode_char(dcode, seg, 1, 1);
    if(!sym) {
        seg->finder = -1;
        if(pair)
            pair->partial = 1;
    }
    else
        db->epoch++;
    dbprintf(2, "\n");

    return(sym);
}
