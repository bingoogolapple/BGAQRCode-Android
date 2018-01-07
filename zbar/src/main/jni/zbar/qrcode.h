/*Copyright (C) 2008-2009  Timothy B. Terriberry (tterribe@xiph.org)
  You can redistribute this library and/or modify it under the terms of the
   GNU Lesser General Public License as published by the Free Software
   Foundation; either version 2.1 of the License, or (at your option) any later
   version.*/
#ifndef _QRCODE_H_
#define _QRCODE_H_

#include <zbar.h>

typedef struct qr_reader qr_reader;

typedef int qr_point[2];
typedef struct qr_finder_line qr_finder_line;

/*The number of bits of subpel precision to store image coordinates in.
  This helps when estimating positions in low-resolution images, which may have
   a module pitch only a pixel or two wide, making rounding errors matter a
   great deal.*/
#define QR_FINDER_SUBPREC (2)

/*A line crossing a finder pattern.
  Whether the line is horizontal or vertical is determined by context.
  The offsts to various parts of the finder pattern are as follows:
    |*****|     |*****|*****|*****|     |*****|
    |*****|     |*****|*****|*****|     |*****|
       ^        ^                 ^        ^
       |        |                 |        |
       |        |                 |       pos[v]+len+eoffs
       |        |                pos[v]+len
       |       pos[v]
      pos[v]-boffs
  Here v is 0 for horizontal and 1 for vertical lines.*/
struct qr_finder_line {
  /*The location of the upper/left endpoint of the line.
    The left/upper edge of the center section is used, since other lines must
     cross in this region.*/
  qr_point pos;
  /*The length of the center section.
    This extends to the right/bottom of the center section, since other lines
     must cross in this region.*/
  int      len;
  /*The offset to the midpoint of the upper/left section (part of the outside
     ring), or 0 if we couldn't identify the edge of the beginning section.
    We use the midpoint instead of the edge because it can be located more
     reliably.*/
  int      boffs;
  /*The offset to the midpoint of the end section (part of the outside ring),
     or 0 if we couldn't identify the edge of the end section.
    We use the midpoint instead of the edge because it can be located more
     reliably.*/
  int      eoffs;
};

qr_reader *_zbar_qr_create(void);
void _zbar_qr_destroy(qr_reader *reader);
void _zbar_qr_reset(qr_reader *reader);

int _zbar_qr_found_line(qr_reader *reader,
                        int direction,
                        const qr_finder_line *line);
int _zbar_qr_decode(qr_reader *reader,
                    zbar_image_scanner_t *iscn,
                    zbar_image_t *img);

#endif
