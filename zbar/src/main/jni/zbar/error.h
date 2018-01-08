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
#ifndef _ERROR_H_
#define _ERROR_H_

#include <config.h>
#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_ERRNO_H
# include <errno.h>
#endif
#include <assert.h>

#include <zbar.h>

#ifdef _WIN32
# include <windows.h>
#endif

#if __STDC_VERSION__ < 199901L
# if __GNUC__ >= 2
#  define __func__ __FUNCTION__
# else
#  define __func__ "<unknown>"
# endif
#endif

#define ERRINFO_MAGIC (0x5252457a) /* "zERR" (LE) */

typedef enum errsev_e {
    SEV_FATAL   = -2,           /* application must terminate */
    SEV_ERROR   = -1,           /* might be able to recover and continue */
    SEV_OK      =  0,
    SEV_WARNING =  1,           /* unexpected condition */
    SEV_NOTE    =  2,           /* fyi */
} errsev_t;

typedef enum errmodule_e {
    ZBAR_MOD_PROCESSOR,
    ZBAR_MOD_VIDEO,
    ZBAR_MOD_WINDOW,
    ZBAR_MOD_IMAGE_SCANNER,
    ZBAR_MOD_UNKNOWN,
} errmodule_t;

typedef struct errinfo_s {
    uint32_t magic;             /* just in case */
    errmodule_t module;         /* reporting module */
    char *buf;                  /* formatted and passed to application */
    int errnum;                 /* errno for system errors */

    errsev_t sev;
    zbar_error_t type;
    const char *func;           /* reporting function */
    const char *detail;         /* description */
    char *arg_str;              /* single string argument */
    int arg_int;                /* single integer argument */
} errinfo_t;

extern int _zbar_verbosity;

/* FIXME don't we need varargs hacks here? */

#ifdef _WIN32
# define ZFLUSH fflush(stderr);
#else
# define ZFLUSH
#endif

#ifdef ZNO_MESSAGES

# ifdef __GNUC__
    /* older versions of gcc (< 2.95) require a named varargs parameter */
#  define zprintf(args...)
# else
    /* unfortunately named vararg parameter is a gcc-specific extension */
#  define zprintf(...)
# endif

#else

# ifdef __GNUC__
#  define zprintf(level, format, args...) do {                          \
        if(_zbar_verbosity >= level) {                                  \
            fprintf(stderr, "%s: " format, __func__ , ##args);          \
            ZFLUSH                                                      \
        }                                                               \
    } while(0)
# else
#  define zprintf(level, format, ...) do {                              \
        if(_zbar_verbosity >= level) {                                  \
            fprintf(stderr, "%s: " format, __func__ , ##__VA_ARGS__);   \
            ZFLUSH                                                      \
        }                                                               \
    } while(0)
# endif

#endif

static inline int err_copy (void *dst_c,
                            void *src_c)
{
    errinfo_t *dst = dst_c;
    errinfo_t *src = src_c;
    assert(dst->magic == ERRINFO_MAGIC);
    assert(src->magic == ERRINFO_MAGIC);

    dst->errnum = src->errnum;
    dst->sev = src->sev;
    dst->type = src->type;
    dst->func = src->func;
    dst->detail = src->detail;
    dst->arg_str = src->arg_str;
    src->arg_str = NULL; /* unused at src, avoid double free */
    dst->arg_int = src->arg_int;
    return(-1);
}

static inline int err_capture (const void *container,
                               errsev_t sev,
                               zbar_error_t type,
                               const char *func,
                               const char *detail)
{
    errinfo_t *err = (errinfo_t*)container;
    assert(err->magic == ERRINFO_MAGIC);
#ifdef HAVE_ERRNO_H
    if(type == ZBAR_ERR_SYSTEM)
        err->errnum = errno;
#endif
#ifdef _WIN32
    if(type == ZBAR_ERR_WINAPI)
        err->errnum = GetLastError();
#endif
    err->sev = sev;
    err->type = type;
    err->func = func;
    err->detail = detail;
    if(_zbar_verbosity >= 1)
        _zbar_error_spew(err, 0);
    return(-1);
}

static inline int err_capture_str (const void *container,
                                   errsev_t sev,
                                   zbar_error_t type,
                                   const char *func,
                                   const char *detail,
                                   const char *arg)
{
    errinfo_t *err = (errinfo_t*)container;
    assert(err->magic == ERRINFO_MAGIC);
    if(err->arg_str)
        free(err->arg_str);
    err->arg_str = strdup(arg);
    return(err_capture(container, sev, type, func, detail));
}

static inline int err_capture_int (const void *container,
                                   errsev_t sev,
                                   zbar_error_t type,
                                   const char *func,
                                   const char *detail,
                                   int arg)
{
    errinfo_t *err = (errinfo_t*)container;
    assert(err->magic == ERRINFO_MAGIC);
    err->arg_int = arg;
    return(err_capture(container, sev, type, func, detail));
}

static inline int err_capture_num (const void *container,
                                   errsev_t sev,
                                   zbar_error_t type,
                                   const char *func,
                                   const char *detail,
                                   int num)
{
    errinfo_t *err = (errinfo_t*)container;
    assert(err->magic == ERRINFO_MAGIC);
    err->errnum = num;
    return(err_capture(container, sev, type, func, detail));
}

static inline void err_init (errinfo_t *err,
                             errmodule_t module)
{
    err->magic = ERRINFO_MAGIC;
    err->module = module;
}

static inline void err_cleanup (errinfo_t *err)
{
    assert(err->magic == ERRINFO_MAGIC);
    if(err->buf) {
        free(err->buf);
        err->buf = NULL;
    }
    if(err->arg_str) {
        free(err->arg_str);
        err->arg_str = NULL;
    }
}

#endif
