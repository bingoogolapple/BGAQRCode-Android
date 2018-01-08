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

#include "error.h"
#include <string.h>

int _zbar_verbosity = 0;

static const char * const sev_str[] = {
    "FATAL ERROR", "ERROR", "OK", "WARNING", "NOTE"
};
#define SEV_MAX (strlen(sev_str[0]))

static const char * const mod_str[] = {
    "processor", "video", "window", "image scanner", "<unknown>"
};
#define MOD_MAX (strlen(mod_str[ZBAR_MOD_IMAGE_SCANNER]))

static const char * const err_str[] = {
    "no error",                 /* OK */
    "out of memory",            /* NOMEM */
    "internal library error",   /* INTERNAL */
    "unsupported request",      /* UNSUPPORTED */
    "invalid request",          /* INVALID */
    "system error",             /* SYSTEM */
    "locking error",            /* LOCKING */
    "all resources busy",       /* BUSY */
    "X11 display error",        /* XDISPLAY */
    "X11 protocol error",       /* XPROTO */
    "output window is closed",  /* CLOSED */
    "windows system error",     /* WINAPI */
    "unknown error"             /* NUM */
};
#define ERR_MAX (strlen(err_str[ZBAR_ERR_CLOSED]))

int zbar_version (unsigned *major,
                  unsigned *minor)
{
    if(major)
        *major = ZBAR_VERSION_MAJOR;
    if(minor)
        *minor = ZBAR_VERSION_MINOR;
    return(0);
}

void zbar_set_verbosity (int level)
{
    _zbar_verbosity = level;
}

void zbar_increase_verbosity ()
{
    if(!_zbar_verbosity)
        _zbar_verbosity++;
    else
        _zbar_verbosity <<= 1;
}

int _zbar_error_spew (const void *container,
                      int verbosity)
{
    const errinfo_t *err = container;
    assert(err->magic == ERRINFO_MAGIC);
    fprintf(stderr, "%s", _zbar_error_string(err, verbosity));
    return(-err->sev);
}

zbar_error_t _zbar_get_error_code (const void *container)
{
    const errinfo_t *err = container;
    assert(err->magic == ERRINFO_MAGIC);
    return(err->type);
}

/* ERROR: zbar video in v4l1_set_format():
 *     system error: blah[: blah]
 */

const char *_zbar_error_string (const void *container,
                                int verbosity)
{
    static const char basefmt[] = "%s: zbar %s in %s():\n    %s: ";
    errinfo_t *err = (errinfo_t*)container;
    const char *sev, *mod, *func, *type;
    int len;

    assert(err->magic == ERRINFO_MAGIC);

    if(err->sev >= SEV_FATAL && err->sev <= SEV_NOTE)
        sev = sev_str[err->sev + 2];
    else
        sev = sev_str[1];

    if(err->module >= ZBAR_MOD_PROCESSOR &&
       err->module < ZBAR_MOD_UNKNOWN)
        mod = mod_str[err->module];
    else
        mod = mod_str[ZBAR_MOD_UNKNOWN];

    func = (err->func) ? err->func : "<unknown>";

    if(err->type >= 0 && err->type < ZBAR_ERR_NUM)
        type = err_str[err->type];
    else
        type = err_str[ZBAR_ERR_NUM];

    len = SEV_MAX + MOD_MAX + ERR_MAX + strlen(func) + sizeof(basefmt);
    err->buf = realloc(err->buf, len);
    len = sprintf(err->buf, basefmt, sev, mod, func, type);
    if(len <= 0)
        return("<unknown>");

    if(err->detail) {
        int newlen = len + strlen(err->detail) + 1;
        if(strstr(err->detail, "%s")) {
            if(!err->arg_str)
                err->arg_str = strdup("<?>");
            err->buf = realloc(err->buf, newlen + strlen(err->arg_str));
            len += sprintf(err->buf + len, err->detail, err->arg_str);
        }
        else if(strstr(err->detail, "%d") || strstr(err->detail, "%x")) {
            err->buf = realloc(err->buf, newlen + 32);
            len += sprintf(err->buf + len, err->detail, err->arg_int);
        }
        else {
            err->buf = realloc(err->buf, newlen);
            len += sprintf(err->buf + len, "%s", err->detail);
        }
        if(len <= 0)
            return("<unknown>");
    }

#ifdef HAVE_ERRNO_H
    if(err->type == ZBAR_ERR_SYSTEM) {
        static const char sysfmt[] = ": %s (%d)\n";
        const char *syserr = strerror(err->errnum);
        err->buf = realloc(err->buf, len + strlen(sysfmt) + strlen(syserr));
        len += sprintf(err->buf + len, sysfmt, syserr, err->errnum);
    }
#endif
#ifdef _WIN32
    else if(err->type == ZBAR_ERR_WINAPI) {
        char *syserr = NULL;
        if(FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
                         FORMAT_MESSAGE_ALLOCATE_BUFFER |
                         FORMAT_MESSAGE_IGNORE_INSERTS,
                         NULL, err->errnum, 0, (LPTSTR)&syserr, 1, NULL) &&
           syserr) {
            char sysfmt[] = ": %s (%d)\n";
            err->buf = realloc(err->buf, len + strlen(sysfmt) + strlen(syserr));
            len += sprintf(err->buf + len, sysfmt, syserr, err->errnum);
            LocalFree(syserr);
        }
    }
#endif
    else {
        err->buf = realloc(err->buf, len + 2);
        len += sprintf(err->buf + len, "\n");
    }
    return(err->buf);
}
