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
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <zbar.h>
#include "symbol.h"

const char *zbar_get_symbol_name (zbar_symbol_type_t sym)
{
    switch(sym & ZBAR_SYMBOL) {
    case ZBAR_EAN2: return("EAN-2");
    case ZBAR_EAN5: return("EAN-5");
    case ZBAR_EAN8: return("EAN-8");
    case ZBAR_UPCE: return("UPC-E");
    case ZBAR_ISBN10: return("ISBN-10");
    case ZBAR_UPCA: return("UPC-A");
    case ZBAR_EAN13: return("EAN-13");
    case ZBAR_ISBN13: return("ISBN-13");
    case ZBAR_COMPOSITE: return("COMPOSITE");
    case ZBAR_I25: return("I2/5");
    case ZBAR_DATABAR: return("DataBar");
    case ZBAR_DATABAR_EXP: return("DataBar-Exp");
    case ZBAR_CODABAR: return("Codabar");
    case ZBAR_CODE39: return("CODE-39");
    case ZBAR_CODE93: return("CODE-93");
    case ZBAR_CODE128: return("CODE-128");
    case ZBAR_PDF417: return("PDF417");
    case ZBAR_QRCODE: return("QR-Code");
    default: return("UNKNOWN");
    }
}

const char *zbar_get_addon_name (zbar_symbol_type_t sym)
{
    return("");
}

const char *zbar_get_config_name (zbar_config_t cfg)
{
    switch(cfg) {
    case ZBAR_CFG_ENABLE: return("ENABLE");
    case ZBAR_CFG_ADD_CHECK: return("ADD_CHECK");
    case ZBAR_CFG_EMIT_CHECK: return("EMIT_CHECK");
    case ZBAR_CFG_ASCII: return("ASCII");
    case ZBAR_CFG_MIN_LEN: return("MIN_LEN");
    case ZBAR_CFG_MAX_LEN: return("MAX_LEN");
    case ZBAR_CFG_UNCERTAINTY: return("UNCERTAINTY");
    case ZBAR_CFG_POSITION: return("POSITION");
    case ZBAR_CFG_X_DENSITY: return("X_DENSITY");
    case ZBAR_CFG_Y_DENSITY: return("Y_DENSITY");
    default: return("");
    }
}

const char *zbar_get_modifier_name (zbar_modifier_t mod)
{
    switch(mod) {
    case ZBAR_MOD_GS1: return("GS1");
    case ZBAR_MOD_AIM: return("AIM");
    default: return("");
    }
}

const char *zbar_get_orientation_name (zbar_orientation_t orient)
{
    switch(orient) {
    case ZBAR_ORIENT_UP: return("UP");
    case ZBAR_ORIENT_RIGHT: return("RIGHT");
    case ZBAR_ORIENT_DOWN: return("DOWN");
    case ZBAR_ORIENT_LEFT: return("LEFT");
    default: return("UNKNOWN");
    }
}

int _zbar_get_symbol_hash (zbar_symbol_type_t sym)
{
    static const signed char hash[0x20] = {
        0x00, 0x01, 0x10, 0x11,   -1, 0x11, 0x16, 0x0c,
	0x05, 0x06, 0x08,   -1, 0x04, 0x03, 0x07, 0x12,
	  -1,   -1,   -1,   -1,   -1,   -1,   -1, 0x02,
	  -1, 0x00, 0x12, 0x0c, 0x0b, 0x1d, 0x0a, 0x00,
     };
    int g0 = hash[sym & 0x1f];
    int g1 = hash[~(sym >> 4) & 0x1f];
    assert(g0 >= 0 && g1 >= 0);
    if(g0 < 0 || g1 < 0)
        return(0);
    return((g0 + g1) & 0x1f);
}

void _zbar_symbol_free (zbar_symbol_t *sym)
{
    if(sym->syms) {
        zbar_symbol_set_ref(sym->syms, -1);
        sym->syms = NULL;
    }
    if(sym->pts)
        free(sym->pts);
    if(sym->data_alloc && sym->data)
        free(sym->data);
    free(sym);
}

void zbar_symbol_ref (const zbar_symbol_t *sym,
                      int refs)
{
    zbar_symbol_t *ncsym = (zbar_symbol_t*)sym;
    _zbar_symbol_refcnt(ncsym, refs);
}

