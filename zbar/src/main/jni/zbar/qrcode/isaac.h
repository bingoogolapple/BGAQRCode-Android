/*Written by Timothy B. Terriberry (tterribe@xiph.org) 1999-2009 public domain.
  Based on the public domain implementation by Robert J. Jenkins Jr.*/
#if !defined(_isaac_H)
# define _isaac_H (1)



typedef struct isaac_ctx isaac_ctx;



#define ISAAC_SZ_LOG      (8)
#define ISAAC_SZ          (1<<ISAAC_SZ_LOG)
#define ISAAC_SEED_SZ_MAX (ISAAC_SZ<<2)



/*ISAAC is the most advanced of a series of Pseudo-Random Number Generators
   designed by Robert J. Jenkins Jr. in 1996.
  http://www.burtleburtle.net/bob/rand/isaac.html
  To quote:
    No efficient method is known for deducing their internal states.
    ISAAC requires an amortized 18.75 instructions to produce a 32-bit value.
    There are no cycles in ISAAC shorter than 2**40 values.
    The expected cycle length is 2**8295 values.*/
struct isaac_ctx{
  unsigned n;
  unsigned r[ISAAC_SZ];
  unsigned m[ISAAC_SZ];
  unsigned a;
  unsigned b;
  unsigned c;
};


void isaac_init(isaac_ctx *_ctx,const void *_seed,int _nseed);

unsigned isaac_next_uint32(isaac_ctx *_ctx);
unsigned isaac_next_uint(isaac_ctx *_ctx,unsigned _n);

#endif
