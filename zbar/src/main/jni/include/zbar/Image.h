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
#ifndef _ZBAR_IMAGE_H_
#define _ZBAR_IMAGE_H_

/// @file
/// Image C++ wrapper

#ifndef _ZBAR_H_
# error "include zbar.h in your application, **not** zbar/Image.h"
#endif

#include <assert.h>
#include <iterator>
#include "Symbol.h"
#include "Exception.h"

namespace zbar {

class Video;

/// stores image data samples along with associated format and size
/// metadata

class Image {
public:

    /// general Image result handler.
    /// applications should subtype this and pass an instance to
    /// eg. ImageScanner::set_handler() to implement result processing
    class Handler {
    public:
        virtual ~Handler() { }

        /// invoked by library when Image should be processed
        virtual void image_callback(Image &image) = 0;

        /// cast this handler to the C handler
        operator zbar_image_data_handler_t* () const
        {
            return(_cb);
        }

    private:
        static void _cb (zbar_image_t *zimg,
                         const void *userdata)
        {
            if(userdata) {
                Image *image = (Image*)zbar_image_get_userdata(zimg);
                if(image)
                    ((Handler*)userdata)->image_callback(*image);
                else {
                    Image tmp(zimg, 1);
                    ((Handler*)userdata)->image_callback(tmp);
                }
            }
        }
    };

    class SymbolIterator : public zbar::SymbolIterator {
    public:
        /// default constructor.
        SymbolIterator ()
            : zbar::SymbolIterator()
        { }

        /// constructor.
        SymbolIterator (const SymbolSet &syms)
            : zbar::SymbolIterator(syms)
        { }

        /// copy constructor.
        SymbolIterator (const SymbolIterator& iter)
            : zbar::SymbolIterator(iter)
        { }
    };

    /// constructor.
    /// create a new Image with the specified parameters
    Image (unsigned width = 0,
           unsigned height = 0,
           const std::string& format = "",
           const void *data = NULL,
           unsigned long length = 0)
        : _img(zbar_image_create())
    {
        zbar_image_set_userdata(_img, this);
        if(width && height)
            set_size(width, height);
        if(format.length())
            set_format(format);
        if(data && length)
            set_data(data, length);
    }

    ~Image ()
    {
        if(zbar_image_get_userdata(_img) == this)
            zbar_image_set_userdata(_img, NULL);
        zbar_image_ref(_img, -1);
    }

    /// cast to C image object
    operator const zbar_image_t* () const
    {
        return(_img);
    }

    /// cast to C image object
    operator zbar_image_t* ()
    {
        return(_img);
    }

    /// retrieve the image format.
    /// see zbar_image_get_format()
    unsigned long get_format () const
    {
        return(zbar_image_get_format(_img));
    }

    /// specify the fourcc image format code for image sample data.
    /// see zbar_image_set_format()
    void set_format (unsigned long format)
    {
        zbar_image_set_format(_img, format);
    }

    /// specify the fourcc image format code for image sample data.
    /// see zbar_image_set_format()
    void set_format (const std::string& format)
    {
        unsigned long fourcc = zbar_fourcc_parse(format.c_str());
        zbar_image_set_format(_img, fourcc);
    }

    /// retrieve a "sequence" (page/frame) number associated with this
    /// image.
    /// see zbar_image_get_sequence()
    /// @since 0.6
    unsigned get_sequence () const
    {
        return(zbar_image_get_sequence(_img));
    }

    /// associate a "sequence" (page/frame) number with this image.
    /// see zbar_image_set_sequence()
    /// @since 0.6
    void set_sequence (unsigned sequence_num)
    {
        zbar_image_set_sequence(_img, sequence_num);
    }

    /// retrieve the width of the image.
    /// see zbar_image_get_width()
    unsigned get_width () const
    {
        return(zbar_image_get_width(_img));
    }

    /// retrieve the height of the image.
    /// see zbar_image_get_height()
    unsigned get_height () const
    {
        return(zbar_image_get_height(_img));
    }

    /// retrieve both dimensions of the image.
    /// see zbar_image_get_size()
    /// @since 0.11
    void get_size (unsigned &width,
                   unsigned &height) const
    {
        zbar_image_get_size(_img, &width, &height);
    }

    /// specify the pixel size of the image.
    /// see zbar_image_set_size()
    void set_size (unsigned width,
                   unsigned height)
    {
        zbar_image_set_size(_img, width, height);
    }

    /// retrieve the scan crop rectangle.
    /// see zbar_image_get_crop()
    void get_crop (unsigned &x,
                   unsigned &y,
                   unsigned &width,
                   unsigned &height) const
    {
        zbar_image_get_crop(_img, &x, &y, &width, &height);
    }

    /// set the scan crop rectangle.
    /// see zbar_image_set_crop()
    void set_crop (unsigned x,
                   unsigned y,
                   unsigned width,
                   unsigned height)
    {
        zbar_image_set_crop(_img, x, y, width, height);
    }

    /// return the image sample data.
    /// see zbar_image_get_data()
    const void *get_data () const
    {
        return(zbar_image_get_data(_img));
    }

    /// return the size of the image sample data.
    /// see zbar_image_get_data_length()
    /// @since 0.6
    unsigned long get_data_length () const
    {
        return(zbar_image_get_data_length(_img));
    }

    /// specify image sample data.
    /// see zbar_image_set_data()
    void set_data (const void *data,
                   unsigned long length)
    {
        zbar_image_set_data(_img, data, length, _cleanup);
    }

    /// image format conversion.
    /// see zbar_image_convert()
    Image convert (unsigned long format) const
    {
        zbar_image_t *img = zbar_image_convert(_img, format);
        if(img)
            return(Image(img));
        throw FormatError();
    }

    /// image format conversion.
    /// see zbar_image_convert()
    /// @since 0.11
    Image convert (std::string format) const
    {
        unsigned long fourcc = zbar_fourcc_parse(format.c_str());
        return(convert(fourcc));
    }

    /// image format conversion with crop/pad.
    /// see zbar_image_convert_resize()
    /// @since 0.4
    Image convert (unsigned long format,
                   unsigned width,
                   unsigned height) const
    {
        zbar_image_t *img =
            zbar_image_convert_resize(_img, format, width, height);
        if(img)
            return(Image(img));
        throw FormatError();
    }

    const SymbolSet get_symbols () const {
        return(SymbolSet(zbar_image_get_symbols(_img)));
    }

    void set_symbols (const SymbolSet &syms) {
        zbar_image_set_symbols(_img, syms);
    }

    /// create a new SymbolIterator over decoded results.
    SymbolIterator symbol_begin () const {
        return(SymbolIterator(get_symbols()));
    }

    /// return a SymbolIterator suitable for ending iteration.
    SymbolIterator symbol_end () const {
        return(SymbolIterator());
    }

protected:

    friend class Video;

    /// constructor.
    /// @internal
    /// create a new Image from a zbar_image_t C object
    Image (zbar_image_t *src,
           int refs = 0)
        : _img(src)
    {
        if(refs)
            zbar_image_ref(_img, refs);
        zbar_image_set_userdata(_img, this);
    }

    /// default data cleanup (noop)
    /// @internal
    static void _cleanup (zbar_image_t *img)
    {
        // by default nothing is cleaned
        assert(img);
    }

private:
    zbar_image_t *_img;
};

}

#endif
