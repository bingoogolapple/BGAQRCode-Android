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
#ifndef _ZBAR_H_
#define _ZBAR_H_

/** @file
 * ZBar Barcode Reader C API definition
 */

/** @mainpage
 *
 * interface to the barcode reader is available at several levels.
 * most applications will want to use the high-level interfaces:
 *
 * @section high-level High-Level Interfaces
 *
 * these interfaces wrap all library functionality into an easy-to-use
 * package for a specific toolkit:
 * - the "GTK+ 2.x widget" may be used with GTK GUI applications.  a
 *   Python wrapper is included for PyGtk
 * - the @ref zbar::QZBar "Qt4 widget" may be used with Qt GUI
 *   applications
 * - the Processor interface (in @ref c-processor "C" or @ref
 *   zbar::Processor "C++") adds a scanning window to an application
 *   with no GUI.
 *
 * @section mid-level Intermediate Interfaces
 *
 * building blocks used to construct high-level interfaces:
 * - the ImageScanner (in @ref c-imagescanner "C" or @ref
 *   zbar::ImageScanner "C++") looks for barcodes in a library defined
 *   image object
 * - the Window abstraction (in @ref c-window "C" or @ref
 *   zbar::Window "C++") sinks library images, displaying them on the
 *   platform display
 * - the Video abstraction (in @ref c-video "C" or @ref zbar::Video
 *   "C++") sources library images from a video device
 *
 * @section low-level Low-Level Interfaces
 *
 * direct interaction with barcode scanning and decoding:
 * - the Scanner (in @ref c-scanner "C" or @ref zbar::Scanner "C++")
 *   looks for barcodes in a linear intensity sample stream
 * - the Decoder (in @ref c-decoder "C" or @ref zbar::Decoder "C++")
 *   extracts barcodes from a stream of bar and space widths
 */

#ifdef __cplusplus

