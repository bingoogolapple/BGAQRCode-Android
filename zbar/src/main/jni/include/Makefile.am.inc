zincludedir = $(includedir)/zbar
include_HEADERS = include/zbar.h
zinclude_HEADERS = include/zbar/Scanner.h include/zbar/Decoder.h \
    include/zbar/Exception.h include/zbar/Symbol.h include/zbar/Image.h \
    include/zbar/ImageScanner.h include/zbar/Video.h include/zbar/Window.h \
    include/zbar/Processor.h

if HAVE_GTK
zinclude_HEADERS += include/zbar/zbargtk.h
endif
if HAVE_QT
zinclude_HEADERS += include/zbar/QZBar.h include/zbar/QZBarImage.h
endif
