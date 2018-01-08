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
#ifndef _ZBAR_SYMBOL_H_
#define _ZBAR_SYMBOL_H_

/// @file
/// Symbol C++ wrapper

#ifndef _ZBAR_H_
# error "include zbar.h in your application, **not** zbar/Symbol.h"
#endif

#include <stdlib.h>
#include <string>
#include <ostream>
#include <assert.h>

namespace zbar {

class SymbolIterator;

/// container for decoded result symbols associated with an image
/// or a composite symbol.

class SymbolSet {
public:
    /// constructor.
    SymbolSet (const zbar_symbol_set_t *syms = NULL)
        : _syms(syms)
    {
        ref();
    }

    /// copy constructor.
    SymbolSet (const SymbolSet& syms)
        : _syms(syms._syms)
    {
        ref();
    }

    /// destructor.
    ~SymbolSet ()
    {
        ref(-1);
    }

    /// assignment.
    SymbolSet& operator= (const SymbolSet& syms)
    {
        syms.ref();
        ref(-1);
        _syms = syms._syms;
        return(*this);
    }

    /// truth testing.
    bool operator! () const
    {
        return(!_syms || !get_size());
    }

    /// manipulate reference count.
    void ref (int delta = 1) const
    {
        if(_syms)
            zbar_symbol_set_ref((zbar_symbol_set_t*)_syms, delta);
    }

    /// cast to C symbol set.
    operator const zbar_symbol_set_t* () const
    {
        return(_syms);
    }

    int get_size () const
    {
        return((_syms) ? zbar_symbol_set_get_size(_syms) : 0);
    }

    /// create a new SymbolIterator over decoded results.
    SymbolIterator symbol_begin() const;

    /// return a SymbolIterator suitable for ending iteration.
    const SymbolIterator symbol_end() const;

private:
    const zbar_symbol_set_t *_syms;
};

/// decoded barcode symbol result object.  stores type, data, and
/// image location of decoded symbol

class Symbol {
public:

    /// image pixel location (x, y) coordinate tuple.
    class Point {
    public:
        int x;  ///< x-coordinate.
        int y;  ///< y-coordinate.

        Point () { }

        Point(int x, int y)
            : x(x), y(y)
        { }

        /// copy constructor.
        Point (const Point& pt)
            : x(pt.x),
              y(pt.y)
        { }

        /// assignment.
        Point& operator= (const Point& pt)
        {
            x = pt.x;
            y = pt.y;
            return(*this);
        }
    };

    /// iteration over Point objects in a symbol location polygon.
    class PointIterator
        : public std::iterator<std::input_iterator_tag, Point> {

    public:
        /// constructor.
        PointIterator (const Symbol *sym = NULL,
                       int index = 0)
            : _sym(sym),
              _index(index)
        {
            if(sym)
                sym->ref(1);
            if(!sym ||
               (unsigned)_index >= zbar_symbol_get_loc_size(*_sym))
                _index = -1;
        }

        /// copy constructor.
        PointIterator (const PointIterator& iter)
            : _sym(iter._sym),
              _index(iter._index)
        {
            if(_sym)
                _sym->ref();
        }

        /// destructor.
        ~PointIterator ()
        {
            if(_sym)
                _sym->ref(-1);
        }

        /// assignment.
        PointIterator& operator= (const PointIterator& iter)
        {
            if(iter._sym)
                iter._sym->ref();
            if(_sym)
                _sym->ref(-1);
            _sym = iter._sym;
            _index = iter._index;
            return(*this);
        }

        /// truth testing.
        bool operator! () const
        {
            return(!_sym || _index < 0);
        }

        /// advance iterator to next Point.
        PointIterator& operator++ ()
        {
            unsigned int i = ++_index;
            if(!_sym || i >= zbar_symbol_get_loc_size(*_sym))
                _index = -1;
            return(*this);
        }

        /// retrieve currently referenced Point.
        const Point operator* () const
        {
            assert(!!*this);
            if(!*this)
                return(Point());
            return(Point(zbar_symbol_get_loc_x(*_sym, _index),
                         zbar_symbol_get_loc_y(*_sym, _index)));
        }

        /// test if two iterators refer to the same Point in the same
        /// Symbol.
        bool operator== (const PointIterator& iter) const
        {
            return(_index == iter._index &&
                   ((_index < 0) || _sym == iter._sym));
        }

        /// test if two iterators refer to the same Point in the same
        /// Symbol.
        bool operator!= (const PointIterator& iter) const
        {
            return(!(*this == iter));
        }

    private:
        const Symbol *_sym;
        int _index;
    };

