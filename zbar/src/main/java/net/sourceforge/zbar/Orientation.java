/*------------------------------------------------------------------------
 *  Orientation
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

/** Decoded symbol coarse orientation.
 */
public class Orientation
{
    /** Unable to determine orientation. */
    public static final int UNKNOWN = -1;
    /** Upright, read left to right. */
    public static final int UP = 0;
    /** sideways, read top to bottom */
    public static final int RIGHT = 1;
    /** upside-down, read right to left */
    public static final int DOWN = 2;
    /** sideways, read bottom to top */
    public static final int LEFT = 3;
}
