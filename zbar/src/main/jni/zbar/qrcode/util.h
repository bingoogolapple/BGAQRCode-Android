/*Copyright (C) 2008-2009  Timothy B. Terriberry (tterribe@xiph.org)
  You can redistribute this library and/or modify it under the terms of the
   GNU Lesser General Public License as published by the Free Software
   Foundation; either version 2.1 of the License, or (at your option) any later
   version.*/
#if !defined(_qrcode_util_H)
# define _qrcode_util_H (1)

#define QR_MAXI(_a,_b)      ((_a)-((_a)-(_b)&-((_b)>(_a))))
#define QR_MINI(_a,_b)      ((_a)+((_b)-(_a)&-((_b)<(_a))))
#define QR_SIGNI(_x)        (((_x)>0)-((_x)<0))
#define QR_SIGNMASK(_x)     (-((_x)<0))
/*Unlike copysign(), simply inverts the sign of _a if _b is negative.*/
#define QR_FLIPSIGNI(_a,_b) ((_a)+QR_SIGNMASK(_b)^QR_SIGNMASK(_b))
#define QR_COPYSIGNI(_a,_b) QR_FLIPSIGNI(abs(_a),_b)
/*Divides a signed integer by a positive value with exact rounding.*/
#define QR_DIVROUND(_x,_y)  (((_x)+QR_FLIPSIGNI(_y>>1,_x))/(_y))
#define QR_CLAMPI(_a,_b,_c) (QR_MAXI(_a,QR_MINI(_b,_c)))
#define QR_CLAMP255(_x)     ((unsigned char)((((_x)<0)-1)&((_x)|-((_x)>255))))
#define QR_SWAP2I(_a,_b) \
  do{ \
    int t__; \
    t__=(_a); \
    (_a)=(_b); \
    (_b)=t__; \
  } \
  while(0)
/*Swaps two integers _a and _b if _a>_b.*/
#define QR_SORT2I(_a,_b) \
  do{ \
    int t__; \
    t__=QR_MINI(_a,_b)^(_a); \
    (_a)^=t__; \
    (_b)^=t__; \
  } \
  while(0)
#define QR_ILOG0(_v) (!!((_v)&0x2))
#define QR_ILOG1(_v) (((_v)&0xC)?2+QR_ILOG0((_v)>>2):QR_ILOG0(_v))
#define QR_ILOG2(_v) (((_v)&0xF0)?4+QR_ILOG1((_v)>>4):QR_ILOG1(_v))
#define QR_ILOG3(_v) (((_v)&0xFF00)?8+QR_ILOG2((_v)>>8):QR_ILOG2(_v))
#define QR_ILOG4(_v) (((_v)&0xFFFF0000)?16+QR_ILOG3((_v)>>16):QR_ILOG3(_v))
/*Computes the integer logarithm of a (positive, 32-bit) constant.*/
#define QR_ILOG(_v) ((int)QR_ILOG4((unsigned)(_v)))

/*Multiplies 32-bit numbers _a and _b, adds (possibly 64-bit) number _r, and
   takes bits [_s,_s+31] of the result.*/
#define QR_FIXMUL(_a,_b,_r,_s) ((int)((_a)*(long long)(_b)+(_r)>>(_s)))
/*Multiplies 32-bit numbers _a and _b, adds (possibly 64-bit) number _r, and
   gives all 64 bits of the result.*/
#define QR_EXTMUL(_a,_b,_r)    ((_a)*(long long)(_b)+(_r))

unsigned qr_isqrt(unsigned _val);
unsigned qr_ihypot(int _x,int _y);
int qr_ilog(unsigned _val);

#endif
