//------------------------------------------------------------------------
//  Copyright 2007-2009 (c) Jeff Brown <spadix@users.sourceforge.net>
//
//  This file is part of the ZBar Bar Code Reader.
//
//  The ZBar Bar Code Reader is free software; you can redistribute it
//  and/or modify it under the terms of the GNU Lesser Public License as
//  published by the Free Software Foundation; either version 2.1 of
//  the License, or (at your option) any later version.
//
//  The ZBar Bar Code Reader is distributed in the hope that it will be
//  useful, but WITHOUT ANY WARRANTY; without even the implied warranty
//  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser Public License for more details.
//
//  You should have received a copy of the GNU Lesser Public License
//  along with the ZBar Bar Code Reader; if not, write to the Free
//  Software Foundation, Inc., 51 Franklin St, Fifth Floor,
//  Boston, MA  02110-1301  USA
//
//  http://sourceforge.net/projects/zbar
//------------------------------------------------------------------------
#ifndef _ZBAR_SCANNER_H_
#define _ZBAR_SCANNER_H_

/// @file
/// Scanner C++ wrapper

#ifndef _ZBAR_H_
# error "include zbar.h in your application, **not** zbar/Scanner.h"
#endif

#include <stdio.h>

namespace zbar {

/// low-level linear intensity sample stream scanner interface.
/// identifies "bar" edges and measures width between them.
/// optionally passes to bar width Decoder

class Scanner {
 public:

    /// constructor.
    /// @param decoder reference to a Decoder instance which will
    /// be passed scan results automatically
    Scanner (Decoder& decoder)
    {
        _scanner = zbar_scanner_create(decoder._decoder);
    }

    /// constructor.
    /// @param decoder pointer to a Decoder instance which will
    /// be passed scan results automatically
    Scanner (Decoder* decoder = NULL)
    {
        zbar_decoder_t *zdcode = NULL;
        if(decoder)
            zdcode = decoder->_decoder;
        _scanner = zbar_scanner_create(zdcode);
    }

    ~Scanner ()
    {
        zbar_scanner_destroy(_scanner);
    }

    /// clear all scanner state.
    /// see zbar_scanner_reset()
    void reset ()
    {
        zbar_scanner_reset(_scanner);
    }

    /// mark start of a new scan pass.
    /// see zbar_scanner_new_scan()
    zbar_symbol_type_t new_scan ()
    {
        _type = zbar_scanner_new_scan(_scanner);
        return(_type);
    }

    /// flush scanner pipeline.
    /// see zbar_scanner_flush()
    zbar_symbol_type_t flush ()
    {
        _type = zbar_scanner_flush(_scanner);
        return(_type);
    }

    /// process next sample intensity value.
    /// see zbar_scan_y()
    zbar_symbol_type_t scan_y (int y)
    {
        _type = zbar_scan_y(_scanner, y);
        return(_type);
    }

    /// process next sample intensity value.
    /// see zbar_scan_y()
    Scanner& operator<< (int y)
    {
        _type = zbar_scan_y(_scanner, y);
        return(*this);
    }

    /// process next sample from RGB (or BGR) triple.
    /// see zbar_scan_rgb24()
    zbar_symbol_type_t scan_rgb24 (unsigned char *rgb)
    {
        _type = zbar_scan_rgb24(_scanner, rgb);
        return(_type);
    }

    /// process next sample from RGB (or BGR) triple.
    /// see zbar_scan_rgb24()
    Scanner& operator<< (unsigned char *rgb)
    {
        _type = zbar_scan_rgb24(_scanner, rgb);
        return(*this);
    }

    /// retrieve last scanned width.
    /// see zbar_scanner_get_width()
    unsigned get_width () const
    {
        return(zbar_scanner_get_width(_scanner));
    }

    /// retrieve last scanned color.
    /// see zbar_scanner_get_color()
    zbar_color_t get_color () const
    {
        return(zbar_scanner_get_color(_scanner));
    }

    /// retrieve last scan result.
    zbar_symbol_type_t get_type () const
    {
        return(_type);
    }

    /// cast to C scanner
    operator zbar_scanner_t* () const
    {
        return(_scanner);
    }

    /// retrieve C scanner
    const zbar_scanner_t *get_c_scanner () const
    {
        return(_scanner);
    }

 private:
    zbar_scanner_t *_scanner;
    zbar_symbol_type_t _type;
};

}

#endif