    /// constructor.
    Symbol (const zbar_symbol_t *sym = NULL)
        : _xmlbuf(NULL),
          _xmllen(0)
    {
        init(sym);
        ref();
    }

    /// copy constructor.
    Symbol (const Symbol& sym)
        : _sym(sym._sym),
          _type(sym._type),
          _data(sym._data),
          _xmlbuf(NULL),
          _xmllen(0)
    {
        ref();
    }

    /// destructor.
    ~Symbol () {
        if(_xmlbuf)
            free(_xmlbuf);
        ref(-1);
    }

    /// assignment.
    Symbol& operator= (const Symbol& sym)
    {
        sym.ref(1);
        ref(-1);
        _sym = sym._sym;
        _type = sym._type;
        _data = sym._data;
        return(*this);
    }

    Symbol& operator= (const zbar_symbol_t *sym)
    {
        if(sym)
            zbar_symbol_ref(sym, 1);
        ref(-1);
        init(sym);
        return(*this);
    }

    /// truth testing.
    bool operator! () const
    {
        return(!_sym);
    }

    void ref (int delta = 1) const
    {
        if(_sym)
            zbar_symbol_ref((zbar_symbol_t*)_sym, delta);
    }

    /// cast to C symbol.
    operator const zbar_symbol_t* () const
    {
        return(_sym);
    }

    /// test if two Symbol objects refer to the same C symbol.
    bool operator== (const Symbol& sym) const
    {
        return(_sym == sym._sym);
    }

    /// test if two Symbol objects refer to the same C symbol.
    bool operator!= (const Symbol& sym) const
    {
        return(!(*this == sym));
    }

    /// retrieve type of decoded symbol.
    zbar_symbol_type_t get_type () const
    {
        return(_type);
    }

    /// retrieve the string name of the symbol type.
    const std::string get_type_name () const
    {
        return(zbar_get_symbol_name(_type));
    }

    /// retrieve the string name for any addon.
    /// @deprecated in 0.11
    const std::string get_addon_name () const
    {
        return(zbar_get_addon_name(_type));
    }

    /// retrieve data decoded from symbol.
    const std::string get_data () const
    {
        return(_data);
    }

    /// retrieve length of binary data
    unsigned get_data_length () const
    {
        return((_sym) ? zbar_symbol_get_data_length(_sym) : 0);
    }

    /// retrieve inter-frame coherency count.
    /// see zbar_symbol_get_count()
    /// @since 0.5
    int get_count () const
    {
        return((_sym) ? zbar_symbol_get_count(_sym) : -1);
    }

    /// retrieve loosely defined relative quality metric.
    /// see zbar_symbol_get_quality()
    /// @since 0.11
    int get_quality () const
    {
        return((_sym) ? zbar_symbol_get_quality(_sym) : 0);
    }

    SymbolSet get_components () const
    {
        return(SymbolSet((_sym) ? zbar_symbol_get_components(_sym) : NULL));
    }

    /// create a new PointIterator at the start of the location
    /// polygon.
    PointIterator point_begin() const
    {
        return(PointIterator(this));
    }

    /// return a PointIterator suitable for ending iteration.
    const PointIterator point_end() const
    {
        return(PointIterator());
    }

    /// see zbar_symbol_get_loc_size().
    int get_location_size () const
    {
        return((_sym) ? zbar_symbol_get_loc_size(_sym) : 0);
    }

