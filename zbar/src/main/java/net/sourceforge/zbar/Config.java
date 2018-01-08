/*------------------------------------------------------------------------
 *  Config
 *
 *  Copyright 2010 (c) Jeff Brown <spadix@users.sourceforge.net>
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

/** Decoder configuration options.
 */
public class Config
{
    /** Enable symbology/feature. */
    public static final int ENABLE = 0;
    /** Enable check digit when optional. */
    public static final int ADD_CHECK = 1;
    /** Return check digit when present. */
    public static final int EMIT_CHECK = 2;
    /** Enable full ASCII character set. */
    public static final int ASCII = 3;

    /** Minimum data length for valid decode. */
    public static final int MIN_LEN = 0x20;
    /** Maximum data length for valid decode. */
    public static final int MAX_LEN = 0x21;

    /** Required video consistency frames. */
    public static final int UNCERTAINTY = 0x40;

    /** Enable scanner to collect position data. */
    public static final int POSITION = 0x80;

    /** Image scanner vertical scan density. */
    public static final int X_DENSITY = 0x100;
    /** Image scanner horizontal scan density. */
    public static final int Y_DENSITY = 0x101;
}
