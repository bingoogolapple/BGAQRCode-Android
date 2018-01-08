//------------------------------------------------------------------------
//  Copyright 2008-2009 (c) Jeff Brown <spadix@users.sourceforge.net>
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
#ifndef _QZBAR_H_
#define _QZBAR_H_

/// @file
/// Barcode Reader Qt4 Widget

#include <qwidget.h>

namespace zbar {

class QZBarThread;

/// barcode reader Qt4 widget.
/// embeds a barcode reader directly into a Qt4 based GUI.  the widget
/// can process barcodes from a video source (using the QZBar::videoDevice
/// and QZBar::videoEnabled properties) or from individual QImages
/// supplied to the QZBar::scanImage() slot
/// @since 1.5

class QZBar : public QWidget
{
    Q_OBJECT

    /// the currently opened video device.
    ///
    /// setting a new device opens it and automatically sets
    /// QZBar::videoEnabled
    ///
    /// @see videoDevice(), setVideoDevice()
    Q_PROPERTY(QString videoDevice
               READ videoDevice
               WRITE setVideoDevice
               DESIGNABLE false)

    /// video device streaming state.
    ///
    /// use to pause/resume video scanning.
    ///
    /// @see isVideoEnabled(), setVideoEnabled()
    Q_PROPERTY(bool videoEnabled
               READ isVideoEnabled
               WRITE setVideoEnabled
               DESIGNABLE false)

    /// video device opened state.
    ///
    /// (re)setting QZBar::videoDevice should eventually cause it
    /// to be opened or closed.  any errors while streaming/scanning
    /// will also cause the device to be closed
    ///
    /// @see isVideoOpened()
    Q_PROPERTY(bool videoOpened
               READ isVideoOpened
               DESIGNABLE false)

public:

    /// constructs a barcode reader widget with the given @a parent
    QZBar(QWidget *parent = NULL);

    ~QZBar();

    /// retrieve the currently opened video device.
    /// @returns the current video device or the empty string if no
    /// device is opened
    QString videoDevice() const;

    /// retrieve the current video enabled state.
    /// @returns true if video scanning is currently enabled, false
    /// otherwise
    bool isVideoEnabled() const;

    /// retrieve the current video opened state.
    /// @returns true if video device is currently opened, false otherwise
    bool isVideoOpened() const;

    /// @{
    /// @internal

    QSize sizeHint() const;
    int heightForWidth(int) const;
    QPaintEngine *paintEngine() const;

    /// @}

public Q_SLOTS:

    /// open a new video device.
    ///
    /// use an empty string to close a currently opened device.
    ///
    /// @note since opening a device may take some time, this call will
    /// return immediately and the device will be opened asynchronously
    void setVideoDevice(const QString &videoDevice);

    /// enable/disable video scanning.
    /// has no effect unless a video device is opened
    void setVideoEnabled(bool videoEnabled = true);

    /// scan for barcodes in a QImage.
    void scanImage(const QImage &image);

Q_SIGNALS:
    /// emitted when when a video device is opened or closed.
    ///
    /// (re)setting QZBar::videoDevice should eventually cause it
    /// to be opened or closed.  any errors while streaming/scanning
    /// will also cause the device to be closed
    void videoOpened(bool videoOpened);

    /// emitted when a barcode is decoded from an image.
    /// the symbol type and contained data are provided as separate
    /// parameters.
    void decoded(int type, const QString &data);

    /// emitted when a barcode is decoded from an image.
    /// the symbol type name is prefixed to the data, separated by a
    /// colon
    void decodedText(const QString &text);

    /// @{
    /// @internal

protected:
    void attach();
    void showEvent(QShowEvent*);
    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent*);
    void changeEvent(QEvent*);
    void dragEnterEvent(QDragEnterEvent*);
    void dropEvent(QDropEvent*);

protected Q_SLOTS:
    void sizeChange();

    /// @}

private:
    QZBarThread *thread;
    QString _videoDevice;
    bool _videoEnabled;
    bool _attached;
};

};

#endif