    /// see zbar_symbol_get_loc_x().
    int get_location_x (unsigned index) const
    {
        return((_sym) ? zbar_symbol_get_loc_x(_sym, index) : -1);
    }

    /// see zbar_symbol_get_loc_y().
    int get_location_y (unsigned index) const
    {
        return((_sym) ? zbar_symbol_get_loc_y(_sym, index) : -1);
    }

    /// see zbar_symbol_get_orientation().
    /// @since 0.11
    int get_orientation () const
    {
        return(zbar_symbol_get_orientation(_sym));
    }

    /// see zbar_symbol_xml().
    const std::string xml () const
    {
        if(!_sym)
            return("");
        return(zbar_symbol_xml(_sym, (char**)&_xmlbuf, (unsigned*)&_xmllen));
    }

protected:
    /// (re)initialize Symbol from C symbol object.
    void init (const zbar_symbol_t *sym = NULL)
    {
        _sym = sym;
        if(sym) {
            _type = zbar_symbol_get_type(sym);
            _data = std::string(zbar_symbol_get_data(sym),
                                zbar_symbol_get_data_length(sym));
        }
        else {
            _type = ZBAR_NONE;
            _data = "";
        }
    }

private:
    const zbar_symbol_t *_sym;
    zbar_symbol_type_t _type;
    std::string _data;
    char *_xmlbuf;
    unsigned _xmllen;
};

/// iteration over Symbol result objects in a scanned Image or SymbolSet.
class SymbolIterator
    : public std::iterator<std::input_iterator_tag, Symbol> {

public:
    /// default constructor.
    SymbolIterator ()
    { }

    /// constructor.
    SymbolIterator (const SymbolSet &syms)
        : _syms(syms)
    {
        const zbar_symbol_set_t *zsyms = _syms;
        if(zsyms)
            _sym = zbar_symbol_set_first_symbol(zsyms);
    }

    /// copy constructor.
    SymbolIterator (const SymbolIterator& iter)
        : _syms(iter._syms)
    {
        const zbar_symbol_set_t *zsyms = _syms;
        if(zsyms)
            _sym = zbar_symbol_set_first_symbol(zsyms);
    }

    ~SymbolIterator ()
    {
    }

    /// assignment.
    SymbolIterator& operator= (const SymbolIterator& iter)
    {
        _syms = iter._syms;
        _sym = iter._sym;
        return(*this);
    }

    bool operator! () const
    {
        return(!_syms || !_sym);
    }

    /// advance iterator to next Symbol.
    SymbolIterator& operator++ ()
    {
        if(!!_sym)
            _sym = zbar_symbol_next(_sym);
        else if(!!_syms)
            _sym = zbar_symbol_set_first_symbol(_syms);
        return(*this);
    }

    /// retrieve currently referenced Symbol.
    const Symbol operator* () const
    {
        return(_sym);
    }

    /// access currently referenced Symbol.
    const Symbol* operator-> () const
    {
        return(&_sym);
    }

    /// test if two iterators refer to the same Symbol
    bool operator== (const SymbolIterator& iter) const
    {
        // it is enough to test the symbols, as they belong
        // to only one set (also simplifies invalid case)
        return(_sym == iter._sym);
    }

    /// test if two iterators refer to the same Symbol
    bool operator!= (const SymbolIterator& iter) const
    {
        return(!(*this == iter));
    }

    const SymbolIterator end () const {
        return(SymbolIterator());
    }

private:
    SymbolSet _syms;
    Symbol _sym;
};

inline SymbolIterator SymbolSet::symbol_begin () const {
    return(SymbolIterator(*this));
}

inline const SymbolIterator SymbolSet::symbol_end () const {
    return(SymbolIterator());
}

/// @relates Symbol
/// stream the string representation of a Symbol.
static inline std::ostream& operator<< (std::ostream& out,
                                        const Symbol& sym)
{
    out << sym.get_type_name() << ":" << sym.get_data();
    return(out);
}

}

#endif
