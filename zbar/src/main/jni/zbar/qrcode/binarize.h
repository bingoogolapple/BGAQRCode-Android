/*Copyright (C) 2008-2009  Timothy B. Terriberry (tterribe@xiph.org)
  You can redistribute this library and/or modify it under the terms of the
   GNU Lesser General Public License as published by the Free Software
   Foundation; either version 2.1 of the License, or (at your option) any later
   version.*/
#if !defined(_qrcode_binarize_H)
# define _qrcode_binarize_H (1)

void qr_image_cross_masking_median_filter(unsigned char *_img,
 int _width,int _height);

void qr_wiener_filter(unsigned char *_img,int _width,int _height);

/*Binarizes a grayscale image.*/
unsigned char *qr_binarize(const unsigned char *_img,int _width,int _height);

#endif