/** C++ namespace for library interfaces */
namespace zbar {
    extern "C" {
#endif


/** @name Global library interfaces */
/*@{*/

/** "color" of element: bar or space. */
typedef enum zbar_color_e {
    ZBAR_SPACE = 0,    /**< light area or space between bars */
    ZBAR_BAR = 1,      /**< dark area or colored bar segment */
} zbar_color_t;

/** decoded symbol type. */
typedef enum zbar_symbol_type_e {
    ZBAR_NONE        =      0,  /**< no symbol decoded */
    ZBAR_PARTIAL     =      1,  /**< intermediate status */
    ZBAR_EAN2        =      2,  /**< GS1 2-digit add-on */
    ZBAR_EAN5        =      5,  /**< GS1 5-digit add-on */
    ZBAR_EAN8        =      8,  /**< EAN-8 */
    ZBAR_UPCE        =      9,  /**< UPC-E */
    ZBAR_ISBN10      =     10,  /**< ISBN-10 (from EAN-13). @since 0.4 */
    ZBAR_UPCA        =     12,  /**< UPC-A */
    ZBAR_EAN13       =     13,  /**< EAN-13 */
    ZBAR_ISBN13      =     14,  /**< ISBN-13 (from EAN-13). @since 0.4 */
    ZBAR_COMPOSITE   =     15,  /**< EAN/UPC composite */
    ZBAR_I25         =     25,  /**< Interleaved 2 of 5. @since 0.4 */
    ZBAR_DATABAR     =     34,  /**< GS1 DataBar (RSS). @since 0.11 */
    ZBAR_DATABAR_EXP =     35,  /**< GS1 DataBar Expanded. @since 0.11 */
    ZBAR_CODABAR     =     38,  /**< Codabar. @since 0.11 */
    ZBAR_CODE39      =     39,  /**< Code 39. @since 0.4 */
    ZBAR_PDF417      =     57,  /**< PDF417. @since 0.6 */
    ZBAR_QRCODE      =     64,  /**< QR Code. @since 0.10 */
    ZBAR_CODE93      =     93,  /**< Code 93. @since 0.11 */
    ZBAR_CODE128     =    128,  /**< Code 128 */

    /** mask for base symbol type.
     * @deprecated in 0.11, remove this from existing code
     */
    ZBAR_SYMBOL      = 0x00ff,
    /** 2-digit add-on flag.
     * @deprecated in 0.11, a ::ZBAR_EAN2 component is used for
     * 2-digit GS1 add-ons
     */
    ZBAR_ADDON2      = 0x0200,
    /** 5-digit add-on flag.
     * @deprecated in 0.11, a ::ZBAR_EAN5 component is used for
     * 5-digit GS1 add-ons
     */
    ZBAR_ADDON5      = 0x0500,
    /** add-on flag mask.
     * @deprecated in 0.11, GS1 add-ons are represented using composite
     * symbols of type ::ZBAR_COMPOSITE; add-on components use ::ZBAR_EAN2
     * or ::ZBAR_EAN5
     */
    ZBAR_ADDON       = 0x0700,
} zbar_symbol_type_t;

/** decoded symbol coarse orientation.
 * @since 0.11
 */
typedef enum zbar_orientation_e {
    ZBAR_ORIENT_UNKNOWN = -1,   /**< unable to determine orientation */
    ZBAR_ORIENT_UP,             /**< upright, read left to right */
    ZBAR_ORIENT_RIGHT,          /**< sideways, read top to bottom */
    ZBAR_ORIENT_DOWN,           /**< upside-down, read right to left */
    ZBAR_ORIENT_LEFT,           /**< sideways, read bottom to top */
} zbar_orientation_t;

/** error codes. */
typedef enum zbar_error_e {
    ZBAR_OK = 0,                /**< no error */
    ZBAR_ERR_NOMEM,             /**< out of memory */
    ZBAR_ERR_INTERNAL,          /**< internal library error */
    ZBAR_ERR_UNSUPPORTED,       /**< unsupported request */
    ZBAR_ERR_INVALID,           /**< invalid request */
    ZBAR_ERR_SYSTEM,            /**< system error */
    ZBAR_ERR_LOCKING,           /**< locking error */
    ZBAR_ERR_BUSY,              /**< all resources busy */
    ZBAR_ERR_XDISPLAY,          /**< X11 display error */
    ZBAR_ERR_XPROTO,            /**< X11 protocol error */
    ZBAR_ERR_CLOSED,            /**< output window is closed */
    ZBAR_ERR_WINAPI,            /**< windows system error */
    ZBAR_ERR_NUM                /**< number of error codes */
} zbar_error_t;

/** decoder configuration options.
 * @since 0.4
 */
typedef enum zbar_config_e {
    ZBAR_CFG_ENABLE = 0,        /**< enable symbology/feature */
    ZBAR_CFG_ADD_CHECK,         /**< enable check digit when optional */
    ZBAR_CFG_EMIT_CHECK,        /**< return check digit when present */
    ZBAR_CFG_ASCII,             /**< enable full ASCII character set */
    ZBAR_CFG_NUM,               /**< number of boolean decoder configs */

    ZBAR_CFG_MIN_LEN = 0x20,    /**< minimum data length for valid decode */
    ZBAR_CFG_MAX_LEN,           /**< maximum data length for valid decode */

    ZBAR_CFG_UNCERTAINTY = 0x40,/**< required video consistency frames */

    ZBAR_CFG_POSITION = 0x80,   /**< enable scanner to collect position data */

    ZBAR_CFG_X_DENSITY = 0x100, /**< image scanner vertical scan density */
    ZBAR_CFG_Y_DENSITY,         /**< image scanner horizontal scan density */
} zbar_config_t;

/** decoder symbology modifier flags.
 * @since 0.11
 */
typedef enum zbar_modifier_e {
    /** barcode tagged as GS1 (EAN.UCC) reserved
     * (eg, FNC1 before first data character).
     * data may be parsed as a sequence of GS1 AIs
     */
    ZBAR_MOD_GS1 = 0,

    /** barcode tagged as AIM reserved
     * (eg, FNC1 after first character or digit pair)
     */
    ZBAR_MOD_AIM,

    /** number of modifiers */
    ZBAR_MOD_NUM,
} zbar_modifier_t;

/** retrieve runtime library version information.
 * @param major set to the running major version (unless NULL)
 * @param minor set to the running minor version (unless NULL)
 * @returns 0
 */
extern int zbar_version(unsigned *major,
                        unsigned *minor);

/** set global library debug level.
 * @param verbosity desired debug level.  higher values create more spew
 */
extern void zbar_set_verbosity(int verbosity);

/** increase global library debug level.
 * eg, for -vvvv
 */
extern void zbar_increase_verbosity(void);

/** retrieve string name for symbol encoding.
 * @param sym symbol type encoding
 * @returns the static string name for the specified symbol type,
 * or "UNKNOWN" if the encoding is not recognized
 */
extern const char *zbar_get_symbol_name(zbar_symbol_type_t sym);

/** retrieve string name for addon encoding.
 * @param sym symbol type encoding
 * @returns static string name for any addon, or the empty string
 * if no addons were decoded
 * @deprecated in 0.11
 */
extern const char *zbar_get_addon_name(zbar_symbol_type_t sym);

/** retrieve string name for configuration setting.
 * @param config setting to name
 * @returns static string name for config,
 * or the empty string if value is not a known config
 */
extern const char *zbar_get_config_name(zbar_config_t config);

/** retrieve string name for modifier.
 * @param modifier flag to name
 * @returns static string name for modifier,
 * or the empty string if the value is not a known flag
 */
extern const char *zbar_get_modifier_name(zbar_modifier_t modifier);

/** retrieve string name for orientation.
 * @param orientation orientation encoding
 * @returns the static string name for the specified orientation,
 * or "UNKNOWN" if the orientation is not recognized
 * @since 0.11
 */
extern const char *zbar_get_orientation_name(zbar_orientation_t orientation);

/** parse a configuration string of the form "[symbology.]config[=value]".
 * the config must match one of the recognized names.
 * the symbology, if present, must match one of the recognized names.
 * if symbology is unspecified, it will be set to 0.
 * if value is unspecified it will be set to 1.
 * @returns 0 if the config is parsed successfully, 1 otherwise
 * @since 0.4
 */
extern int zbar_parse_config(const char *config_string,
                             zbar_symbol_type_t *symbology,
                             zbar_config_t *config,
                             int *value);

/** consistently compute fourcc values across architectures
 * (adapted from v4l2 specification)
 * @since 0.11
 */
#define zbar_fourcc(a, b, c, d)                 \
        ((unsigned long)(a) |                   \
         ((unsigned long)(b) << 8) |            \
         ((unsigned long)(c) << 16) |           \
         ((unsigned long)(d) << 24))

/** parse a fourcc string into its encoded integer value.
 * @since 0.11
 */
static inline unsigned long zbar_fourcc_parse (const char *format)
{
    unsigned long fourcc = 0;
    if(format) {
        int i;
        for(i = 0; i < 4 && format[i]; i++)
            fourcc |= ((unsigned long)format[i]) << (i * 8);
    }
    return(fourcc);
}

/** @internal type unsafe error API (don't use) */
extern int _zbar_error_spew(const void *object,
                            int verbosity);
extern const char *_zbar_error_string(const void *object,
                                      int verbosity);
extern zbar_error_t _zbar_get_error_code(const void *object);

/*@}*/

struct zbar_symbol_s;
typedef struct zbar_symbol_s zbar_symbol_t;

struct zbar_symbol_set_s;
typedef struct zbar_symbol_set_s zbar_symbol_set_t;


/*------------------------------------------------------------*/
/** @name Symbol interface
 * decoded barcode symbol result object.  stores type, data, and image
 * location of decoded symbol.  all memory is owned by the library
 */
/*@{*/

/** @typedef zbar_symbol_t
 * opaque decoded symbol object.
 */

/** symbol reference count manipulation.
 * increment the reference count when you store a new reference to the
 * symbol.  decrement when the reference is no longer used.  do not
 * refer to the symbol once the count is decremented and the
 * containing image has been recycled or destroyed.
 * @note the containing image holds a reference to the symbol, so you
 * only need to use this if you keep a symbol after the image has been
 * destroyed or reused.
 * @since 0.9
 */
extern void zbar_symbol_ref(const zbar_symbol_t *symbol,
                            int refs);

/** retrieve type of decoded symbol.
 * @returns the symbol type
 */
extern zbar_symbol_type_t zbar_symbol_get_type(const zbar_symbol_t *symbol);

/** retrieve symbology boolean config settings.
 * @returns a bitmask indicating which configs were set for the detected
 * symbology during decoding.
 * @since 0.11
 */
extern unsigned int zbar_symbol_get_configs(const zbar_symbol_t *symbol);

/** retrieve symbology modifier flag settings.
 * @returns a bitmask indicating which characteristics were detected
 * during decoding.
 * @since 0.11
 */
extern unsigned int zbar_symbol_get_modifiers(const zbar_symbol_t *symbol);

/** retrieve data decoded from symbol.
 * @returns the data string
 */
extern const char *zbar_symbol_get_data(const zbar_symbol_t *symbol);

/** retrieve length of binary data.
 * @returns the length of the decoded data
 */
extern unsigned int zbar_symbol_get_data_length(const zbar_symbol_t *symbol);

/** retrieve a symbol confidence metric.
 * @returns an unscaled, relative quantity: larger values are better
 * than smaller values, where "large" and "small" are application
 * dependent.
 * @note expect the exact definition of this quantity to change as the
 * metric is refined.  currently, only the ordered relationship
 * between two values is defined and will remain stable in the future
 * @since 0.9
 */
extern int zbar_symbol_get_quality(const zbar_symbol_t *symbol);

/** retrieve current cache count.  when the cache is enabled for the
 * image_scanner this provides inter-frame reliability and redundancy
 * information for video streams.
 * @returns < 0 if symbol is still uncertain.
 * @returns 0 if symbol is newly verified.
 * @returns > 0 for duplicate symbols
 */
extern int zbar_symbol_get_count(const zbar_symbol_t *symbol);

/** retrieve the number of points in the location polygon.  the
 * location polygon defines the image area that the symbol was
 * extracted from.
 * @returns the number of points in the location polygon
 * @note this is currently not a polygon, but the scan locations
 * where the symbol was decoded
 */
extern unsigned zbar_symbol_get_loc_size(const zbar_symbol_t *symbol);

/** retrieve location polygon x-coordinates.
 * points are specified by 0-based index.
 * @returns the x-coordinate for a point in the location polygon.
 * @returns -1 if index is out of range
 */
extern int zbar_symbol_get_loc_x(const zbar_symbol_t *symbol,
                                 unsigned index);

/** retrieve location polygon y-coordinates.
 * points are specified by 0-based index.
 * @returns the y-coordinate for a point in the location polygon.
 * @returns -1 if index is out of range
 */
extern int zbar_symbol_get_loc_y(const zbar_symbol_t *symbol,
                                 unsigned index);

/** retrieve general orientation of decoded symbol.
 * @returns a coarse, axis-aligned indication of symbol orientation or
 * ::ZBAR_ORIENT_UNKNOWN if unknown
 * @since 0.11
 */
extern zbar_orientation_t
zbar_symbol_get_orientation(const zbar_symbol_t *symbol);

/** iterate the set to which this symbol belongs (there can be only one).
 * @returns the next symbol in the set, or
 * @returns NULL when no more results are available
 */
extern const zbar_symbol_t *zbar_symbol_next(const zbar_symbol_t *symbol);

/** retrieve components of a composite result.
 * @returns the symbol set containing the components
 * @returns NULL if the symbol is already a physical symbol
 * @since 0.10
 */
extern const zbar_symbol_set_t*
zbar_symbol_get_components(const zbar_symbol_t *symbol);

/** iterate components of a composite result.
 * @returns the first physical component symbol of a composite result
 * @returns NULL if the symbol is already a physical symbol
 * @since 0.10
 */
extern const zbar_symbol_t*
zbar_symbol_first_component(const zbar_symbol_t *symbol);

/** print XML symbol element representation to user result buffer.
 * @see http://zbar.sourceforge.net/2008/barcode.xsd for the schema.
 * @param symbol is the symbol to print
 * @param buffer is the inout result pointer, it will be reallocated
 * with a larger size if necessary.
 * @param buflen is inout length of the result buffer.
 * @returns the buffer pointer
 * @since 0.6
 */
extern char *zbar_symbol_xml(const zbar_symbol_t *symbol,
                             char **buffer,
                             unsigned *buflen);

/*@}*/

/*------------------------------------------------------------*/
/** @name Symbol Set interface
 * container for decoded result symbols associated with an image
 * or a composite symbol.
 * @since 0.10
 */
/*@{*/

/** @typedef zbar_symbol_set_t
 * opaque symbol iterator object.
 * @since 0.10
 */

/** reference count manipulation.
 * increment the reference count when you store a new reference.
 * decrement when the reference is no longer used.  do not refer to
 * the object any longer once references have been released.
 * @since 0.10
 */
extern void zbar_symbol_set_ref(const zbar_symbol_set_t *symbols,
                                int refs);

/** retrieve set size.
 * @returns the number of symbols in the set.
 * @since 0.10
 */
extern int zbar_symbol_set_get_size(const zbar_symbol_set_t *symbols);

/** set iterator.
 * @returns the first decoded symbol result in a set
 * @returns NULL if the set is empty
 * @since 0.10
 */
extern const zbar_symbol_t*
zbar_symbol_set_first_symbol(const zbar_symbol_set_t *symbols);

/** raw result iterator.
 * @returns the first decoded symbol result in a set, *before* filtering
 * @returns NULL if the set is empty
 * @since 0.11
 */
extern const zbar_symbol_t*
zbar_symbol_set_first_unfiltered(const zbar_symbol_set_t *symbols);

/*@}*/

/*------------------------------------------------------------*/
/** @name Image interface
 * stores image data samples along with associated format and size
 * metadata
 */
/*@{*/

struct zbar_image_s;
/** opaque image object. */
typedef struct zbar_image_s zbar_image_t;

/** cleanup handler callback function.
 * called to free sample data when an image is destroyed.
 */
typedef void (zbar_image_cleanup_handler_t)(zbar_image_t *image);

/** data handler callback function.
 * called when decoded symbol results are available for an image
 */
typedef void (zbar_image_data_handler_t)(zbar_image_t *image,
                                         const void *userdata);

/** new image constructor.
 * @returns a new image object with uninitialized data and format.
 * this image should be destroyed (using zbar_image_destroy()) as
 * soon as the application is finished with it
 */
extern zbar_image_t *zbar_image_create(void);

/** image destructor.  all images created by or returned to the
 * application should be destroyed using this function.  when an image
 * is destroyed, the associated data cleanup handler will be invoked
 * if available
 * @note make no assumptions about the image or the data buffer.
 * they may not be destroyed/cleaned immediately if the library
 * is still using them.  if necessary, use the cleanup handler hook
 * to keep track of image data buffers
 */
extern void zbar_image_destroy(zbar_image_t *image);

/** image reference count manipulation.
 * increment the reference count when you store a new reference to the
 * image.  decrement when the reference is no longer used.  do not
 * refer to the image any longer once the count is decremented.
 * zbar_image_ref(image, -1) is the same as zbar_image_destroy(image)
 * @since 0.5
 */
extern void zbar_image_ref(zbar_image_t *image,
                           int refs);

/** image format conversion.  refer to the documentation for supported
 * image formats
 * @returns a @em new image with the sample data from the original image
 * converted to the requested format.  the original image is
 * unaffected.
 * @note the converted image size may be rounded (up) due to format
 * constraints
 */
extern zbar_image_t *zbar_image_convert(const zbar_image_t *image,
                                        unsigned long format);

/** image format conversion with crop/pad.
 * if the requested size is larger than the image, the last row/column
 * are duplicated to cover the difference.  if the requested size is
 * smaller than the image, the extra rows/columns are dropped from the
 * right/bottom.
 * @returns a @em new image with the sample data from the original
 * image converted to the requested format and size.
 * @note the image is @em not scaled
 * @see zbar_image_convert()
 * @since 0.4
 */
extern zbar_image_t *zbar_image_convert_resize(const zbar_image_t *image,
                                               unsigned long format,
                                               unsigned width,
                                               unsigned height);

/** retrieve the image format.
 * @returns the fourcc describing the format of the image sample data
 */
extern unsigned long zbar_image_get_format(const zbar_image_t *image);

/** retrieve a "sequence" (page/frame) number associated with this image.
 * @since 0.6
 */
extern unsigned zbar_image_get_sequence(const zbar_image_t *image);

/** retrieve the width of the image.
 * @returns the width in sample columns
 */
extern unsigned zbar_image_get_width(const zbar_image_t *image);

/** retrieve the height of the image.
 * @returns the height in sample rows
 */
extern unsigned zbar_image_get_height(const zbar_image_t *image);

/** retrieve both dimensions of the image.
 * fills in the width and height in samples
 */
extern void zbar_image_get_size(const zbar_image_t *image,
                                unsigned *width,
                                unsigned *height);

/** retrieve the crop rectangle.
 * fills in the image coordinates of the upper left corner and size
 * of an axis-aligned rectangular area of the image that will be scanned.
 * defaults to the full image
 * @since 0.11
 */
extern void zbar_image_get_crop(const zbar_image_t *image,
                                unsigned *x,
                                unsigned *y,
                                unsigned *width,
                                unsigned *height);

/** return the image sample data.  the returned data buffer is only
 * valid until zbar_image_destroy() is called
 */
extern const void *zbar_image_get_data(const zbar_image_t *image);

/** return the size of image data.
 * @since 0.6
 */
extern unsigned long zbar_image_get_data_length(const zbar_image_t *img);

/** retrieve the decoded results.
 * @returns the (possibly empty) set of decoded symbols
 * @returns NULL if the image has not been scanned
 * @since 0.10
 */
extern const zbar_symbol_set_t*
zbar_image_get_symbols(const zbar_image_t *image);

/** associate the specified symbol set with the image, replacing any
 * existing results.  use NULL to release the current results from the
 * image.
 * @see zbar_image_scanner_recycle_image()
 * @since 0.10
 */
extern void zbar_image_set_symbols(zbar_image_t *image,
                                   const zbar_symbol_set_t *symbols);

/** image_scanner decode result iterator.
 * @returns the first decoded symbol result for an image
 * or NULL if no results are available
 */
extern const zbar_symbol_t*
zbar_image_first_symbol(const zbar_image_t *image);

/** specify the fourcc image format code for image sample data.
 * refer to the documentation for supported formats.
 * @note this does not convert the data!
 * (see zbar_image_convert() for that)
 */
extern void zbar_image_set_format(zbar_image_t *image,
                                  unsigned long format);

/** associate a "sequence" (page/frame) number with this image.
 * @since 0.6
 */
extern void zbar_image_set_sequence(zbar_image_t *image,
                                    unsigned sequence_num);

/** specify the pixel size of the image.
 * @note this also resets the crop rectangle to the full image
 * (0, 0, width, height)
 * @note this does not affect the data!
 */
extern void zbar_image_set_size(zbar_image_t *image,
                                unsigned width,
                                unsigned height);

/** specify a rectangular region of the image to scan.
 * the rectangle will be clipped to the image boundaries.
 * defaults to the full image specified by zbar_image_set_size()
 */
extern void zbar_image_set_crop(zbar_image_t *image,
                                unsigned x,
                                unsigned y,
                                unsigned width,
                                unsigned height);

/** specify image sample data.  when image data is no longer needed by
 * the library the specific data cleanup handler will be called
 * (unless NULL)
 * @note application image data will not be modified by the library
 */
extern void zbar_image_set_data(zbar_image_t *image,
                                const void *data,
                                unsigned long data_byte_length,
                                zbar_image_cleanup_handler_t *cleanup_hndlr);

/** built-in cleanup handler.
 * passes the image data buffer to free()
 */
extern void zbar_image_free_data(zbar_image_t *image);

/** associate user specified data value with an image.
 * @since 0.5
 */
extern void zbar_image_set_userdata(zbar_image_t *image,
                                    void *userdata);

/** return user specified data value associated with the image.
 * @since 0.5
 */
extern void *zbar_image_get_userdata(const zbar_image_t *image);

/** dump raw image data to a file for debug.
 * the data will be prefixed with a 16 byte header consisting of:
 *   - 4 bytes uint = 0x676d697a ("zimg")
 *   - 4 bytes format fourcc
 *   - 2 bytes width
 *   - 2 bytes height
 *   - 4 bytes size of following image data in bytes
 * this header can be dumped w/eg:
 * @verbatim
       od -Ax -tx1z -N16 -w4 [file]
@endverbatim
 * for some formats the image can be displayed/converted using
 * ImageMagick, eg:
 * @verbatim
       display -size 640x480+16 [-depth ?] [-sampling-factor ?x?] \
           {GRAY,RGB,UYVY,YUV}:[file]
@endverbatim
 *
 * @param image the image object to dump
 * @param filebase base filename, appended with ".XXXX.zimg" where
 * XXXX is the format fourcc
 * @returns 0 on success or a system error code on failure
 */
extern int zbar_image_write(const zbar_image_t *image,
                            const char *filebase);

/** read back an image in the format written by zbar_image_write()
 * @note TBD
 */
extern zbar_image_t *zbar_image_read(char *filename);

/*@}*/

/*------------------------------------------------------------*/
/** @name Processor interface
 * @anchor c-processor
 * high-level self-contained image processor.
 * processes video and images for barcodes, optionally displaying
 * images to a library owned output window
 */
/*@{*/

struct zbar_processor_s;
/** opaque standalone processor object. */
typedef struct zbar_processor_s zbar_processor_t;

/** constructor.
 * if threaded is set and threading is available the processor
 * will spawn threads where appropriate to avoid blocking and
 * improve responsiveness
 */
extern zbar_processor_t *zbar_processor_create(int threaded);

/** destructor.  cleans up all resources associated with the processor
 */
extern void zbar_processor_destroy(zbar_processor_t *processor);

/** (re)initialization.
 * opens a video input device and/or prepares to display output
 */
extern int zbar_processor_init(zbar_processor_t *processor,
                               const char *video_device,
                               int enable_display);

/** request a preferred size for the video image from the device.
 * the request may be adjusted or completely ignored by the driver.
 * @note must be called before zbar_processor_init()
 * @since 0.6
 */
extern int zbar_processor_request_size(zbar_processor_t *processor,
                                       unsigned width,
                                       unsigned height);

/** request a preferred video driver interface version for
 * debug/testing.
 * @note must be called before zbar_processor_init()
 * @since 0.6
 */
extern int zbar_processor_request_interface(zbar_processor_t *processor,
                                            int version);

/** request a preferred video I/O mode for debug/testing.  You will
 * get errors if the driver does not support the specified mode.
 * @verbatim
    0 = auto-detect
    1 = force I/O using read()
    2 = force memory mapped I/O using mmap()
    3 = force USERPTR I/O (v4l2 only)
@endverbatim
 * @note must be called before zbar_processor_init()
 * @since 0.7
 */
extern int zbar_processor_request_iomode(zbar_processor_t *video,
                                         int iomode);

/** force specific input and output formats for debug/testing.
 * @note must be called before zbar_processor_init()
 */
extern int zbar_processor_force_format(zbar_processor_t *processor,
                                       unsigned long input_format,
                                       unsigned long output_format);

/** setup result handler callback.
 * the specified function will be called by the processor whenever
 * new results are available from the video stream or a static image.
 * pass a NULL value to disable callbacks.
 * @param processor the object on which to set the handler.
 * @param handler the function to call when new results are available.
 * @param userdata is set as with zbar_processor_set_userdata().
 * @returns the previously registered handler
 */
extern zbar_image_data_handler_t*
zbar_processor_set_data_handler(zbar_processor_t *processor,
                                zbar_image_data_handler_t *handler,
                                const void *userdata);

/** associate user specified data value with the processor.
 * @since 0.6
 */
extern void zbar_processor_set_userdata(zbar_processor_t *processor,
                                        void *userdata);

/** return user specified data value associated with the processor.
 * @since 0.6
 */
extern void *zbar_processor_get_userdata(const zbar_processor_t *processor);

/** set config for indicated symbology (0 for all) to specified value.
 * @returns 0 for success, non-0 for failure (config does not apply to
 * specified symbology, or value out of range)
 * @see zbar_decoder_set_config()
 * @since 0.4
 */
extern int zbar_processor_set_config(zbar_processor_t *processor,
                                     zbar_symbol_type_t symbology,
                                     zbar_config_t config,
                                     int value);

/** parse configuration string using zbar_parse_config()
 * and apply to processor using zbar_processor_set_config().
 * @returns 0 for success, non-0 for failure
 * @see zbar_parse_config()
 * @see zbar_processor_set_config()
 * @since 0.4
 */
static inline int zbar_processor_parse_config (zbar_processor_t *processor,
                                               const char *config_string)
{
    zbar_symbol_type_t sym;
    zbar_config_t cfg;
    int val;
    return(zbar_parse_config(config_string, &sym, &cfg, &val) ||
           zbar_processor_set_config(processor, sym, cfg, val));
}

/** retrieve the current state of the ouput window.
 * @returns 1 if the output window is currently displayed, 0 if not.
 * @returns -1 if an error occurs
 */
extern int zbar_processor_is_visible(zbar_processor_t *processor);

/** show or hide the display window owned by the library.
 * the size will be adjusted to the input size
 */
extern int zbar_processor_set_visible(zbar_processor_t *processor,
                                      int visible);

/** control the processor in free running video mode.
 * only works if video input is initialized. if threading is in use,
 * scanning will occur in the background, otherwise this is only
 * useful wrapping calls to zbar_processor_user_wait(). if the
 * library output window is visible, video display will be enabled.
 */
extern int zbar_processor_set_active(zbar_processor_t *processor,
                                     int active);

/** retrieve decode results for last scanned image/frame.
 * @returns the symbol set result container or NULL if no results are
 * available
 * @note the returned symbol set has its reference count incremented;
 * ensure that the count is decremented after use
 * @since 0.10
 */
extern const zbar_symbol_set_t*
zbar_processor_get_results(const zbar_processor_t *processor);

/** wait for input to the display window from the user
 * (via mouse or keyboard).
 * @returns >0 when input is received, 0 if timeout ms expired
 * with no input or -1 in case of an error
 */
extern int zbar_processor_user_wait(zbar_processor_t *processor,
                                    int timeout);

/** process from the video stream until a result is available,
 * or the timeout (in milliseconds) expires.
 * specify a timeout of -1 to scan indefinitely
 * (zbar_processor_set_active() may still be used to abort the scan
 * from another thread).
 * if the library window is visible, video display will be enabled.
 * @note that multiple results may still be returned (despite the
 * name).
 * @returns >0 if symbols were successfully decoded,
 * 0 if no symbols were found (ie, the timeout expired)
 * or -1 if an error occurs
 */
extern int zbar_process_one(zbar_processor_t *processor,
                            int timeout);

/** process the provided image for barcodes.
 * if the library window is visible, the image will be displayed.
 * @returns >0 if symbols were successfully decoded,
 * 0 if no symbols were found or -1 if an error occurs
 */
extern int zbar_process_image(zbar_processor_t *processor,
                              zbar_image_t *image);

/** display detail for last processor error to stderr.
 * @returns a non-zero value suitable for passing to exit()
 */
static inline int
zbar_processor_error_spew (const zbar_processor_t *processor,
                           int verbosity)
{
    return(_zbar_error_spew(processor, verbosity));
}

/** retrieve the detail string for the last processor error. */
static inline const char*
zbar_processor_error_string (const zbar_processor_t *processor,
                             int verbosity)
{
    return(_zbar_error_string(processor, verbosity));
}

/** retrieve the type code for the last processor error. */
static inline zbar_error_t
zbar_processor_get_error_code (const zbar_processor_t *processor)
{
    return(_zbar_get_error_code(processor));
}

/*@}*/

/*------------------------------------------------------------*/
/** @name Video interface
 * @anchor c-video
 * mid-level video source abstraction.
 * captures images from a video device
 */
/*@{*/

struct zbar_video_s;
/** opaque video object. */
typedef struct zbar_video_s zbar_video_t;

/** constructor. */
extern zbar_video_t *zbar_video_create(void);

/** destructor. */
extern void zbar_video_destroy(zbar_video_t *video);

/** open and probe a video device.
 * the device specified by platform specific unique name
 * (v4l device node path in *nix eg "/dev/video",
 *  DirectShow DevicePath property in windows).
 * @returns 0 if successful or -1 if an error occurs
 */
extern int zbar_video_open(zbar_video_t *video,
                           const char *device);

/** retrieve file descriptor associated with open *nix video device
 * useful for using select()/poll() to tell when new images are
 * available (NB v4l2 only!!).
 * @returns the file descriptor or -1 if the video device is not open
 * or the driver only supports v4l1
 */
extern int zbar_video_get_fd(const zbar_video_t *video);

/** request a preferred size for the video image from the device.
 * the request may be adjusted or completely ignored by the driver.
 * @returns 0 if successful or -1 if the video device is already
 * initialized
 * @since 0.6
 */
extern int zbar_video_request_size(zbar_video_t *video,
                                   unsigned width,
                                   unsigned height);

/** request a preferred driver interface version for debug/testing.
 * @note must be called before zbar_video_open()
 * @since 0.6
 */
extern int zbar_video_request_interface(zbar_video_t *video,
                                        int version);

/** request a preferred I/O mode for debug/testing.  You will get
 * errors if the driver does not support the specified mode.
 * @verbatim
    0 = auto-detect
    1 = force I/O using read()
    2 = force memory mapped I/O using mmap()
    3 = force USERPTR I/O (v4l2 only)
@endverbatim
 * @note must be called before zbar_video_open()
 * @since 0.7
 */
extern int zbar_video_request_iomode(zbar_video_t *video,
                                     int iomode);

/** retrieve current output image width.
 * @returns the width or 0 if the video device is not open
 */
extern int zbar_video_get_width(const zbar_video_t *video);

/** retrieve current output image height.
 * @returns the height or 0 if the video device is not open
 */
extern int zbar_video_get_height(const zbar_video_t *video);

/** initialize video using a specific format for debug.
 * use zbar_negotiate_format() to automatically select and initialize
 * the best available format
 */
extern int zbar_video_init(zbar_video_t *video,
                           unsigned long format);

/** start/stop video capture.
 * all buffered images are retired when capture is disabled.
 * @returns 0 if successful or -1 if an error occurs
 */
extern int zbar_video_enable(zbar_video_t *video,
                             int enable);

/** retrieve next captured image.  blocks until an image is available.
 * @returns NULL if video is not enabled or an error occurs
 */
extern zbar_image_t *zbar_video_next_image(zbar_video_t *video);

/** display detail for last video error to stderr.
 * @returns a non-zero value suitable for passing to exit()
 */
static inline int zbar_video_error_spew (const zbar_video_t *video,
                                         int verbosity)
{
    return(_zbar_error_spew(video, verbosity));
}

/** retrieve the detail string for the last video error. */
static inline const char *zbar_video_error_string (const zbar_video_t *video,
                                                   int verbosity)
{
    return(_zbar_error_string(video, verbosity));
}

/** retrieve the type code for the last video error. */
static inline zbar_error_t
zbar_video_get_error_code (const zbar_video_t *video)
{
    return(_zbar_get_error_code(video));
}

/*@}*/

/*------------------------------------------------------------*/
/** @name Window interface
 * @anchor c-window
 * mid-level output window abstraction.
 * displays images to user-specified platform specific output window
 */
/*@{*/

struct zbar_window_s;
/** opaque window object. */
typedef struct zbar_window_s zbar_window_t;

/** constructor. */
extern zbar_window_t *zbar_window_create(void);

/** destructor. */
extern void zbar_window_destroy(zbar_window_t *window);

/** associate reader with an existing platform window.
 * This can be any "Drawable" for X Windows or a "HWND" for windows.
 * input images will be scaled into the output window.
 * pass NULL to detach from the resource, further input will be
 * ignored
 */
extern int zbar_window_attach(zbar_window_t *window,
                              void *x11_display_w32_hwnd,
                              unsigned long x11_drawable);

/** control content level of the reader overlay.
 * the overlay displays graphical data for informational or debug
 * purposes.  higher values increase the level of annotation (possibly
 * decreasing performance). @verbatim
    0 = disable overlay
    1 = outline decoded symbols (default)
    2 = also track and display input frame rate
@endverbatim
 */
extern void zbar_window_set_overlay(zbar_window_t *window,
                                    int level);

/** retrieve current content level of reader overlay.
 * @see zbar_window_set_overlay()
 * @since 0.10
 */
extern int zbar_window_get_overlay(const zbar_window_t *window);

/** draw a new image into the output window. */
extern int zbar_window_draw(zbar_window_t *window,
                            zbar_image_t *image);

/** redraw the last image (exposure handler). */
extern int zbar_window_redraw(zbar_window_t *window);

/** resize the image window (reconfigure handler).
 * this does @em not update the contents of the window
 * @since 0.3, changed in 0.4 to not redraw window
 */
extern int zbar_window_resize(zbar_window_t *window,
                              unsigned width,
                              unsigned height);

/** display detail for last window error to stderr.
 * @returns a non-zero value suitable for passing to exit()
 */
static inline int zbar_window_error_spew (const zbar_window_t *window,
                                          int verbosity)
{
    return(_zbar_error_spew(window, verbosity));
}

/** retrieve the detail string for the last window error. */
static inline const char*
zbar_window_error_string (const zbar_window_t *window,
                          int verbosity)
{
    return(_zbar_error_string(window, verbosity));
}

/** retrieve the type code for the last window error. */
static inline zbar_error_t
zbar_window_get_error_code (const zbar_window_t *window)
{
    return(_zbar_get_error_code(window));
}


/** select a compatible format between video input and output window.
 * the selection algorithm attempts to use a format shared by
 * video input and window output which is also most useful for
 * barcode scanning.  if a format conversion is necessary, it will
 * heuristically attempt to minimize the cost of the conversion
 */
extern int zbar_negotiate_format(zbar_video_t *video,
                                 zbar_window_t *window);

/*@}*/

/*------------------------------------------------------------*/
/** @name Image Scanner interface
 * @anchor c-imagescanner
 * mid-level image scanner interface.
 * reads barcodes from 2-D images
 */
/*@{*/

struct zbar_image_scanner_s;
/** opaque image scanner object. */
typedef struct zbar_image_scanner_s zbar_image_scanner_t;

/** constructor. */
extern zbar_image_scanner_t *zbar_image_scanner_create(void);

/** destructor. */
extern void zbar_image_scanner_destroy(zbar_image_scanner_t *scanner);

/** setup result handler callback.
 * the specified function will be called by the scanner whenever
 * new results are available from a decoded image.
 * pass a NULL value to disable callbacks.
 * @returns the previously registered handler
 */
extern zbar_image_data_handler_t*
zbar_image_scanner_set_data_handler(zbar_image_scanner_t *scanner,
                                    zbar_image_data_handler_t *handler,
                                    const void *userdata);


/** set config for indicated symbology (0 for all) to specified value.
 * @returns 0 for success, non-0 for failure (config does not apply to
 * specified symbology, or value out of range)
 * @see zbar_decoder_set_config()
 * @since 0.4
 */
extern int zbar_image_scanner_set_config(zbar_image_scanner_t *scanner,
                                         zbar_symbol_type_t symbology,
                                         zbar_config_t config,
                                         int value);

/** parse configuration string using zbar_parse_config()
 * and apply to image scanner using zbar_image_scanner_set_config().
 * @returns 0 for success, non-0 for failure
 * @see zbar_parse_config()
 * @see zbar_image_scanner_set_config()
 * @since 0.4
 */
static inline int
zbar_image_scanner_parse_config (zbar_image_scanner_t *scanner,
                                 const char *config_string)
{
    zbar_symbol_type_t sym;
    zbar_config_t cfg;
    int val;
    return(zbar_parse_config(config_string, &sym, &cfg, &val) ||
           zbar_image_scanner_set_config(scanner, sym, cfg, val));
}

/** enable or disable the inter-image result cache (default disabled).
 * mostly useful for scanning video frames, the cache filters
 * duplicate results from consecutive images, while adding some
 * consistency checking and hysteresis to the results.
 * this interface also clears the cache
 */
extern void zbar_image_scanner_enable_cache(zbar_image_scanner_t *scanner,
                                            int enable);

/** remove any previously decoded results from the image scanner and the
 * specified image.  somewhat more efficient version of
 * zbar_image_set_symbols(image, NULL) which may retain memory for
 * subsequent decodes
 * @since 0.10
 */
extern void zbar_image_scanner_recycle_image(zbar_image_scanner_t *scanner,
                                             zbar_image_t *image);

/** retrieve decode results for last scanned image.
 * @returns the symbol set result container or NULL if no results are
 * available
 * @note the symbol set does not have its reference count adjusted;
 * ensure that the count is incremented if the results may be kept
 * after the next image is scanned
 * @since 0.10
 */
extern const zbar_symbol_set_t*
zbar_image_scanner_get_results(const zbar_image_scanner_t *scanner);

/** scan for symbols in provided image.  The image format must be
 * "Y800" or "GRAY".
 * @returns >0 if symbols were successfully decoded from the image,
 * 0 if no symbols were found or -1 if an error occurs
 * @see zbar_image_convert()
 * @since 0.9 - changed to only accept grayscale images
 */
extern int zbar_scan_image(zbar_image_scanner_t *scanner,
                           zbar_image_t *image);

/*@}*/

/*------------------------------------------------------------*/
/** @name Decoder interface
 * @anchor c-decoder
 * low-level bar width stream decoder interface.
 * identifies symbols and extracts encoded data
 */
/*@{*/

struct zbar_decoder_s;
/** opaque decoder object. */
typedef struct zbar_decoder_s zbar_decoder_t;

/** decoder data handler callback function.
 * called by decoder when new data has just been decoded
 */
typedef void (zbar_decoder_handler_t)(zbar_decoder_t *decoder);

/** constructor. */
extern zbar_decoder_t *zbar_decoder_create(void);

/** destructor. */
extern void zbar_decoder_destroy(zbar_decoder_t *decoder);

/** set config for indicated symbology (0 for all) to specified value.
 * @returns 0 for success, non-0 for failure (config does not apply to
 * specified symbology, or value out of range)
 * @since 0.4
 */
extern int zbar_decoder_set_config(zbar_decoder_t *decoder,
                                   zbar_symbol_type_t symbology,
                                   zbar_config_t config,
                                   int value);

/** parse configuration string using zbar_parse_config()
 * and apply to decoder using zbar_decoder_set_config().
 * @returns 0 for success, non-0 for failure
 * @see zbar_parse_config()
 * @see zbar_decoder_set_config()
 * @since 0.4
 */
static inline int zbar_decoder_parse_config (zbar_decoder_t *decoder,
                                             const char *config_string)
{
    zbar_symbol_type_t sym;
    zbar_config_t cfg;
    int val;
    return(zbar_parse_config(config_string, &sym, &cfg, &val) ||
           zbar_decoder_set_config(decoder, sym, cfg, val));
}

/** retrieve symbology boolean config settings.
 * @returns a bitmask indicating which configs are currently set for the
 * specified symbology.
 * @since 0.11
 */
extern unsigned int zbar_decoder_get_configs(const zbar_decoder_t *decoder,
                                             zbar_symbol_type_t symbology);

/** clear all decoder state.
 * any partial symbols are flushed
 */
extern void zbar_decoder_reset(zbar_decoder_t *decoder);

/** mark start of a new scan pass.
 * clears any intra-symbol state and resets color to ::ZBAR_SPACE.
 * any partially decoded symbol state is retained
 */
extern void zbar_decoder_new_scan(zbar_decoder_t *decoder);

/** process next bar/space width from input stream.
 * the width is in arbitrary relative units.  first value of a scan
 * is ::ZBAR_SPACE width, alternating from there.
 * @returns appropriate symbol type if width completes
 * decode of a symbol (data is available for retrieval)
 * @returns ::ZBAR_PARTIAL as a hint if part of a symbol was decoded
 * @returns ::ZBAR_NONE (0) if no new symbol data is available
 */
extern zbar_symbol_type_t zbar_decode_width(zbar_decoder_t *decoder,
                                            unsigned width);

/** retrieve color of @em next element passed to
 * zbar_decode_width(). */
extern zbar_color_t zbar_decoder_get_color(const zbar_decoder_t *decoder);

/** retrieve last decoded data.
 * @returns the data string or NULL if no new data available.
 * the returned data buffer is owned by library, contents are only
 * valid between non-0 return from zbar_decode_width and next library
 * call
 */
extern const char *zbar_decoder_get_data(const zbar_decoder_t *decoder);

/** retrieve length of binary data.
 * @returns the length of the decoded data or 0 if no new data
 * available.
 */
extern unsigned int
zbar_decoder_get_data_length(const zbar_decoder_t *decoder);

/** retrieve last decoded symbol type.
 * @returns the type or ::ZBAR_NONE if no new data available
 */
extern zbar_symbol_type_t
zbar_decoder_get_type(const zbar_decoder_t *decoder);

/** retrieve modifier flags for the last decoded symbol.
 * @returns a bitmask indicating which characteristics were detected
 * during decoding.
 * @since 0.11
 */
extern unsigned int zbar_decoder_get_modifiers(const zbar_decoder_t *decoder);

/** retrieve last decode direction.
 * @returns 1 for forward and -1 for reverse
 * @returns 0 if the decode direction is unknown or does not apply
 * @since 0.11
 */
extern int zbar_decoder_get_direction(const zbar_decoder_t *decoder);

/** setup data handler callback.
 * the registered function will be called by the decoder
 * just before zbar_decode_width() returns a non-zero value.
 * pass a NULL value to disable callbacks.
 * @returns the previously registered handler
 */
extern zbar_decoder_handler_t*
zbar_decoder_set_handler(zbar_decoder_t *decoder,
                         zbar_decoder_handler_t *handler);

/** associate user specified data value with the decoder. */
extern void zbar_decoder_set_userdata(zbar_decoder_t *decoder,
                                      void *userdata);

/** return user specified data value associated with the decoder. */
extern void *zbar_decoder_get_userdata(const zbar_decoder_t *decoder);

/*@}*/

/*------------------------------------------------------------*/
/** @name Scanner interface
 * @anchor c-scanner
 * low-level linear intensity sample stream scanner interface.
 * identifies "bar" edges and measures width between them.
 * optionally passes to bar width decoder
 */
/*@{*/

struct zbar_scanner_s;
/** opaque scanner object. */
typedef struct zbar_scanner_s zbar_scanner_t;

/** constructor.
 * if decoder is non-NULL it will be attached to scanner
 * and called automatically at each new edge
 * current color is initialized to ::ZBAR_SPACE
 * (so an initial BAR->SPACE transition may be discarded)
 */
extern zbar_scanner_t *zbar_scanner_create(zbar_decoder_t *decoder);

/** destructor. */
extern void zbar_scanner_destroy(zbar_scanner_t *scanner);

/** clear all scanner state.
 * also resets an associated decoder
 */
extern zbar_symbol_type_t zbar_scanner_reset(zbar_scanner_t *scanner);

/** mark start of a new scan pass. resets color to ::ZBAR_SPACE.
 * also updates an associated decoder.
 * @returns any decode results flushed from the pipeline
 * @note when not using callback handlers, the return value should
 * be checked the same as zbar_scan_y()
 * @note call zbar_scanner_flush() at least twice before calling this
 * method to ensure no decode results are lost
 */
extern zbar_symbol_type_t zbar_scanner_new_scan(zbar_scanner_t *scanner);

/** flush scanner processing pipeline.
 * forces current scanner position to be a scan boundary.
 * call multiple times (max 3) to completely flush decoder.
 * @returns any decode/scan results flushed from the pipeline
 * @note when not using callback handlers, the return value should
 * be checked the same as zbar_scan_y()
 * @since 0.9
 */
extern zbar_symbol_type_t zbar_scanner_flush(zbar_scanner_t *scanner);

/** process next sample intensity value.
 * intensity (y) is in arbitrary relative units.
 * @returns result of zbar_decode_width() if a decoder is attached,
 * otherwise @returns (::ZBAR_PARTIAL) when new edge is detected
 * or 0 (::ZBAR_NONE) if no new edge is detected
 */
extern zbar_symbol_type_t zbar_scan_y(zbar_scanner_t *scanner,
                                      int y);

/** process next sample from RGB (or BGR) triple. */
static inline zbar_symbol_type_t zbar_scan_rgb24 (zbar_scanner_t *scanner,
                                                    unsigned char *rgb)
{
    return(zbar_scan_y(scanner, rgb[0] + rgb[1] + rgb[2]));
}

/** retrieve last scanned width. */
extern unsigned zbar_scanner_get_width(const zbar_scanner_t *scanner);

/** retrieve sample position of last edge.
 * @since 0.10
 */
extern unsigned zbar_scanner_get_edge(const zbar_scanner_t *scn,
                                      unsigned offset,
                                      int prec);

/** retrieve last scanned color. */
extern zbar_color_t zbar_scanner_get_color(const zbar_scanner_t *scanner);

/*@}*/

#ifdef __cplusplus
    }
}

# include "zbar/Exception.h"
# include "zbar/Decoder.h"
# include "zbar/Scanner.h"
# include "zbar/Symbol.h"
# include "zbar/Image.h"
# include "zbar/ImageScanner.h"
# include "zbar/Video.h"
# include "zbar/Window.h"
# include "zbar/Processor.h"
#endif

#endif
