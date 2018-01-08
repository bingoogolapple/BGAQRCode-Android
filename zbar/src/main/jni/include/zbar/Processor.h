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
#ifndef _ZBAR_PROCESSOR_H_
#define _ZBAR_PROCESSOR_H_

/// @file
/// Processor C++ wrapper

#ifndef _ZBAR_H_
# error "include zbar.h in your application, **not** zbar/Processor.h"
#endif

#include "Exception.h"
#include "Image.h"

namespace zbar {

/// high-level self-contained image processor.
/// processes video and images for barcodes, optionally displaying
/// images to a library owned output window

class Processor {
 public:
    /// value to pass for no timeout.
    static const int FOREVER = -1;

    /// constructor.
    Processor (bool threaded = true,
               const char *video_device = "",
               bool enable_display = true)
    {
        _processor = zbar_processor_create(threaded);
        if(!_processor)
            throw std::bad_alloc();
        init(video_device, enable_display);
    }

    ~Processor ()
    {
        zbar_processor_destroy(_processor);
    }

    /// cast to C processor object.
    operator zbar_processor_t* ()
    {
        return(_processor);
    }

    /// opens a video input device and/or prepares to display output.
    /// see zbar_processor_init()
    void init (const char *video_device = "",
               bool enable_display = true)
    {
        if(zbar_processor_init(_processor, video_device, enable_display))
            throw_exception(_processor);
    }

    /// setup result handler callback.
    /// see zbar_processor_set_data_handler()
    void set_handler (Image::Handler& handler)
    {
        zbar_processor_set_data_handler(_processor, handler, &handler);
    }

    /// set config for indicated symbology (0 for all) to specified value.
    /// @see zbar_processor_set_config()
    /// @since 0.4
    int set_config (zbar_symbol_type_t symbology,
                    zbar_config_t config,
                    int value)
    {
        return(zbar_processor_set_config(_processor, symbology,
                                          config, value));
    }

    /// set config parsed from configuration string.
    /// @see zbar_processor_parse_config()
    /// @since 0.4
    int set_config (std::string cfgstr)
    {
        return(zbar_processor_parse_config(_processor, cfgstr.c_str()));
    }

    /// retrieve the current state of the ouput window.
    /// see zbar_processor_is_visible()
    bool is_visible ()
    {
        int rc = zbar_processor_is_visible(_processor);
        if(rc < 0)
            throw_exception(_processor);
        return(rc != 0);
    }

    /// show or hide the display window owned by the library.
    /// see zbar_processor_set_visible()
    void set_visible (bool visible = true)
    {
        if(zbar_processor_set_visible(_processor, visible) < 0)
            throw_exception(_processor);
    }

    /// control the processor in free running video mode.
    /// see zbar_processor_set_active()
    void set_active (bool active = true)
    {
        if(zbar_processor_set_active(_processor, active) < 0)
            throw_exception(_processor);
    }

    /// retrieve decode results for last scanned image.
    /// @see zbar_processor_get_results()
    /// @since 0.10
    const SymbolSet get_results () const {
        return(SymbolSet(zbar_processor_get_results(_processor)));
    }

    /// wait for input to the display window from the user.
    /// see zbar_processor_user_wait()
    int user_wait (int timeout = FOREVER)
    {
        int rc = zbar_processor_user_wait(_processor, timeout);
        if(rc < 0)
            throw_exception(_processor);
        return(rc);
    }

    /// process from the video stream until a result is available.
    /// see zbar_process_one()
    void process_one (int timeout = FOREVER)
    {
        if(zbar_process_one(_processor, timeout) < 0)
            throw_exception(_processor);
    }

    /// process the provided image for barcodes.
    /// see zbar_process_image()
    void process_image (Image& image)
    {
        if(zbar_process_image(_processor, image) < 0)
            throw_exception(_processor);
    }

    /// process the provided image for barcodes.
    /// see zbar_process_image()
    Processor& operator<< (Image& image)
    {
        process_image(image);
        return(*this);
    }

    /// force specific input and output formats for debug/testing.
    /// see zbar_processor_force_format()
    void force_format (unsigned long input_format,
                       unsigned long output_format)
    {
        if(zbar_processor_force_format(_processor, input_format,
                                        output_format))
            throw_exception(_processor);
    }

    /// force specific input and output formats for debug/testing.
    /// see zbar_processor_force_format()
    void force_format (std::string& input_format,
                       std::string& output_format)
    {
        unsigned long ifourcc = zbar_fourcc_parse(input_format.c_str());
        unsigned long ofourcc = zbar_fourcc_parse(output_format.c_str());
        if(zbar_processor_force_format(_processor, ifourcc, ofourcc))
            throw_exception(_processor);
    }

    /// request a preferred size for the video image from the device.
    /// see zbar_processor_request_size()
    /// @since 0.6
    void request_size (int width, int height)
    {
        zbar_processor_request_size(_processor, width, height);
    }

    /// request a preferred driver interface version for debug/testing.
    /// see zbar_processor_request_interface()
    /// @since 0.6
    void request_interface (int version)
    {
        zbar_processor_request_interface(_processor, version);
    }

    /// request a preferred I/O mode for debug/testing.
    /// see zbar_processor_request_iomode()
    /// @since 0.7
    void request_iomode (int iomode)
    {
        if(zbar_processor_request_iomode(_processor, iomode))
            throw_exception(_processor);
    }

 private:
    zbar_processor_t *_processor;
};

}

#endif