zbar_symbol_type_t zbar_symbol_get_type (const zbar_symbol_t *sym)
{
    return(sym->type);
}

unsigned int zbar_symbol_get_configs (const zbar_symbol_t *sym)
{
    return(sym->configs);
}

unsigned int zbar_symbol_get_modifiers (const zbar_symbol_t *sym)
{
    return(sym->modifiers);
}

const char *zbar_symbol_get_data (const zbar_symbol_t *sym)
{
    return(sym->data);
}

unsigned int zbar_symbol_get_data_length (const zbar_symbol_t *sym)
{
    return(sym->datalen);
}

int zbar_symbol_get_count (const zbar_symbol_t *sym)
{
    return(sym->cache_count);
}

int zbar_symbol_get_quality (const zbar_symbol_t *sym)
{
    return(sym->quality);
}

unsigned zbar_symbol_get_loc_size (const zbar_symbol_t *sym)
{
    return(sym->npts);
}

int zbar_symbol_get_loc_x (const zbar_symbol_t *sym,
                           unsigned idx)
{
    if(idx < sym->npts)
        return(sym->pts[idx].x);
    else
        return(-1);
}

int zbar_symbol_get_loc_y (const zbar_symbol_t *sym,
                           unsigned idx)
{
    if(idx < sym->npts)
        return(sym->pts[idx].y);
    else
        return(-1);
}

zbar_orientation_t zbar_symbol_get_orientation (const zbar_symbol_t *sym)
{
    return(sym->orient);
}

const zbar_symbol_t *zbar_symbol_next (const zbar_symbol_t *sym)
{
    return((sym) ? sym->next : NULL);
}

const zbar_symbol_set_t*
zbar_symbol_get_components (const zbar_symbol_t *sym)
{
    return(sym->syms);
}

const zbar_symbol_t *zbar_symbol_first_component (const zbar_symbol_t *sym)
{
    return((sym && sym->syms) ? sym->syms->head : NULL);
}


unsigned base64_encode (char *dst,
                        const char *src,
                        unsigned srclen)
{
    static const char alphabet[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char *start = dst;
    int nline = 19;
    for(; srclen; srclen -= 3) {
        unsigned int buf = *(src++) << 16;
        if(srclen > 1) buf |= *(src++) << 8;
        if(srclen > 2) buf |= *(src++);
        *(dst++) = alphabet[(buf >> 18) & 0x3f];
        *(dst++) = alphabet[(buf >> 12) & 0x3f];
        *(dst++) = (srclen > 1) ? alphabet[(buf >> 6) & 0x3f] : '=';
        *(dst++) = (srclen > 2) ? alphabet[buf & 0x3f] : '=';
        if(srclen < 3) break;
        if(!--nline) { *(dst++) = '\n'; nline = 19; }
    }
    *(dst++) = '\n';
    *(dst++) = '\0';
    return(dst - start - 1);
}

enum {
    TMPL_START,
    TMPL_MOD_START, TMPL_MOD_ITEM, TMPL_MOD_END,
    TMPL_COUNT,
    TMPL_DATA_START, TMPL_FORMAT, TMPL_CDATA,
    TMPL_NL,
    TMPL_END,
};

/* FIXME suspect... */
#define MAX_STATIC 256
#define MAX_MOD (5 * ZBAR_MOD_NUM)
#define MAX_CFG (10 * ZBAR_CFG_NUM)
#define MAX_INT_DIGITS 10

#define TMPL_COPY(t) do {             \
        static const char *_st = (t); \
        i = strlen(_st);              \
        memcpy(*buf + n, _st, i + 1); \
        n += i;                       \
        assert(n <= maxlen);          \
    } while(0)

#define TMPL_FMT(t, ...) do {                                 \
        static const char *_st = (t);                         \
        i = snprintf(*buf + n, maxlen - n, _st, __VA_ARGS__); \
        assert(i > 0);                                        \
        n += i;                                               \
        assert(n <= maxlen);                                  \
    } while(0)

