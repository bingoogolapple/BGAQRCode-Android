/*------------------------------------------------------------------------
 *  Copyright 2008-2009 (c) Jeff Brown <spadix@users.sourceforge.net>
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
#ifndef __ZBAR_GTK_H__
#define __ZBAR_GTK_H__

/** SECTION:ZBarGtk
 * @short_description: barcode reader GTK+ 2.x widget
 * @include: zbar/zbargtk.h
 *
 * embeds a barcode reader directly into a GTK+ based GUI.  the widget
 * can process barcodes from a video source (using the
 * #ZBarGtk:video-device and #ZBarGtk:video-enabled properties) or
 * from individual GdkPixbufs supplied to zbar_gtk_scan_image()
 *
 * Since: 1.5
 */

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtktable.h>

#include <zbar.h>

G_BEGIN_DECLS

#define ZBAR_TYPE_GTK (zbar_gtk_get_type())
#define ZBAR_GTK(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), ZBAR_TYPE_GTK, ZBarGtk))
#define ZBAR_GTK_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), ZBAR_TYPE_GTK, ZBarGtkClass))
#define ZBAR_IS_GTK(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), ZBAR_TYPE_GTK))
#define ZBAR_IS_GTK_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), ZBAR_TYPE_GTK))
#define ZBAR_GTK_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), ZBAR_TYPE_GTK, ZBarGtkClass))

typedef struct _ZBarGtk ZBarGtk;
typedef struct _ZBarGtkClass ZBarGtkClass;

struct _ZBarGtk {
    GtkWidget widget;
    gpointer *_private;

    /* properties */

    /**
     * ZBarGtk:video-device:
     *
     * the currently set video device.
     *
     * setting a new device opens it and automatically sets
     * #ZBarGtk:video-enabled.  set the empty string ("") or NULL to
     * close.
     */

    /**
     * ZBarGtk:video-enabled:
     *
     * video device streaming state.
     *
     * use to pause/resume video scanning.
     */

    /**
     * ZBarGtk:video-opened:
     *
     * video device opened state.
     *
     * (re)setting #ZBarGtk:video-device should eventually cause it
     * to be opened or closed.  any errors while streaming/scanning
     * will also cause the device to be closed
     */
};

struct _ZBarGtkClass {
    GtkWidgetClass parent_class;

    /* signals */

    /**
     * ZBarGtk::decoded:
     * @widget: the object that received the signal
     * @symbol_type: the type of symbol decoded (a zbar_symbol_type_t)
     * @data: the data decoded from the symbol
     * 
     * emitted when a barcode is decoded from an image.
     * the symbol type and contained data are provided as separate
     * parameters
     */
    void (*decoded) (ZBarGtk *zbar,
                     zbar_symbol_type_t symbol_type,
                     const char *data);

    /**
     * ZBarGtk::decoded-text:
     * @widget: the object that received the signal
     * @text: the decoded data prefixed by the string name of the
     *   symbol type (separated by a colon)
     *
     * emitted when a barcode is decoded from an image.
     * the symbol type name is prefixed to the data, separated by a
     * colon
     */
    void (*decoded_text) (ZBarGtk *zbar,
                          const char *text);

    /**
     * ZBarGtk::scan-image:
     * @widget: the object that received the signal
     * @image: the image to scan for barcodes
     */
    void (*scan_image) (ZBarGtk *zbar,
                        GdkPixbuf *image);
};

GType zbar_gtk_get_type(void) G_GNUC_CONST;

/**
 * zbar_gtk_new:
 * create a new barcode reader widget instance.
 * initially has no associated video device or image.
 *
 * Returns: a new #ZBarGtk widget instance
 */
GtkWidget *zbar_gtk_new(void);

/**
 * zbar_gtk_scan_image:
 * 
 */
void zbar_gtk_scan_image(ZBarGtk *zbar,
                         GdkPixbuf *image);

/** retrieve the currently opened video device.
 *
 * Returns: the current video device or NULL if no device is opened
 */
const char *zbar_gtk_get_video_device(ZBarGtk *zbar);

/** open a new video device.
 *
 * @video_device: the platform specific name of the device to open.
 *   use NULL to close a currently opened device.
 *
 * @note since opening a device may take some time, this call will
 * return immediately and the device will be opened asynchronously
 */
void zbar_gtk_set_video_device(ZBarGtk *zbar,
                               const char *video_device);

/** retrieve the current video enabled state.
 *
 * Returns: true if video scanning is currently enabled, false otherwise
 */
gboolean zbar_gtk_get_video_enabled(ZBarGtk *zbar);

/** enable/disable video scanning.
 * @video_enabled: true to enable video scanning, false to disable
 *
 * has no effect unless a video device is opened
 */
void zbar_gtk_set_video_enabled(ZBarGtk *zbar,
                                gboolean video_enabled);

/** retrieve the current video opened state.
 *
 * Returns: true if video device is currently opened, false otherwise
 */
gboolean zbar_gtk_get_video_opened(ZBarGtk *zbar);

/** set video camera resolution.
 * @width: width in pixels
 * @height: height in pixels
 *
 * @note this call must be made before video is initialized
 */
void zbar_gtk_request_video_size(ZBarGtk *zbar,
                                 int width,
                                 int height);

/**
 * utility function to populate a zbar_image_t from a GdkPixbuf.
 * @image: the zbar library image destination to populate
 * @pixbuf: the GdkPixbuf source
 *
 * Returns: TRUE if successful or FALSE if the conversion could not be
 * performed for some reason
 */
gboolean zbar_gtk_image_from_pixbuf(zbar_image_t *image,
                                    GdkPixbuf *pixbuf);

G_END_DECLS

#endif
