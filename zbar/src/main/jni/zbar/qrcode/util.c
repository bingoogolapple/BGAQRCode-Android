/*Copyright (C) 2008-2009  Timothy B. Terriberry (tterribe@xiph.org)
  You can redistribute this library and/or modify it under the terms of the
   GNU Lesser General Public License as published by the Free Software
   Foundation; either version 2.1 of the License, or (at your option) any later
   version.*/
#include <stdlib.h>
#include "util.h"

/*Computes floor(sqrt(_val)) exactly.*/
unsigned qr_isqrt(unsigned _val){
  unsigned g;
  unsigned b;
  int      bshift;
  /*Uses the second method from
     http://www.azillionmonkeys.com/qed/sqroot.html
    The main idea is to search for the largest binary digit b such that
     (g+b)*(g+b) <= _val, and add it to the solution g.*/
  g=0;
  b=0x8000;
  for(bshift=16;bshift-->0;){
    unsigned t;
    t=(g<<1)+b<<bshift;
    if(t<=_val){
      g+=b;
      _val-=t;
    }
    b>>=1;
  }
  return g;
}

/*Computes sqrt(_x*_x+_y*_y) using CORDIC.
  This implementation is valid for all 32-bit inputs and returns a result
   accurate to about 27 bits of precision.
  It has been tested for all postiive 16-bit inputs, where it returns correctly
   rounded results in 99.998% of cases and the maximum error is
   0.500137134862598032 (for _x=48140, _y=63018).
  Very nearly all results less than (1<<16) are correctly rounded.
  All Pythagorean triples with a hypotenuse of less than ((1<<27)-1) evaluate
   correctly, and the total bias over all Pythagorean triples is -0.04579, with
   a relative RMS error of 7.2864E-10 and a relative peak error of 7.4387E-9.*/
unsigned qr_ihypot(int _x,int _y){
  unsigned x;
  unsigned y;
  int      mask;
  int      shift;
  int      u;
  int      v;
  int      i;
  x=_x=abs(_x);
  y=_y=abs(_y);
  mask=-(x>y)&(_x^_y);
  x^=mask;
  y^=mask;
  _y^=mask;
  shift=31-qr_ilog(y);
  shift=QR_MAXI(shift,0);
  x=(unsigned)((x<<shift)*0x9B74EDAAULL>>32);
  _y=(int)((_y<<shift)*0x9B74EDA9LL>>32);
  u=x;
  mask=-(_y<0);
  x+=_y+mask^mask;
  _y-=u+mask^mask;
  u=x+1>>1;
  v=_y+1>>1;
  mask=-(_y<0);
  x+=v+mask^mask;
  _y-=u+mask^mask;
  for(i=1;i<16;i++){
    int r;
    u=x+1>>2;
    r=(1<<2*i)>>1;
    v=_y+r>>2*i;
    mask=-(_y<0);
    x+=v+mask^mask;
    _y=_y-(u+mask^mask)<<1;
  }
  return x+((1U<<shift)>>1)>>shift;
}

#if defined(__GNUC__) && defined(HAVE_FEATURES_H)
# include <features.h>
# if __GNUC_PREREQ(3,4)
#  include <limits.h>
#  if INT_MAX>=2147483647
#   define QR_CLZ0 sizeof(unsigned)*CHAR_BIT
#   define QR_CLZ(_x) (__builtin_clz(_x))
#  elif LONG_MAX>=2147483647L
#   define QR_CLZ0 sizeof(unsigned long)*CHAR_BIT
#   define QR_CLZ(_x) (__builtin_clzl(_x))
#  endif
# endif
#endif

int qr_ilog(unsigned _v){
#if defined(QR_CLZ)
/*Note that __builtin_clz is not defined when _x==0, according to the gcc
   documentation (and that of the x86 BSR instruction that implements it), so
   we have to special-case it.*/
  return QR_CLZ0-QR_CLZ(_v)&-!!_v;
#else
  int ret;
  int m;
  m=!!(_v&0xFFFF0000)<<4;
  _v>>=m;
  ret=m;
  m=!!(_v&0xFF00)<<3;
  _v>>=m;
  ret|=m;
  m=!!(_v&0xF0)<<2;
  _v>>=m;
  ret|=m;
  m=!!(_v&0xC)<<1;
  _v>>=m;
  ret|=m;
  ret|=!!(_v&0x2);
  return ret + !!_v;
#endif
}

#if defined(QR_TEST_SQRT)
#include <math.h>
#include <stdio.h>

/*Exhaustively test the integer square root function.*/
int main(void){
  unsigned u;
  u=0;
  do{
    unsigned r;
    unsigned s;
    r=qr_isqrt(u);
    s=(int)sqrt(u);
    if(r!=s)printf("%u: %u!=%u\n",u,r,s);
    u++;
  }
  while(u);
  return 0;
}
#endif
