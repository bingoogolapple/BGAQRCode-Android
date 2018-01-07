/*Copyright (C) 2008-2009  Timothy B. Terriberry (tterribe@xiph.org)
  You can redistribute this library and/or modify it under the terms of the
   GNU Lesser General Public License as published by the Free Software
   Foundation; either version 2.1 of the License, or (at your option) any later
   version.*/
#if !defined(_bch15_5_H)
# define _bch15_5_H (1)

/*Encodes a raw 5-bit value _x into a 15-bit BCH(15,5) code.
  This is capable of correcting up to 3 bit errors, and detecting as many as
   5 bit errors in some cases.*/
unsigned bch15_5_encode(unsigned _x);

/*Corrects the received code *_y, if possible.
  The original data is located in the top five bits.
  Returns the number of errors corrected, or a negative value if decoding
   failed due to too many bit errors, in which case *_y is left unchanged.*/
int bch15_5_correct(unsigned *_y);

#endif