char *zbar_symbol_xml (const zbar_symbol_t *sym,
                       char **buf,
                       unsigned *len)
{
    unsigned int datalen, maxlen;
    int i, n = 0;

    const char *type = zbar_get_symbol_name(sym->type);
    const char *orient = zbar_get_orientation_name(sym->orient);

    /* check for binary data */
    unsigned char *data = (unsigned char*)sym->data;
    char binary = ((data[0] == 0xff && data[1] == 0xfe) ||
                   (data[0] == 0xfe && data[1] == 0xff) ||
                   !strncmp(sym->data, "<?xml", 5));
    for(i = 0; !binary && i < sym->datalen; i++) {
        unsigned char c = sym->data[i];
        binary = ((c < 0x20 && ((~0x00002600 >> c) & 1)) ||
                  (c >= 0x7f && c < 0xa0) ||
                  (c == ']' && i + 2 < sym->datalen &&
                   sym->data[i + 1] == ']' &&
                   sym->data[i + 2] == '>'));
    }

    datalen = strlen(sym->data);
    if(binary)
        datalen = (sym->datalen + 2) / 3 * 4 + sym->datalen / 57 + 3;

    maxlen = (MAX_STATIC + strlen(type) + strlen(orient) +
              datalen + MAX_INT_DIGITS + 1);
    unsigned int mods = sym->modifiers;
    if(mods)
        maxlen += MAX_MOD;
    unsigned int cfgs = sym->configs & ~(1 << ZBAR_CFG_ENABLE);
    if(cfgs)
        maxlen += MAX_CFG;
    if(binary)
        maxlen += MAX_INT_DIGITS;

    if(!*buf || (*len < maxlen)) {
        if(*buf)
            free(*buf);
        *buf = malloc(maxlen);
        /* FIXME check OOM */
        *len = maxlen;
    }

    TMPL_FMT("<symbol type='%s' quality='%d' orientation='%s'",
             type, sym->quality, orient);

    if(mods) {
        int j;
        TMPL_COPY(" modifiers='");
        for(j = 0; mods && j < ZBAR_MOD_NUM; j++, mods >>= 1)
            if(mods & 1)
                TMPL_FMT("%s ", zbar_get_modifier_name(j));
        /* cleanup trailing space */
        n--;
        TMPL_COPY("'");
    }

    if(cfgs) {
        int j;
        TMPL_COPY(" configs='");
        for(j = 0; cfgs && j < ZBAR_CFG_NUM; j++, cfgs >>= 1)
            if(cfgs & 1)
                TMPL_FMT("%s ", zbar_get_config_name(j));
        /* cleanup trailing space */
        n--;
        TMPL_COPY("'");
    }

    if(sym->cache_count)
        TMPL_FMT(" count='%d'", sym->cache_count);

    TMPL_COPY("><data");
    if(binary)
        TMPL_FMT(" format='base64' length='%d'", sym->datalen);
    TMPL_COPY("><![CDATA[");

    if(!binary) {
        memcpy(*buf + n, sym->data, sym->datalen + 1);
        n += sym->datalen;
    }
    else {
        TMPL_COPY("\n");
        n += base64_encode(*buf + n, sym->data, sym->datalen);
    }
    assert(n <= maxlen);

    TMPL_COPY("]]></data></symbol>");

    *len = n;
    return(*buf);
}


zbar_symbol_set_t *_zbar_symbol_set_create ()
{
    zbar_symbol_set_t *syms = calloc(1, sizeof(*syms));
    _zbar_refcnt(&syms->refcnt, 1);
    return(syms);
}

inline void _zbar_symbol_set_free (zbar_symbol_set_t *syms)
{
    zbar_symbol_t *sym, *next;
    for(sym = syms->head; sym; sym = next) {
        next = sym->next;
        sym->next = NULL;
        _zbar_symbol_refcnt(sym, -1);
    }
    syms->head = NULL;
    free(syms);
}

void zbar_symbol_set_ref (const zbar_symbol_set_t *syms,
                          int delta)
{
    zbar_symbol_set_t *ncsyms = (zbar_symbol_set_t*)syms;
    if(!_zbar_refcnt(&ncsyms->refcnt, delta) && delta <= 0)
        _zbar_symbol_set_free(ncsyms);
}

int zbar_symbol_set_get_size (const zbar_symbol_set_t *syms)
{
    return(syms->nsyms);
}

const zbar_symbol_t*
zbar_symbol_set_first_symbol (const zbar_symbol_set_t *syms)
{
    zbar_symbol_t *sym = syms->tail;
    if(sym)
        return(sym->next);
    return(syms->head);
}

const zbar_symbol_t*
zbar_symbol_set_first_unfiltered (const zbar_symbol_set_t *syms)
{
    return(syms->head);
}
