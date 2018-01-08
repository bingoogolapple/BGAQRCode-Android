/*------------------------------------------------------------------------
 *  Symbol
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
public class Symbol
{
    /** No symbol decoded. */
    public static final int NONE = 0;
    /** Symbol detected but not decoded. */
    public static final int PARTIAL = 1;

    /** EAN-8. */
    public static final int EAN8 = 8;
    /** UPC-E. */
    public static final int UPCE = 9;
    /** ISBN-10 (from EAN-13). */
    public static final int ISBN10 = 10;
    /** UPC-A. */
    public static final int UPCA = 12;
    /** EAN-13. */
    public static final int EAN13 = 13;
    /** ISBN-13 (from EAN-13). */
    public static final int ISBN13 = 14;
    /** Interleaved 2 of 5. */
    public static final int I25 = 25;
    /** DataBar (RSS-14). */
    public static final int DATABAR = 34;
    /** DataBar Expanded. */
    public static final int DATABAR_EXP = 35;
    /** Codabar. */
    public static final int CODABAR = 38;
    /** Code 39. */
    public static final int CODE39 = 39;
    /** PDF417. */
    public static final int PDF417 = 57;
    /** QR Code. */
    public static final int QRCODE = 64;
    /** Code 93. */
    public static final int CODE93 = 93;
    /** Code 128. */
    public static final int CODE128 = 128;

    /** C pointer to a zbar_symbol_t. */
    private long peer;

    /** Cached attributes. */
    private int type;

    static
    {
        System.loadLibrary("zbarjni");
        init();
    }
    private static native void init();

    /** Symbols are only created by other package methods. */
    Symbol (long peer)
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

    /** Retrieve type of decoded symbol. */
    public int getType ()
    {
        if(type == 0)
            type = getType(peer);
        return(type);
    }

    private native int getType(long peer);

    /** Retrieve symbology boolean configs settings used during decode. */
    public native int getConfigMask();

    /** Retrieve symbology characteristics detected during decode. */
    public native int getModifierMask();

    /** Retrieve data decoded from symbol as a String. */
    public native String getData();

    /** Retrieve raw data bytes decoded from symbol. */
    public native byte[] getDataBytes();

    /** Retrieve a symbol confidence metric.  Quality is an unscaled,
     * relative quantity: larger values are better than smaller
     * values, where "large" and "small" are application dependent.
     */
    public native int getQuality();

    /** Retrieve current cache count.  When the cache is enabled for
     * the image_scanner this provides inter-frame reliability and
     * redundancy information for video streams.
     * @returns < 0 if symbol is still uncertain
     * @returns 0 if symbol is newly verified
     * @returns > 0 for duplicate symbols
     */
    public native int getCount();

    /** Retrieve an approximate, axis-aligned bounding box for the
     * symbol.
     */
    public int[] getBounds ()
    {
        int n = getLocationSize(peer);
        if(n <= 0)
            return(null);

        int[] bounds = new int[4];
        int xmin = Integer.MAX_VALUE;
        int xmax = Integer.MIN_VALUE;
        int ymin = Integer.MAX_VALUE;
        int ymax = Integer.MIN_VALUE;

        for(int i = 0; i < n; i++) {
            int x = getLocationX(peer, i);
            if(xmin > x) xmin = x;
            if(xmax < x) xmax = x;

            int y = getLocationY(peer, i);
            if(ymin > y) ymin = y;
            if(ymax < y) ymax = y;
        }
        bounds[0] = xmin;
        bounds[1] = ymin;
        bounds[2] = xmax - xmin;
        bounds[3] = ymax - ymin;
        return(bounds);
    }

    private native int getLocationSize(long peer);
    private native int getLocationX(long peer, int idx);
    private native int getLocationY(long peer, int idx);

    public int[] getLocationPoint (int idx)
    {
        int[] p = new int[2];
        p[0] = getLocationX(peer, idx);
        p[1] = getLocationY(peer, idx);
        return(p);
    }

    /** Retrieve general axis-aligned, orientation of decoded
     * symbol.
     */
    public native int getOrientation();

    /** Retrieve components of a composite result. */
    public SymbolSet getComponents ()
    {
        return(new SymbolSet(getComponents(peer)));
    }

    private native long getComponents(long peer);

    native long next();
}
