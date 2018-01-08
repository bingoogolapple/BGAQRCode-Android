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
#ifndef _ZBAR_WINDOW_H_
#define _ZBAR_WINDOW_H_

/// @file
/// Output Window C++ wrapper

#ifndef _ZBAR_H_
# error "include zbar.h in your application, **not** zbar/Window.h"
#endif

#include "Image.h"

namespace zbar {

/// mid-level output window abstraction.
/// displays images to user-specified platform specific output window

class Window {
public:
    /// constructor.
    Window (zbar_window_t *window = NULL)
    {
        if(window)
            _window = window;
        else
            _window = zbar_window_create();
    }

    /// constructor.
    Window (void *x11_display_w32_hwnd,
            unsigned long x11_drawable)
    {
        _window = zbar_window_create();
        attach(x11_display_w32_hwnd, x11_drawable);
    }

    ~Window ()
    {
        zbar_window_destroy(_window);
    }

    /// cast to C window object.
    operator zbar_window_t* () const
    {
        return(_window);
    }

    /// associate reader with an existing platform window.
    /// see zbar_window_attach()
    void attach (void *x11_display_w32_hwnd,
                 unsigned long x11_drawable = 0)
    {
        if(zbar_window_attach(_window,
                               x11_display_w32_hwnd, x11_drawable) < 0)
            throw_exception(_window);
    }

    /// control content level of the reader overlay.
    /// see zbar_window_set_overlay()
    void set_overlay (int level)
    {
        zbar_window_set_overlay(_window, level);
    }

    /// retrieve current content level of reader overlay.
    /// see zbar_window_get_overlay()

    /// draw a new image into the output window.
    /// see zbar_window_draw()
    void draw (Image& image)
    {
        if(zbar_window_draw(_window, image) < 0)
            throw_exception(_window);
    }

    /// clear the image from the output window.
    /// see zbar_window_draw()
    void clear ()
    {
        if(zbar_window_draw(_window, NULL) < 0)
            throw_exception(_window);
    }

    /// redraw the last image.
    /// zbar_window_redraw()
    void redraw ()
    {
        if(zbar_window_redraw(_window) < 0)
            throw_exception(_window);
    }

    /// resize the image window.
    /// zbar_window_resize()
    void resize (unsigned width, unsigned height)
    {
        if(zbar_window_resize(_window, width, height) < 0)
            throw_exception(_window);
    }

private:
    zbar_window_t *_window;
};

/// select a compatible format between video input and output window.
/// see zbar_negotiate_format()
static inline void negotiate_format (Video& video, Window& window)
{
    if(zbar_negotiate_format(video, window) < 0)
        throw_exception(video);
}

}

#endif
