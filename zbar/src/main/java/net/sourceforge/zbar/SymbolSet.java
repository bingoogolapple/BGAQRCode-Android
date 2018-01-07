/*------------------------------------------------------------------------
 *  SymbolSet
 *
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

package net.sourceforge.zbar;

/** Immutable container for decoded result symbols associated with an image
 * or a composite symbol.
 */
public class SymbolSet
    extends java.util.AbstractCollection<Symbol>
{
    /** C pointer to a zbar_symbol_set_t. */
    private long peer;

    static
    {
        System.loadLibrary("zbarjni");
        init();
    }
    private static native void init();

    /** SymbolSets are only created by other package methods. */
    SymbolSet (long peer)
    {
        this.peer = peer;
    }

    protected void finalize ()
    {
        destroy();
    }

    /** Clean up native data associated with an instance. */
    public synchronized void destroy ()
    {
        if(peer != 0) {
            destroy(peer);
            peer = 0;
        }
    }

    /** Release the associated peer instance.  */
    private native void destroy(long peer);

    /** Retrieve an iterator over the Symbol elements in this collection. */
    public java.util.Iterator<Symbol> iterator ()
    {
        long sym = firstSymbol(peer);
        if(sym == 0)
            return(new SymbolIterator(null));

        return(new SymbolIterator(new Symbol(sym)));
    }

    /** Retrieve the number of elements in the collection. */
    public native int size();

    /** Retrieve C pointer to first symbol in the set. */
    private native long firstSymbol(long peer);
}
