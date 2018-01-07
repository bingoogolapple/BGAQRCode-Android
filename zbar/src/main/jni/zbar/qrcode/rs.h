/*Copyright (C) 1991-1995  Henry Minsky (hqm@ua.com, hqm@ai.mit.edu)
  Copyright (C) 2008-2009  Timothy B. Terriberry (tterribe@xiph.org)
  You can redistribute this library and/or modify it under the terms of the
   GNU Lesser General Public License as published by the Free Software
   Foundation; either version 2.1 of the License, or (at your option) any later
   version.*/
#if !defined(_qrcode_rs_H)
# define _qrcode_rs_H (1)

/*This is one of 16 irreducible primitive polynomials of degree 8:
    x**8+x**4+x**3+x**2+1.
  Under such a polynomial, x (i.e., 0x02) is a generator of GF(2**8).
  The high order 1 bit is implicit.
  From~\cite{MD88}, Ch. 5, p. 275 by Patel.
  @BOOK{MD88,
    author="C. Dennis Mee and Eric D. Daniel",
    title="Video, Audio, and Instrumentation Recording",
    series="Magnetic Recording",
    volume=3,
    publisher="McGraw-Hill Education",
    address="Columbus, OH",
    month=Jun,
    year=1988
  }*/
#define QR_PPOLY (0x1D)

/*The index to start the generator polynomial from (0...254).*/
#define QR_M0 (0)

typedef struct rs_gf256 rs_gf256;

struct rs_gf256{
  /*A logarithm table in GF(2**8).*/
  unsigned char log[256];
  /*An exponential table in GF(2**8): exp[i] contains x^i reduced modulo the
     irreducible primitive polynomial used to define the field.
    The extra 256 entries are used to do arithmetic mod 255, since some extra
     table lookups are generally faster than doing the modulus.*/
  unsigned char exp[511];
};

/*Initialize discrete logarithm tables for GF(2**8) using a given primitive
   irreducible polynomial.*/
void rs_gf256_init(rs_gf256 *_gf,unsigned _ppoly);

/*Corrects a codeword with _ndata<256 bytes, of which the last _npar are parity
   bytes.
  Known locations of errors can be passed in the _erasures array.
  Twice as many (up to _npar) errors with a known location can be corrected
   compared to errors with an unknown location.
  Returns the number of errors corrected if successful, or a negative number if
   the message could not be corrected because too many errors were detected.*/
int rs_correct(const rs_gf256 *_gf,int _m0,unsigned char *_data,int _ndata,
 int _npar,const unsigned char *_erasures,int _nerasures);

/*Create an _npar-coefficient generator polynomial for a Reed-Solomon code with
   _npar<256 parity bytes.*/
void rs_compute_genpoly(const rs_gf256 *_gf,int _m0,
 unsigned char *_genpoly,int _npar);

/*Adds _npar<=_ndata parity bytes to an _ndata-_npar byte message.
  _data must contain room for _ndata<256 bytes.*/
void rs_encode(const rs_gf256 *_gf,unsigned char *_data,int _ndata,
 const unsigned char *_genpoly,int _npar);

#endif
