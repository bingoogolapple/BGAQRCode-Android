/*Written by Timothy B. Terriberry (tterribe@xiph.org) 1999-2009 public domain.
  Based on the public domain implementation by Robert J. Jenkins Jr.*/
#include <float.h>
#include <math.h>
#include <string.h>
#include "isaac.h"



#define ISAAC_MASK        (0xFFFFFFFFU)



static void isaac_update(isaac_ctx *_ctx){
  unsigned *m;
  unsigned *r;
  unsigned  a;
  unsigned  b;
  unsigned  x;
  unsigned  y;
  int       i;
  m=_ctx->m;
  r=_ctx->r;
  a=_ctx->a;
  b=_ctx->b+(++_ctx->c)&ISAAC_MASK;
  for(i=0;i<ISAAC_SZ/2;i++){
    x=m[i];
    a=(a^a<<13)+m[i+ISAAC_SZ/2]&ISAAC_MASK;
    m[i]=y=m[(x&ISAAC_SZ-1<<2)>>2]+a+b&ISAAC_MASK;
    r[i]=b=m[y>>ISAAC_SZ_LOG+2&ISAAC_SZ-1]+x&ISAAC_MASK;
    x=m[++i];
    a=(a^a>>6)+m[i+ISAAC_SZ/2]&ISAAC_MASK;
    m[i]=y=m[(x&ISAAC_SZ-1<<2)>>2]+a+b&ISAAC_MASK;
    r[i]=b=m[y>>ISAAC_SZ_LOG+2&ISAAC_SZ-1]+x&ISAAC_MASK;
    x=m[++i];
    a=(a^a<<2)+m[i+ISAAC_SZ/2]&ISAAC_MASK;
    m[i]=y=m[(x&ISAAC_SZ-1<<2)>>2]+a+b&ISAAC_MASK;
    r[i]=b=m[y>>ISAAC_SZ_LOG+2&ISAAC_SZ-1]+x&ISAAC_MASK;
    x=m[++i];
    a=(a^a>>16)+m[i+ISAAC_SZ/2]&ISAAC_MASK;
    m[i]=y=m[(x&ISAAC_SZ-1<<2)>>2]+a+b&ISAAC_MASK;
    r[i]=b=m[y>>ISAAC_SZ_LOG+2&ISAAC_SZ-1]+x&ISAAC_MASK;
  }
  for(i=ISAAC_SZ/2;i<ISAAC_SZ;i++){
    x=m[i];
    a=(a^a<<13)+m[i-ISAAC_SZ/2]&ISAAC_MASK;
    m[i]=y=m[(x&ISAAC_SZ-1<<2)>>2]+a+b&ISAAC_MASK;
    r[i]=b=m[y>>ISAAC_SZ_LOG+2&ISAAC_SZ-1]+x&ISAAC_MASK;
    x=m[++i];
    a=(a^a>>6)+m[i-ISAAC_SZ/2]&ISAAC_MASK;
    m[i]=y=m[(x&ISAAC_SZ-1<<2)>>2]+a+b&ISAAC_MASK;
    r[i]=b=m[y>>ISAAC_SZ_LOG+2&ISAAC_SZ-1]+x&ISAAC_MASK;
    x=m[++i];
    a=(a^a<<2)+m[i-ISAAC_SZ/2]&ISAAC_MASK;
    m[i]=y=m[(x&ISAAC_SZ-1<<2)>>2]+a+b&ISAAC_MASK;
    r[i]=b=m[y>>ISAAC_SZ_LOG+2&ISAAC_SZ-1]+x&ISAAC_MASK;
    x=m[++i];
    a=(a^a>>16)+m[i-ISAAC_SZ/2]&ISAAC_MASK;
    m[i]=y=m[(x&ISAAC_SZ-1<<2)>>2]+a+b&ISAAC_MASK;
    r[i]=b=m[y>>ISAAC_SZ_LOG+2&ISAAC_SZ-1]+x&ISAAC_MASK;
  }
  _ctx->b=b;
  _ctx->a=a;
  _ctx->n=ISAAC_SZ;
}

static void isaac_mix(unsigned _x[8]){
  static const unsigned char SHIFT[8]={11,2,8,16,10,4,8,9};
  int i;
  for(i=0;i<8;i++){
    _x[i]^=_x[i+1&7]<<SHIFT[i];
    _x[i+3&7]+=_x[i];
    _x[i+1&7]+=_x[i+2&7];
    i++;
    _x[i]^=_x[i+1&7]>>SHIFT[i];
    _x[i+3&7]+=_x[i];
    _x[i+1&7]+=_x[i+2&7];
  }
}


void isaac_init(isaac_ctx *_ctx,const void *_seed,int _nseed){
  const unsigned char *seed;
  unsigned            *m;
  unsigned            *r;
  unsigned             x[8];
  int                  i;
  int                  j;
  _ctx->a=_ctx->b=_ctx->c=0;
  m=_ctx->m;
  r=_ctx->r;
  x[0]=x[1]=x[2]=x[3]=x[4]=x[5]=x[6]=x[7]=0x9E3779B9;
  for(i=0;i<4;i++)isaac_mix(x);
  if(_nseed>ISAAC_SEED_SZ_MAX)_nseed=ISAAC_SEED_SZ_MAX;
  seed=(const unsigned char *)_seed;
  for(i=0;i<_nseed>>2;i++){
    r[i]=seed[i<<2|3]<<24|seed[i<<2|2]<<16|seed[i<<2|1]<<8|seed[i<<2];
  }
  if(_nseed&3){
    r[i]=seed[i<<2];
    for(j=1;j<(_nseed&3);j++)r[i]+=seed[i<<2|j]<<(j<<3);
    i++;
  }
  memset(r+i,0,(ISAAC_SZ-i)*sizeof(*r));
  for(i=0;i<ISAAC_SZ;i+=8){
    for(j=0;j<8;j++)x[j]+=r[i+j];
    isaac_mix(x);
    memcpy(m+i,x,sizeof(x));
  }
  for(i=0;i<ISAAC_SZ;i+=8){
    for(j=0;j<8;j++)x[j]+=m[i+j];
    isaac_mix(x);
    memcpy(m+i,x,sizeof(x));
  }
  isaac_update(_ctx);
}

unsigned isaac_next_uint32(isaac_ctx *_ctx){
  if(!_ctx->n)isaac_update(_ctx);
  return _ctx->r[--_ctx->n];
}

/*Returns a uniform random integer less than the given maximum value.
  _n: The upper bound on the range of numbers returned (not inclusive).
      This must be strictly less than 2**32.
  Return: An integer uniformly distributed between 0 (inclusive) and _n
           (exclusive).*/
unsigned isaac_next_uint(isaac_ctx *_ctx,unsigned _n){
  unsigned r;
  unsigned v;
  unsigned d;
  do{
    r=isaac_next_uint32(_ctx);
    v=r%_n;
    d=r-v;
  }
  while((d+_n-1&ISAAC_MASK)<d);
  return v;
}
