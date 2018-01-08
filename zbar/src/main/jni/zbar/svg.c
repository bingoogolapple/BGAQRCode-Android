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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "svg.h"

static const char svg_head[] =
    "<?xml version='1.0'?>\n"
    "<!DOCTYPE svg PUBLIC '-//W3C//DTD SVG 1.1//EN'"
    " 'http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd'>\n"
    "<svg version='1.1' id='top' width='8in' height='8in'"
    " preserveAspectRatio='xMidYMid' overflow='visible'"
    " viewBox='%g,%g %g,%g' xmlns:xlink='http://www.w3.org/1999/xlink'"
    " xmlns='http://www.w3.org/2000/svg'>\n"
    "<defs><style type='text/css'><![CDATA[\n"
    "* { image-rendering: optimizeSpeed }\n"
    "image { opacity: .9 }\n"
    "path, line, circle { fill: none; stroke-width: .5;"
    " stroke-linejoin: round; stroke-linecap: butt;"
    " stroke-opacity: .666; fill-opacity: .666 }\n"
    /*"#hedge line, #vedge line { stroke: #34f }\n"*/
    "#hedge, #vedge { stroke: yellow }\n"
    "#target * { fill: none; stroke: #f24 }\n"
    "#mdot * { fill: #e2f; stroke: none }\n"
    "#ydot * { fill: yellow; stroke: none }\n"
    "#cross * { stroke: #44f }\n"
    ".hedge { marker: url(#hedge) }\n"
    ".vedge { marker: url(#vedge) }\n"
    ".scanner .hedge, .scanner .vedge { stroke-width: 8 }\n"
    ".finder .hedge, .finder .vedge { /*stroke: #a0c;*/ stroke-width: .9 }\n"
    ".cluster { stroke: #a0c; stroke-width: 1; stroke-opacity: .45 }\n"
    ".cluster.valid { stroke: #c0a; stroke-width: 1; stroke-opacity: .666 }\n"
    ".h.cluster { marker: url(#vedge) }\n"
    ".v.cluster { marker: url(#hedge) }\n"
    ".centers { marker: url(#target); stroke-width: 3 }\n"
    ".edge-pts { marker: url(#ydot); stroke-width: .5 }\n"
    ".align { marker: url(#mdot); stroke-width: 1.5 }\n"
    ".sampling-grid { stroke-width: .75; marker: url(#cross) }\n"
    "]]></style>\n"
    "<marker id='hedge' overflow='visible'><line x1='-2' x2='2'/></marker>\n"
    "<marker id='vedge' overflow='visible'><line y1='-2' y2='2'/></marker>\n"
    "<marker id='ydot' overflow='visible'><circle r='2'/></marker>\n"
    "<marker id='mdot' overflow='visible'><circle r='2'/></marker>\n"
    "<marker id='cross' overflow='visible'><path d='M-2,0h4 M0,-2v4'/></marker>\n"
    "<marker id='target' overflow='visible'><path d='M-4,0h8 M0,-4v8'/><circle r='2'/></marker>\n"
    "</defs>\n";

static FILE *svg = NULL;

void svg_open (const char *name, double x, double y, double w, double h)
{
    svg = fopen(name, "w");
    if(!svg) return;

    /* FIXME calc scaled size */
    fprintf(svg, svg_head, x, y, w, h);
}

void svg_close ()
{
    if(!svg) return;
    fprintf(svg, "</svg>\n");
    fclose(svg);
    svg = NULL;
}

void svg_commentf (const char *format, ...)
{
    if(!svg) return;
    fprintf(svg, "<!-- ");
    va_list args;
    va_start(args, format);
    vfprintf(svg, format, args);
    va_end(args);
    fprintf(svg, " -->\n");
}

void svg_image (const char *name, double width, double height)
{
    if(!svg) return;
    fprintf(svg, "<image width='%g' height='%g' xlink:href='%s'/>\n",
            width, height, name);
}

void svg_group_start (const char *cls,
                      double deg,
                      double sx,
                      double sy,
                      double x,
                      double y)
{
    if(!svg) return;
    fprintf(svg, "<g class='%s'", cls);
    if(sx != 1. || sy != 1 || deg || x || y) {
        fprintf(svg, " transform='");
        if(deg)
            fprintf(svg, "rotate(%g)", deg);
        if(x || y)
            fprintf(svg, "translate(%g,%g)", x, y);
        if(sx != 1. || sy != 1.) {
            if(!sy)
                fprintf(svg, "scale(%g)", sx);
            else
                fprintf(svg, "scale(%g,%g)", sx, sy);
        }
        fprintf(svg, "'");
    }
    fprintf(svg, ">\n");
}

void svg_group_end ()
{
    fprintf(svg, "</g>\n");
}

void svg_path_start (const char *cls,
                     double scale,
                     double x,
                     double y)
{
    if(!svg) return;
    fprintf(svg, "<path class='%s'", cls);
    if(scale != 1. || x || y) {
        fprintf(svg, " transform='");
        if(x || y)
            fprintf(svg, "translate(%g,%g)", x, y);
        if(scale != 1.)
            fprintf(svg, "scale(%g)", scale);
        fprintf(svg, "'");
    }
    fprintf(svg, " d='");
}

void svg_path_end ()
{
    if(!svg) return;
    fprintf(svg, "'/>\n");
}

void svg_path_close ()
{
    if(!svg) return;
    fprintf(svg, "z");
}

void svg_path_moveto (svg_absrel_t abs, double x, double y)
{
    if(!svg) return;
    fprintf(svg, " %c%g,%g", (abs) ? 'M' : 'm', x, y);
}

void svg_path_lineto(svg_absrel_t abs, double x, double y)
{
    if(!svg) return;
    if(x && y)
        fprintf(svg, "%c%g,%g", (abs) ? 'L' : 'l', x, y);
    else if(x)
        fprintf(svg, "%c%g", (abs) ? 'H' : 'h', x);
    else if(y)
        fprintf(svg, "%c%g", (abs) ? 'V' : 'v', y);
}
