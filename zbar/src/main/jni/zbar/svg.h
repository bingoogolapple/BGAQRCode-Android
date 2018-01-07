/*------------------------------------------------------------------------
 *  Copyright 2009 (c) Jeff Brown <spadix@users.sourceforge.net>
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
#ifndef _SVG_H_
#define _SVG_H_

#ifdef DEBUG_SVG

typedef enum { SVG_REL, SVG_ABS } svg_absrel_t;

void svg_open(const char *name, double x, double y, double w, double h);
void svg_close(void);

void svg_commentf(const char *format, ...);
void svg_image(const char *name, double width, double height);

void svg_group_start(const char *cls, double rotate,
                     double scalex, double scaley,
                     double x, double y);
void svg_group_end(void);

void svg_path_start(const char *cls, double scale, double x, double y);
void svg_path_end(void);
void svg_path_close(void);
void svg_path_moveto(svg_absrel_t abs, double x, double y);
void svg_path_lineto(svg_absrel_t abs, double x, double y);

#else

# define svg_open(...)
# define svg_close(...)

# define svg_image(...)

# define svg_group_start(...)
# define svg_group_end(...)

# define svg_path_start(...)
# define svg_path_end(...)
# define svg_path_moveto(...)
# define svg_path_lineto(...)
# define svg_path_close(...)

#endif

#endif
