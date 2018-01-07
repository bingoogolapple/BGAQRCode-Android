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
#ifndef _ZBAR_EXCEPTION_H_
#define _ZBAR_EXCEPTION_H_

/// @file
/// C++ Exception definitions

#ifndef _ZBAR_H_
# error "include zbar.h in your application, **not** zbar/Exception.h"
#endif

#include <exception>
#include <new>

namespace zbar {

/// base class for exceptions defined by this API.
class Exception : public std::exception {

public:
    /// create exception from C library error
    Exception (const void *obj = NULL)
        : std::exception(),
          _obj(obj)
    { }

    ~Exception () throw() { }

    /// retrieve error message
    virtual const char* what () const throw()
    {
        if(!_obj)
            return("zbar library unspecified generic error");
        return(_zbar_error_string(_obj, 0));
    }

private:
    const void *_obj;
};

/// internal library error.
class InternalError : public Exception {
public:
    /// create exception from C library error
    InternalError (const void *obj)
        : Exception(obj)
    { }
};

/// unsupported request.
class UnsupportedError : public Exception {
public:
    /// create exception from C library error
    UnsupportedError (const void *obj)
        : Exception(obj)
    { }
};

/// invalid request.
class InvalidError : public Exception {
public:
    /// create exception from C library error
    InvalidError (const void *obj)
        : Exception(obj)
    { }
};

/// failed system call.
class SystemError : public Exception {
public:
    /// create exception from C library error
    SystemError (const void *obj)
        : Exception(obj)
    { }
};

/// locking error.
class LockingError : public Exception {
public:
    /// create exception from C library error
    LockingError (const void *obj)
        : Exception(obj)
    { }
};

/// all resources busy.
class BusyError : public Exception {
public:
    /// create exception from C library error
    BusyError (const void *obj)
        : Exception(obj)
    { }
};

/// X11 display error.
class XDisplayError : public Exception {
public:
    /// create exception from C library error
    XDisplayError (const void *obj)
        : Exception(obj)
    { }
};

/// X11 protocol error.
class XProtoError : public Exception {
public:
    /// create exception from C library error
    XProtoError (const void *obj)
        : Exception(obj)
    { }
};

/// output window is closed.
class ClosedError : public Exception {
public:
    /// create exception from C library error
    ClosedError (const void *obj)
        : Exception(obj)
    { }
};

/// image format error
class FormatError : public Exception {
    // FIXME needs c equivalent

    virtual const char* what () const throw()
    {
        // FIXME what format?
        return("unsupported format");
    }
};

/// @internal

/// extract error information and create exception.
static inline std::exception throw_exception (const void *obj)
{
    switch(_zbar_get_error_code(obj)) {
    case ZBAR_ERR_NOMEM:
        throw std::bad_alloc();
    case ZBAR_ERR_INTERNAL:
        throw InternalError(obj);
    case ZBAR_ERR_UNSUPPORTED:
        throw UnsupportedError(obj);
    case ZBAR_ERR_INVALID:
        throw InvalidError(obj);
    case ZBAR_ERR_SYSTEM:
        throw SystemError(obj);
    case ZBAR_ERR_LOCKING:
        throw LockingError(obj);
    case ZBAR_ERR_BUSY:
        throw BusyError(obj);
    case ZBAR_ERR_XDISPLAY:
        throw XDisplayError(obj);
    case ZBAR_ERR_XPROTO:
        throw XProtoError(obj);
    case ZBAR_ERR_CLOSED:
        throw ClosedError(obj);
    default:
        throw Exception(obj);
    }
}

}

#endif
