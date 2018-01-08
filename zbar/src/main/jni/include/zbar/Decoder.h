//------------------------------------------------------------------------
//  Copyright 2007-2010 (c) Jeff Brown <spadix@users.sourceforge.net>
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
#ifndef _ZBAR_DECODER_H_
#define _ZBAR_DECODER_H_

/// @file
/// Decoder C++ wrapper

#ifndef _ZBAR_H_
# error "include zbar.h in your application, **not** zbar/Decoder.h"
#endif

#include <string>

namespace zbar {

/// low-level bar width stream decoder interface.
/// identifies symbols and extracts encoded data

class Decoder {
 public:

    /// Decoder result handler.
    /// applications should subtype this and pass an instance to
    /// set_handler() to implement result processing
    class Handler {
    public:
        virtual ~Handler() { }

        /// invoked by the Decoder as decode results become available.
        virtual void decode_callback(Decoder &decoder) = 0;
    };

    /// constructor.
    Decoder ()
        : _handler(NULL)
    {
        _decoder = zbar_decoder_create();
    }

    ~Decoder ()
    {
        zbar_decoder_destroy(_decoder);
    }

    /// clear all decoder state.
    /// see zbar_decoder_reset()
    void reset ()
    {
        zbar_decoder_reset(_decoder);
    }

    /// mark start of a new scan pass.
    /// see zbar_decoder_new_scan()
    void new_scan ()
    {
        zbar_decoder_new_scan(_decoder);
    }

    /// process next bar/space width from input stream.
    /// see zbar_decode_width()
    zbar_symbol_type_t decode_width (unsigned width)
    {
        return(zbar_decode_width(_decoder, width));
    }

    /// process next bar/space width from input stream.
    /// see zbar_decode_width()
    Decoder& operator<< (unsigned width)
    {
        zbar_decode_width(_decoder, width);
        return(*this);
    }

    /// retrieve color of @em next element passed to Decoder.
    /// see zbar_decoder_get_color()
    zbar_color_t get_color () const
    {
        return(zbar_decoder_get_color(_decoder));
    }

    /// retrieve last decoded symbol type.
    /// see zbar_decoder_get_type()
    zbar_symbol_type_t get_type () const
    {
        return(zbar_decoder_get_type(_decoder));
    }

    /// retrieve string name of last decoded symbol type.
    /// see zbar_get_symbol_name()
    const char *get_symbol_name () const
    {
        return(zbar_get_symbol_name(zbar_decoder_get_type(_decoder)));
    }

    /// retrieve string name for last decode addon.
    /// see zbar_get_addon_name()
    /// @deprecated in 0.11
    const char *get_addon_name () const
    {
        return(zbar_get_addon_name(zbar_decoder_get_type(_decoder)));
    }

    /// retrieve last decoded data in ASCII format as a char array.
    /// see zbar_decoder_get_data()
    const char *get_data_chars() const
    {
        return(zbar_decoder_get_data(_decoder));
    }

    /// retrieve last decoded data as a std::string.
    /// see zbar_decoder_get_data()
    const std::string get_data_string() const
    {
        return(std::string(zbar_decoder_get_data(_decoder),
                           zbar_decoder_get_data_length(_decoder)));
    }

    /// retrieve last decoded data as a std::string.
    /// see zbar_decoder_get_data()
    const std::string get_data() const
    {
        return(get_data_string());
    }

    /// retrieve length of decoded binary data.
    /// see zbar_decoder_get_data_length()
    int get_data_length() const
    {
        return(zbar_decoder_get_data_length(_decoder));
    }

    /// retrieve last decode direction.
    /// see zbar_decoder_get_direction()
    /// @since 0.11
    int get_direction() const
    {
        return(zbar_decoder_get_direction(_decoder));
    }

    /// setup callback to handle result data.
    void set_handler (Handler &handler)
    {
        _handler = &handler;
        zbar_decoder_set_handler(_decoder, _cb);
        zbar_decoder_set_userdata(_decoder, this);
    }

    /// set config for indicated symbology (0 for all) to specified value.
    /// @see zbar_decoder_set_config()
    /// @since 0.4
    int set_config (zbar_symbol_type_t symbology,
                    zbar_config_t config,
                    int value)
    {
        return(zbar_decoder_set_config(_decoder, symbology, config, value));
    }

    /// set config parsed from configuration string.
    /// @see zbar_decoder_parse_config()
    /// @since 0.4
    int set_config (std::string cfgstr)
    {
        return(zbar_decoder_parse_config(_decoder, cfgstr.c_str()));
    }

 private:
    friend class Scanner;
    zbar_decoder_t *_decoder;
    Handler *_handler;

    static void _cb (zbar_decoder_t *cdcode)
    {
        Decoder *dcode = (Decoder*)zbar_decoder_get_userdata(cdcode);
        if(dcode && dcode->_handler)
            dcode->_handler->decode_callback(*dcode);
    }
};

}

#endif
