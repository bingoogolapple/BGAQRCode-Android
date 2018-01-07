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
#ifndef _ZBAR_VIDEO_H_
#define _ZBAR_VIDEO_H_

/// @file
/// Video Input C++ wrapper

#ifndef _ZBAR_H_
# error "include zbar.h in your application, **not** zbar/Video.h"
#endif

#include "Image.h"

namespace zbar {

/// mid-level video source abstraction.
/// captures images from a video device

class Video {
public:
    /// constructor.
    Video (zbar_video_t *video = NULL)
    {
        if(video)
            _video = video;
        else
            _video = zbar_video_create();
    }

    /// constructor.
    Video (std::string& device)
    {
        _video = zbar_video_create();
        open(device);
    }

    ~Video ()
    {
        zbar_video_destroy(_video);
    }

    /// cast to C video object.
    operator zbar_video_t* () const
    {
        return(_video);
    }

    /// open and probe a video device.
    void open (std::string& device)
    {
        if(zbar_video_open(_video, device.c_str()))
            throw_exception(_video);
    }

    /// close video device if open.
    void close ()
    {
        if(zbar_video_open(_video, NULL))
            throw_exception(_video);
    }

    /// initialize video using a specific format for debug.
    /// see zbar_video_init()
    void init (unsigned long fourcc)
    {
        if(zbar_video_init(_video, fourcc))
            throw_exception(_video);
    }

    /// initialize video using a specific format for debug.
    /// see zbar_video_init()
    void init (std::string& format)
    {
        unsigned int fourcc = zbar_fourcc_parse(format.c_str());
        if(zbar_video_init(_video, fourcc))
            throw_exception(_video);
    }

    /// retrieve file descriptor associated with open *nix video device.
    /// see zbar_video_get_fd()
    int get_fd ()
    {
        return(zbar_video_get_fd(_video));
    }

    /// retrieve current output image width.
    /// see zbar_video_get_width()
    int get_width ()
    {
        return(zbar_video_get_width(_video));
    }

    /// retrieve current output image height.
    /// see zbar_video_get_height()
    int get_height ()
    {
        return(zbar_video_get_height(_video));
    }

    /// start/stop video capture.
    /// see zbar_video_enable()
    void enable (bool enable = true)
    {
        if(zbar_video_enable(_video, enable))
            throw_exception(_video);
    }

    /// retrieve next captured image.
    /// see zbar_video_next_image()
    Image next_image ()
    {
        zbar_image_t *img = zbar_video_next_image(_video);
        if(!img)
            throw_exception(_video);
        return(Image(img));
    }

    /// request a preferred size for the video image from the device.
    /// see zbar_video_request_size()
    /// @since 0.6
    void request_size (int width, int height)
    {
        zbar_video_request_size(_video, width, height);
    }

    /// request a preferred driver interface version for debug/testing.
    /// see zbar_video_request_interface()
    /// @since 0.6
    void request_interface (int version)
    {
        zbar_video_request_interface(_video, version);
    }

    /// request a preferred I/O mode for debug/testing.
    /// see zbar_video_request_iomode()
    /// @since 0.7
    void request_iomode (int iomode)
    {
        if(zbar_video_request_iomode(_video, iomode))
            throw_exception(_video);
    }

private:
    zbar_video_t *_video;
};

}

#endif
