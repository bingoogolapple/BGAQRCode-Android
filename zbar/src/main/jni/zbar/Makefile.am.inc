lib_LTLIBRARIES += zbar/libzbar.la
zbar_libzbar_la_CPPFLAGS = -I$(srcdir)/zbar $(AM_CPPFLAGS)
zbar_libzbar_la_LDFLAGS = -no-undefined -version-info $(LIB_VERSION) \
    -export-symbols-regex "^(zbar|_zbar.*_error)_.*" $(AM_LDFLAGS)
zbar_libzbar_la_LIBADD = $(LTLIBICONV)

zbar_libzbar_la_SOURCES = zbar/debug.h zbar/config.c \
    zbar/error.h zbar/error.c zbar/symbol.h zbar/symbol.c \
    zbar/image.h zbar/image.c zbar/convert.c \
    zbar/processor.c zbar/processor.h zbar/processor/lock.c \
    zbar/refcnt.h zbar/refcnt.c zbar/timer.h zbar/mutex.h \
    zbar/event.h zbar/thread.h \
    zbar/window.h zbar/window.c zbar/video.h zbar/video.c \
    zbar/img_scanner.h zbar/img_scanner.c zbar/scanner.c \
    zbar/decoder.h zbar/decoder.c

EXTRA_zbar_libzbar_la_SOURCES = zbar/svg.h zbar/svg.c

if ENABLE_EAN
zbar_libzbar_la_SOURCES += zbar/decoder/ean.h zbar/decoder/ean.c
endif
if ENABLE_DATABAR
zbar_libzbar_la_SOURCES += zbar/decoder/databar.h zbar/decoder/databar.c
endif
if ENABLE_CODE128
zbar_libzbar_la_SOURCES += zbar/decoder/code128.h zbar/decoder/code128.c
endif
if ENABLE_CODE93
zbar_libzbar_la_SOURCES += zbar/decoder/code93.h zbar/decoder/code93.c
endif
if ENABLE_CODE39
zbar_libzbar_la_SOURCES += zbar/decoder/code39.h zbar/decoder/code39.c
endif
if ENABLE_CODABAR
zbar_libzbar_la_SOURCES += zbar/decoder/codabar.h zbar/decoder/codabar.c
endif
if ENABLE_I25
zbar_libzbar_la_SOURCES += zbar/decoder/i25.h zbar/decoder/i25.c
endif
if ENABLE_PDF417
zbar_libzbar_la_SOURCES += zbar/decoder/pdf417.h zbar/decoder/pdf417.c \
    zbar/decoder/pdf417_hash.h
endif
if ENABLE_QRCODE
zbar_libzbar_la_SOURCES += zbar/qrcode.h \
    zbar/decoder/qr_finder.h zbar/decoder/qr_finder.c \
    zbar/qrcode/qrdec.h zbar/qrcode/qrdec.c zbar/qrcode/qrdectxt.c \
    zbar/qrcode/rs.h zbar/qrcode/rs.c \
    zbar/qrcode/isaac.h zbar/qrcode/isaac.c \
    zbar/qrcode/bch15_5.h zbar/qrcode/bch15_5.c \
    zbar/qrcode/binarize.h zbar/qrcode/binarize.c \
    zbar/qrcode/util.h zbar/qrcode/util.c
endif

if WIN32
zbar_libzbar_la_SOURCES += zbar/processor/win.c zbar/libzbar.rc
zbar_libzbar_la_CPPFLAGS += -mthreads
zbar_libzbar_la_LDFLAGS += -mthreads
# FIXME broken
zbar_libzbar_la_LIBADD += zbar/libzbar-rc.lo
else
zbar_libzbar_la_SOURCES += zbar/processor/posix.h zbar/processor/posix.c
endif

if HAVE_V4L2
zbar_libzbar_la_SOURCES += zbar/video/v4l.c zbar/video/v4l2.c
endif
if HAVE_V4L1
if !HAVE_V4L2
zbar_libzbar_la_SOURCES += zbar/video/v4l.c
endif
zbar_libzbar_la_SOURCES += zbar/video/v4l1.c
endif
if WIN32
if HAVE_VIDEO
zbar_libzbar_la_SOURCES += zbar/video/vfw.c
zbar_libzbar_la_LIBADD += -lvfw32
endif
endif
if !HAVE_VIDEO
zbar_libzbar_la_SOURCES += zbar/video/null.c
endif

if HAVE_JPEG
zbar_libzbar_la_SOURCES += zbar/jpeg.c
endif

if HAVE_X
zbar_libzbar_la_SOURCES +=  zbar/processor/x.c \
    zbar/window/x.h zbar/window/x.c zbar/window/ximage.c
zbar_libzbar_la_CPPFLAGS += $(X_CFLAGS)
zbar_libzbar_la_LDFLAGS += $(X_LIBS)
zbar_libzbar_la_LIBADD += $(X_PRE_LIBS) -lX11 $(X_EXTRA_LIBS)
if HAVE_XV
zbar_libzbar_la_SOURCES += zbar/window/xv.c
zbar_libzbar_la_LIBADD += $(XV_LIBS)
endif
else
if WIN32
zbar_libzbar_la_SOURCES += zbar/window/win.h zbar/window/win.c \
    zbar/window/dib.c
# zbar/window/vfw.c -lvfw32
zbar_libzbar_la_LIBADD += -lgdi32 -lwinmm
else
zbar_libzbar_la_SOURCES += zbar/processor/null.c zbar/window/null.c
endif
endif

zbar_libzbar_la_LDFLAGS += $(AM_LDFLAGS)
zbar_libzbar_la_LIBADD += $(AM_LIBADD)
