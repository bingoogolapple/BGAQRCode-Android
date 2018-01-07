//------------------------------------------------------------------------
//  Copyright 2008-2010 (c) Jeff Brown <spadix@users.sourceforge.net>
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
#ifndef _QZBARIMAGE_H_
#define _QZBARIMAGE_H_

/// @file
/// QImage to Image type conversion wrapper

#include <qimage.h>
#include <zbar.h>

namespace zbar {

/// wrap a QImage and convert into a format suitable for scanning.

class QZBarImage
    : public Image
{
public:

    /// construct a zbar library image based on an existing QImage.

    QZBarImage (const QImage &qimg)
        : qimg(qimg)
    {
        QImage::Format fmt = qimg.format();
        if(fmt != QImage::Format_RGB32 &&
           fmt != QImage::Format_ARGB32 &&
           fmt != QImage::Format_ARGB32_Premultiplied)
            throw FormatError();

        unsigned bpl = qimg.bytesPerLine();
        unsigned width = bpl / 4;
        unsigned height = qimg.height();
        set_size(width, height);
        set_format(zbar_fourcc('B','G','R','4'));
        unsigned long datalen = qimg.numBytes();
        set_data(qimg.bits(), datalen);

        if((width * 4 != bpl) ||
           (width * height * 4 > datalen))
            throw FormatError();
    }

private:
    QImage qimg;
};

};


#endif
