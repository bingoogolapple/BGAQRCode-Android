/*Copyright (C) 1991-1995  Henry Minsky (hqm@ua.com, hqm@ai.mit.edu)
  Copyright (C) 2008-2009  Timothy B. Terriberry (tterribe@xiph.org)
  You can redistribute this library and/or modify it under the terms of the
   GNU Lesser General Public License as published by the Free Software
   Foundation; either version 2.1 of the License, or (at your option) any later
   version.*/
#include <stdlib.h>
#include <string.h>
#include "rs.h"

/*Reed-Solomon encoder and decoder.
  Original implementation (C) Henry Minsky (hqm@ua.com, hqm@ai.mit.edu),
   Universal Access 1991-1995.
  Updates by Timothy B. Terriberry (C) 2008-2009:
   - Properly reject codes when error-locator polynomial has repeated roots or
      non-trivial irreducible factors.
   - Removed the hard-coded parity size and performed general API cleanup.
   - Allow multiple representations of GF(2**8), since different standards use
      different irreducible polynomials.
   - Allow different starting indices for the generator polynomial, since
      different standards use different values.
   - Greatly reduced the computation by eliminating unnecessary operations.
   - Explicitly solve for the roots of low-degree polynomials instead of using
      an exhaustive search.
     This is another major speed boost when there are few errors.*/


/*Galois Field arithmetic in GF(2**8).*/

void rs_gf256_init(rs_gf256 *_gf,unsigned _ppoly){
  unsigned p;
  int      i;
  /*Initialize the table of powers of a primtive root, alpha=0x02.*/
  p=1;
  for(i=0;i<256;i++){
    _gf->exp[i]=_gf->exp[i+255]=p;
    p=((p<<1)^(-(p>>7)&_ppoly))&0xFF;
  }
  /*Invert the table to recover the logs.*/
  for(i=0;i<255;i++)_gf->log[_gf->exp[i]]=i;
  /*Note that we rely on the fact that _gf->log[0]=0 below.*/
  _gf->log[0]=0;
}

/*Multiplication in GF(2**8) using logarithms.*/
static unsigned rs_gmul(const rs_gf256 *_gf,unsigned _a,unsigned _b){
  return _a==0||_b==0?0:_gf->exp[_gf->log[_a]+_gf->log[_b]];
}

/*Division in GF(2**8) using logarithms.
  The result of division by zero is undefined.*/
static unsigned rs_gdiv(const rs_gf256 *_gf,unsigned _a,unsigned _b){
  return _a==0?0:_gf->exp[_gf->log[_a]+255-_gf->log[_b]];
}

/*Multiplication in GF(2**8) when one of the numbers is known to be non-zero
   (proven by representing it by its logarithm).*/
static unsigned rs_hgmul(const rs_gf256 *_gf,unsigned _a,unsigned _logb){
  return _a==0?0:_gf->exp[_gf->log[_a]+_logb];
}

/*Square root in GF(2**8) using logarithms.*/
static unsigned rs_gsqrt(const rs_gf256 *_gf,unsigned _a){
  unsigned loga;
  if(!_a)return 0;
  loga=_gf->log[_a];
  return _gf->exp[loga+(255&-(loga&1))>>1];
}

/*Polynomial root finding in GF(2**8).
  Each routine returns a list of the distinct roots (i.e., with duplicates
   removed).*/

/*Solve a quadratic equation x**2 + _b*x + _c in GF(2**8) using the method
   of~\cite{Wal99}.
  Returns the number of distinct roots.
  ARTICLE{Wal99,
    author="C. Wayne Walker",
    title="New Formulas for Solving Quadratic Equations over Certain Finite
     Fields",
    journal="{IEEE} Transactions on Information Theory",
    volume=45,
    number=1,
    pages="283--284",
    month=Jan,
    year=1999
  }*/
static int rs_quadratic_solve(const rs_gf256 *_gf,unsigned _b,unsigned _c,
 unsigned char _x[2]){
  unsigned b;
  unsigned logb;
  unsigned logb2;
  unsigned logb4;
  unsigned logb8;
  unsigned logb12;
  unsigned logb14;
  unsigned logc;
  unsigned logc2;
  unsigned logc4;
  unsigned c8;
  unsigned g3;
  unsigned z3;
  unsigned l3;
  unsigned c0;
  unsigned g2;
  unsigned l2;
  unsigned z2;
  int      inc;
  /*If _b is zero, all we need is a square root.*/
  if(!_b){
    _x[0]=rs_gsqrt(_gf,_c);
    return 1;
  }
  /*If _c is zero, 0 and _b are the roots.*/
  if(!_c){
    _x[0]=0;
    _x[1]=_b;
    return 2;
  }
  logb=_gf->log[_b];
  logc=_gf->log[_c];
  /*If _b lies in GF(2**4), scale x to move it out.*/
  inc=logb%(255/15)==0;
  if(inc){
    b=_gf->exp[logb+254];
    logb=_gf->log[b];
    _c=_gf->exp[logc+253];
    logc=_gf->log[_c];
  }
  else b=_b;
  logb2=_gf->log[_gf->exp[logb<<1]];
  logb4=_gf->log[_gf->exp[logb2<<1]];
  logb8=_gf->log[_gf->exp[logb4<<1]];
  logb12=_gf->log[_gf->exp[logb4+logb8]];
  logb14=_gf->log[_gf->exp[logb2+logb12]];
  logc2=_gf->log[_gf->exp[logc<<1]];
  logc4=_gf->log[_gf->exp[logc2<<1]];
  c8=_gf->exp[logc4<<1];
  g3=rs_hgmul(_gf,
   _gf->exp[logb14+logc]^_gf->exp[logb12+logc2]^_gf->exp[logb8+logc4]^c8,logb);
  /*If g3 doesn't lie in GF(2**4), then our roots lie in an extension field.
    Note that we rely on the fact that _gf->log[0]==0 here.*/
  if(_gf->log[g3]%(255/15)!=0)return 0;
  /*Construct the corresponding quadratic in GF(2**4):
    x**2 + x/alpha**(255/15) + l3/alpha**(2*(255/15))*/
  z3=rs_gdiv(_gf,g3,_gf->exp[logb8<<1]^b);
  l3=rs_hgmul(_gf,rs_gmul(_gf,z3,z3)^rs_hgmul(_gf,z3,logb)^_c,255-logb2);
  c0=rs_hgmul(_gf,l3,255-2*(255/15));
  /*Construct the corresponding quadratic in GF(2**2):
    x**2 + x/alpha**(255/3) + l2/alpha**(2*(255/3))*/
  g2=rs_hgmul(_gf,
   rs_hgmul(_gf,c0,255-2*(255/15))^rs_gmul(_gf,c0,c0),255-255/15);
  z2=rs_gdiv(_gf,g2,_gf->exp[255-(255/15)*4]^_gf->exp[255-(255/15)]);
  l2=rs_hgmul(_gf,
   rs_gmul(_gf,z2,z2)^rs_hgmul(_gf,z2,255-(255/15))^c0,2*(255/15));
  /*Back substitute to the solution in the original field.*/
  _x[0]=_gf->exp[_gf->log[z3^rs_hgmul(_gf,
   rs_hgmul(_gf,l2,255/3)^rs_hgmul(_gf,z2,255/15),logb)]+inc];
  _x[1]=_x[0]^_b;
  return 2;
}

/*Solve a cubic equation x**3 + _a*x**2 + _b*x + _c in GF(2**8).
  Returns the number of distinct roots.*/
static int rs_cubic_solve(const rs_gf256 *_gf,
 unsigned _a,unsigned _b,unsigned _c,unsigned char _x[3]){
  unsigned k;
  unsigned logd;
  unsigned d2;
  unsigned logd2;
  unsigned logw;
  int      nroots;
  /*If _c is zero, factor out the 0 root.*/
  if(!_c){
    nroots=rs_quadratic_solve(_gf,_a,_b,_x);
    if(_b)_x[nroots++]=0;
    return nroots;
  }
  /*Substitute x=_a+y*sqrt(_a**2+_b) to get y**3 + y + k == 0,
     k = (_a*_b+c)/(_a**2+b)**(3/2).*/
  k=rs_gmul(_gf,_a,_b)^_c;
  d2=rs_gmul(_gf,_a,_a)^_b;
  if(!d2){
    int logx;
    if(!k){
      /*We have a triple root.*/
      _x[0]=_a;
      return 1;
    }
    logx=_gf->log[k];
    if(logx%3!=0)return 0;
    logx/=3;
    _x[0]=_a^_gf->exp[logx];
    _x[1]=_a^_gf->exp[logx+255/3];
    _x[2]=_a^_x[0]^_x[1];
    return 3;
  }
  logd2=_gf->log[d2];
  logd=logd2+(255&-(logd2&1))>>1;
  k=rs_gdiv(_gf,k,_gf->exp[logd+logd2]);
  /*Substitute y=w+1/w and z=w**3 to get z**2 + k*z + 1 == 0.*/
  nroots=rs_quadratic_solve(_gf,k,1,_x);
  if(nroots<1){
    /*The Reed-Solomon code is only valid if we can find 3 distinct roots in
       GF(2**8), so if we know there's only one, we don't actually need to find
       it.
      Note that we're also called by the quartic solver, but if we contain a
       non-trivial irreducible factor, than so does the original
       quartic~\cite{LW72}, and failing to return a root here actually saves us
       some work there, also.*/
    return 0;
  }
  /*Recover w from z.*/
  logw=_gf->log[_x[0]];
  if(logw){
    if(logw%3!=0)return 0;
    logw/=3;
    /*Recover x from w.*/
    _x[0]=_gf->exp[_gf->log[_gf->exp[logw]^_gf->exp[255-logw]]+logd]^_a;
    logw+=255/3;
    _x[1]=_gf->exp[_gf->log[_gf->exp[logw]^_gf->exp[255-logw]]+logd]^_a;
    _x[2]=_x[0]^_x[1]^_a;
    return 3;
  }
  else{
    _x[0]=_a;
    /*In this case _x[1] is a double root, so we know the Reed-Solomon code is
       invalid.
      Note that we still have to return at least one root, because if we're
       being called by the quartic solver, the quartic might still have 4
       distinct roots.
      But we don't need more than one root, so we can avoid computing the
       expensive one.*/
    /*_x[1]=_gf->exp[_gf->log[_gf->exp[255/3]^_gf->exp[2*(255/3)]]+logd]^_a;*/
    return 1;
  }
}

/*Solve a quartic equation x**4 + _a*x**3 + _b*x**2 + _c*x + _d in GF(2**8) by
   decomposing it into the cases given by~\cite{LW72}.
  Returns the number of distinct roots.
  @ARTICLE{LW72,
    author="Philip A. Leonard and Kenneth S. Williams",
    title="Quartics over $GF(2^n)$",
    journal="Proceedings of the American Mathematical Society",
    volume=36,
    number=2,
    pages="347--450",
    month=Dec,
    year=1972
  }*/
static int rs_quartic_solve(const rs_gf256 *_gf,
 unsigned _a,unsigned _b,unsigned _c,unsigned _d,unsigned char _x[3]){
  unsigned r;
  unsigned s;
  unsigned t;
  unsigned b;
  int      nroots;
  int      i;
  /*If _d is zero, factor out the 0 root.*/
  if(!_d){
    nroots=rs_cubic_solve(_gf,_a,_b,_c,_x);
    if(_c)_x[nroots++]=0;
    return nroots;
  }
  if(_a){
    unsigned loga;
    /*Substitute x=(1/y) + sqrt(_c/_a) to eliminate the cubic term.*/
    loga=_gf->log[_a];
    r=rs_hgmul(_gf,_c,255-loga);
    s=rs_gsqrt(_gf,r);
    t=_d^rs_gmul(_gf,_b,r)^rs_gmul(_gf,r,r);
    if(t){
      unsigned logti;
      logti=255-_gf->log[t];
      /*The result is still quartic, but with no cubic term.*/
      nroots=rs_quartic_solve(_gf,0,rs_hgmul(_gf,_b^rs_hgmul(_gf,s,loga),logti),
       _gf->exp[loga+logti],_gf->exp[logti],_x);
      for(i=0;i<nroots;i++)_x[i]=_gf->exp[255-_gf->log[_x[i]]]^s;
    }
    else{
      /*s must be a root~\cite{LW72}, and is in fact a double-root~\cite{CCO69}.
        Thus we're left with only a quadratic to solve.
        @ARTICLE{CCO69,
          author="Robert T. Chien and B. D. Cunningham and I. B. Oldham",
          title="Hybrid Methods for Finding Roots of a Polynomial---With
           Applications to {BCH} Decoding",
          journal="{IEEE} Transactions on Information Theory",
          volume=15,
          number=2,
          pages="329--335",
          month=Mar,
          year=1969
        }*/
      nroots=rs_quadratic_solve(_gf,_a,_b^r,_x);
      /*s may be a triple root if s=_b/_a, but not quadruple, since _a!=0.*/
      if(nroots!=2||_x[0]!=s&&_x[1]!=s)_x[nroots++]=s;
    }
    return nroots;
  }
  /*If there are no odd powers, it's really just a quadratic in disguise.*/
  if(!_c)return rs_quadratic_solve(_gf,rs_gsqrt(_gf,_b),rs_gsqrt(_gf,_d),_x);
  /*Factor into (x**2 + r*x + s)*(x**2 + r*x + t) by solving for r, which can
     be shown to satisfy r**3 + _b*r + _c == 0.*/
  nroots=rs_cubic_solve(_gf,0,_b,_c,_x);
  if(nroots<1){
    /*The Reed-Solomon code is only valid if we can find 4 distinct roots in
       GF(2**8).
      If the cubic does not factor into 3 (possibly duplicate) roots, then we
       know that the quartic must have a non-trivial irreducible factor.*/
    return 0;
  }
  r=_x[0];
  /*Now solve for s and t.*/
  b=rs_gdiv(_gf,_c,r);
  nroots=rs_quadratic_solve(_gf,b,_d,_x);
  if(nroots<2)return 0;
  s=_x[0];
  t=_x[1];
  /*_c=r*(s^t) was non-zero, so s and t must be distinct.
    But if z is a root of z**2 ^ r*z ^ s, then so is (z^r), and s = z*(z^r).
    Hence if z is also a root of z**2 ^ r*z ^ t, then t = s, a contradiction.
    Thus all four roots are distinct, if they exist.*/
  nroots=rs_quadratic_solve(_gf,r,s,_x);
  return nroots+rs_quadratic_solve(_gf,r,t,_x+nroots);
}

/*Polynomial arithmetic with coefficients in GF(2**8).*/

static void rs_poly_zero(unsigned char *_p,int _dp1){
  memset(_p,0,_dp1*sizeof(*_p));
}

static void rs_poly_copy(unsigned char *_p,const unsigned char *_q,int _dp1){
  memcpy(_p,_q,_dp1*sizeof(*_p));
}

/*Multiply the polynomial by the free variable, x (shift the coefficients).
  The number of coefficients, _dp1, must be non-zero.*/
static void rs_poly_mul_x(unsigned char *_p,const unsigned char *_q,int _dp1){
  memmove(_p+1,_q,(_dp1-1)*sizeof(*_p));
  _p[0]=0;
}

/*Divide the polynomial by the free variable, x (shift the coefficients).
  The number of coefficients, _dp1, must be non-zero.*/
static void rs_poly_div_x(unsigned char *_p,const unsigned char *_q,int _dp1){
  memmove(_p,_q+1,(_dp1-1)*sizeof(*_p));
  _p[_dp1-1]=0;
}

/*Compute the first (d+1) coefficients of the product of a degree e and a
   degree f polynomial.*/
static void rs_poly_mult(const rs_gf256 *_gf,unsigned char *_p,int _dp1,
 const unsigned char *_q,int _ep1,const unsigned char *_r,int _fp1){
  int m;
  int i;
  rs_poly_zero(_p,_dp1);
  m=_ep1<_dp1?_ep1:_dp1;
  for(i=0;i<m;i++)if(_q[i]!=0){
    unsigned logqi;
    int      n;
    int      j;
    n=_dp1-i<_fp1?_dp1-i:_fp1;
    logqi=_gf->log[_q[i]];
    for(j=0;j<n;j++)_p[i+j]^=rs_hgmul(_gf,_r[j],logqi);
  }
}

/*Decoding.*/

/*Computes the syndrome of a codeword.*/
static void rs_calc_syndrome(const rs_gf256 *_gf,int _m0,
 unsigned char *_s,int _npar,const unsigned char *_data,int _ndata){
  int i;
  int j;
  for(j=0;j<_npar;j++){
    unsigned alphaj;
    unsigned sj;
    sj=0;
    alphaj=_gf->log[_gf->exp[j+_m0]];
    for(i=0;i<_ndata;i++)sj=_data[i]^rs_hgmul(_gf,sj,alphaj);
    _s[j]=sj;
  }
}

/*Berlekamp-Peterson and Berlekamp-Massey Algorithms for error-location,
   modified to handle known erasures, from \cite{CC81}, p. 205.
  This finds the coefficients of the error locator polynomial.
  The roots are then found by looking for the values of alpha**n where
   evaluating the polynomial yields zero.
  Error correction is done using the error-evaluator equation on p. 207.
  @BOOK{CC81,
    author="George C. Clark, Jr and J. Bibb Cain",
    title="Error-Correction Coding for Digitial Communications",
    series="Applications of Communications Theory",
    publisher="Springer",
    address="New York, NY",
    month=Jun,
    year=1981
  }*/

/*Initialize lambda to the product of (1-x*alpha**e[i]) for erasure locations
   e[i].
  Note that the user passes in array indices counting from the beginning of the
   data, while our polynomial indexes starting from the end, so
   e[i]=(_ndata-1)-_erasures[i].*/
static void rs_init_lambda(const rs_gf256 *_gf,unsigned char *_lambda,int _npar,
 const unsigned char *_erasures,int _nerasures,int _ndata){
  int i;
  int j;
  rs_poly_zero(_lambda,(_npar<4?4:_npar)+1);
  _lambda[0]=1;
  for(i=0;i<_nerasures;i++)for(j=i+1;j>0;j--){
    _lambda[j]^=rs_hgmul(_gf,_lambda[j-1],_ndata-1-_erasures[i]);
  }
}

/*From \cite{CC81}, p. 216.
  Returns the number of errors detected (degree of _lambda).*/
static int rs_modified_berlekamp_massey(const rs_gf256 *_gf,
 unsigned char *_lambda,const unsigned char *_s,unsigned char *_omega,int _npar,
 const unsigned char *_erasures,int _nerasures,int _ndata){
  unsigned char tt[256];
  int           n;
  int           l;
  int           k;
  int           i;
  /*Initialize _lambda, the error locator-polynomial, with the location of
     known erasures.*/
  rs_init_lambda(_gf,_lambda,_npar,_erasures,_nerasures,_ndata);
  rs_poly_copy(tt,_lambda,_npar+1);
  l=_nerasures;
  k=0;
  for(n=_nerasures+1;n<=_npar;n++){
    unsigned d;
    rs_poly_mul_x(tt,tt,n-k+1);
    d=0;
    for(i=0;i<=l;i++)d^=rs_gmul(_gf,_lambda[i],_s[n-1-i]);
    if(d!=0){
      unsigned logd;
      logd=_gf->log[d];
      if(l<n-k){
        int t;
        for(i=0;i<=n-k;i++){
          unsigned tti;
          tti=tt[i];
          tt[i]=rs_hgmul(_gf,_lambda[i],255-logd);
          _lambda[i]=_lambda[i]^rs_hgmul(_gf,tti,logd);
        }
        t=n-k;
        k=n-l;
        l=t;
      }
      else for(i=0;i<=l;i++)_lambda[i]=_lambda[i]^rs_hgmul(_gf,tt[i],logd);
    }
  }
  rs_poly_mult(_gf,_omega,_npar,_lambda,l+1,_s,_npar);
  return l;
}

/*Finds all the roots of an error-locator polynomial _lambda by evaluating it
   at successive values of alpha, and returns the positions of the associated
   errors in _epos.
  Returns the number of valid roots identified.*/
static int rs_find_roots(const rs_gf256 *_gf,unsigned char *_epos,
 const unsigned char *_lambda,int _nerrors,int _ndata){
  unsigned alpha;
  int      nroots;
  int      i;
  nroots=0;
  if(_nerrors<=4){
    /*Explicit solutions for higher degrees are possible.
      Chien uses large lookup tables to solve quintics, and Truong et al. give
       special algorithms for degree up through 11, though they use exhaustive
       search (with reduced complexity) for some portions.
      Quartics are good enough for reading CDs, and represent a reasonable code
       complexity trade-off without requiring any extra tables.
      Note that _lambda[0] is always 1.*/
    _nerrors=rs_quartic_solve(_gf,_lambda[1],_lambda[2],_lambda[3],_lambda[4],
     _epos);
    for(i=0;i<_nerrors;i++)if(_epos[i]){
      alpha=_gf->log[_epos[i]];
      if((int)alpha<_ndata)_epos[nroots++]=alpha;
    }
    return nroots;
  }
  else for(alpha=0;(int)alpha<_ndata;alpha++){
    unsigned alphai;
    unsigned sum;
    sum=0;
    alphai=0;
    for(i=0;i<=_nerrors;i++){
      sum^=rs_hgmul(_gf,_lambda[_nerrors-i],alphai);
      alphai=_gf->log[_gf->exp[alphai+alpha]];
    }
    if(!sum)_epos[nroots++]=alpha;
  }
  return nroots;
}

/*Corrects a codeword with _ndata<256 bytes, of which the last _npar are parity
   bytes.
  Known locations of errors can be passed in the _erasures array.
  Twice as many (up to _npar) errors with a known location can be corrected
   compared to errors with an unknown location.
  Returns the number of errors corrected if successful, or a negative number if
   the message could not be corrected because too many errors were detected.*/
int rs_correct(const rs_gf256 *_gf,int _m0,unsigned char *_data,int _ndata,
 int _npar,const unsigned char *_erasures,int _nerasures){
  /*lambda must have storage for at least five entries to avoid special cases
     in the low-degree polynomial solver.*/
  unsigned char lambda[256];
  unsigned char omega[256];
  unsigned char epos[256];
  unsigned char s[256];
  int           i;
  /*If we already have too many erasures, we can't possibly succeed.*/
  if(_nerasures>_npar)return -1;
  /*Compute the syndrome values.*/
  rs_calc_syndrome(_gf,_m0,s,_npar,_data,_ndata);
  /*Check for a non-zero value.*/
  for(i=0;i<_npar;i++)if(s[i]){
    int nerrors;
    int j;
    /*Construct the error locator polynomial.*/
    nerrors=rs_modified_berlekamp_massey(_gf,lambda,s,omega,_npar,
     _erasures,_nerasures,_ndata);
    /*If we can't locate any errors, we can't force the syndrome values to
       zero, and must have a decoding error.
      Conversely, if we have too many errors, there's no reason to even attempt
       the root search.*/
    if(nerrors<=0||nerrors-_nerasures>_npar-_nerasures>>1)return -1;
    /*Compute the locations of the errors.
      If they are not all distinct, or some of them were outside the valid
       range for our block size, we have a decoding error.*/
    if(rs_find_roots(_gf,epos,lambda,nerrors,_ndata)<nerrors)return -1;
    /*Now compute the error magnitudes.*/
    for(i=0;i<nerrors;i++){
      unsigned a;
      unsigned b;
      unsigned alpha;
      unsigned alphan1;
      unsigned alphan2;
      unsigned alphanj;
      alpha=epos[i];
      /*Evaluate omega at alpha**-1.*/
      a=0;
      alphan1=255-alpha;
      alphanj=0;
      for(j=0;j<_npar;j++){
        a^=rs_hgmul(_gf,omega[j],alphanj);
        alphanj=_gf->log[_gf->exp[alphanj+alphan1]];
      }
      /*Evaluate the derivative of lambda at alpha**-1
        All the odd powers vanish.*/
      b=0;
      alphan2=_gf->log[_gf->exp[alphan1<<1]];
      alphanj=alphan1+_m0*alpha%255;
      for(j=1;j<=_npar;j+=2){
        b^=rs_hgmul(_gf,lambda[j],alphanj);
        alphanj=_gf->log[_gf->exp[alphanj+alphan2]];
      }
      /*Apply the correction.*/
      _data[_ndata-1-alpha]^=rs_gdiv(_gf,a,b);
    }
    return nerrors;
  }
  return 0;
}

/*Encoding.*/

/*Create an _npar-coefficient generator polynomial for a Reed-Solomon code
   with _npar<256 parity bytes.*/
void rs_compute_genpoly(const rs_gf256 *_gf,int _m0,
 unsigned char *_genpoly,int _npar){
  int i;
  if(_npar<=0)return;
  rs_poly_zero(_genpoly,_npar);
  _genpoly[0]=1;
  /*Multiply by (x+alpha^i) for i = 1 ... _ndata.*/
  for(i=0;i<_npar;i++){
    unsigned alphai;
    int      n;
    int      j;
    n=i+1<_npar-1?i+1:_npar-1;
    alphai=_gf->log[_gf->exp[_m0+i]];
    for(j=n;j>0;j--)_genpoly[j]=_genpoly[j-1]^rs_hgmul(_gf,_genpoly[j],alphai);
    _genpoly[0]=rs_hgmul(_gf,_genpoly[0],alphai);
  }
}

/*Adds _npar<=_ndata parity bytes to an _ndata-_npar byte message.
  _data must contain room for _ndata<256 bytes.*/
void rs_encode(const rs_gf256 *_gf,unsigned char *_data,int _ndata,
 const unsigned char *_genpoly,int _npar){
  unsigned char *lfsr;
  unsigned       d;
  int            i;
  int            j;
  if(_npar<=0)return;
  lfsr=_data+_ndata-_npar;
  rs_poly_zero(lfsr,_npar);
  for(i=0;i<_ndata-_npar;i++){
    d=_data[i]^lfsr[0];
    if(d){
      unsigned logd;
      logd=_gf->log[d];
      for(j=0;j<_npar-1;j++){
        lfsr[j]=lfsr[j+1]^rs_hgmul(_gf,_genpoly[_npar-1-j],logd);
      }
      lfsr[_npar-1]=rs_hgmul(_gf,_genpoly[0],logd);
    }
    else rs_poly_div_x(lfsr,lfsr,_npar);
  }
}

#if defined(RS_TEST_ENC)
#include <stdio.h>
#include <stdlib.h>

int main(void){
  rs_gf256 gf;
  int      k;
  rs_gf256_init(&gf,QR_PPOLY);
  srand(0);
  for(k=0;k<64*1024;k++){
    unsigned char genpoly[256];
    unsigned char data[256];
    unsigned char epos[256];
    int           ndata;
    int           npar;
    int           nerrors;
    int           i;
    ndata=rand()&0xFF;
    npar=ndata>0?rand()%ndata:0;
    for(i=0;i<ndata-npar;i++)data[i]=rand()&0xFF;
    rs_compute_genpoly(&gf,QR_M0,genpoly,npar);
    rs_encode(&gf,QR_M0,data,ndata,genpoly,npar);
    /*Write a clean version of the codeword.*/
    printf("%i %i",ndata,npar);
    for(i=0;i<ndata;i++)printf(" %i",data[i]);
    printf(" 0\n");
    /*Write the correct output to compare the decoder against.*/
    fprintf(stderr,"Success!\n",nerrors);
    for(i=0;i<ndata;i++)fprintf(stderr,"%i%s",data[i],i+1<ndata?" ":"\n");
    if(npar>0){
      /*Corrupt it.*/
      nerrors=rand()%(npar+1);
      if(nerrors>0){
        /*This test is not quite correct: there could be so many errors it
           comes within (npar>>1) errors of another valid codeword.
          I don't know a simple way to test for that without trying to decode
           the corrupt codeword, though, which is the very code we're trying to
           test.*/
        if(nerrors<=npar>>1){
          fprintf(stderr,"Success!\n",nerrors);
          for(i=0;i<ndata;i++){
            fprintf(stderr,"%i%s",data[i],i+1<ndata?" ":"\n");
          }
        }
        else fprintf(stderr,"Failure.\n");
        fprintf(stderr,"Success!\n",nerrors);
        for(i=0;i<ndata;i++)fprintf(stderr,"%i%s",data[i],i+1<ndata?" ":"\n");
        for(i=0;i<ndata;i++)epos[i]=i;
        for(i=0;i<nerrors;i++){
          unsigned char e;
          int           ei;
          ei=rand()%(ndata-i)+i;
          e=epos[ei];
          epos[ei]=epos[i];
          epos[i]=e;
          data[e]^=rand()%255+1;
        }
        /*First with no erasure locations.*/
        printf("%i %i",ndata,npar);
        for(i=0;i<ndata;i++)printf(" %i",data[i]);
        printf(" 0\n");
        /*Now with erasure locations.*/
        printf("%i %i",ndata,npar);
        for(i=0;i<ndata;i++)printf(" %i",data[i]);
        printf(" %i",nerrors);
        for(i=0;i<nerrors;i++)printf(" %i",epos[i]);
        printf("\n");
      }
    }
  }
  return 0;
}
#endif

#if defined(RS_TEST_DEC)
#include <stdio.h>

int main(void){
  rs_gf256 gf;
  rs_gf256_init(&gf,QR_PPOLY);
  for(;;){
    unsigned char data[255];
    unsigned char erasures[255];
    int idata[255];
    int ierasures[255];
    int ndata;
    int npar;
    int nerasures;
    int nerrors;
    int i;
    if(scanf("%i",&ndata)<1||ndata<0||ndata>255||
     scanf("%i",&npar)<1||npar<0||npar>ndata)break;
    for(i=0;i<ndata;i++){
      if(scanf("%i",idata+i)<1||idata[i]<0||idata[i]>255)break;
      data[i]=idata[i];
    }
    if(i<ndata)break;
    if(scanf("%i",&nerasures)<1||nerasures<0||nerasures>ndata)break;
    for(i=0;i<nerasures;i++){
      if(scanf("%i",ierasures+i)<1||ierasures[i]<0||ierasures[i]>=ndata)break;
      erasures[i]=ierasures[i];
    }
    nerrors=rs_correct(&gf,QR_M0,data,ndata,npar,erasures,nerasures);
    if(nerrors>=0){
      unsigned char data2[255];
      unsigned char genpoly[255];
      for(i=0;i<ndata-npar;i++)data2[i]=data[i];
      rs_compute_genpoly(&gf,QR_M0,genpoly,npar);
      rs_encode(&gf,QR_M0,data2,ndata,genpoly,npar);
      for(i=ndata-npar;i<ndata;i++)if(data[i]!=data2[i]){
        printf("Abject failure! %i!=%i\n",data[i],data2[i]);
      }
      printf("Success!\n",nerrors);
      for(i=0;i<ndata;i++)printf("%i%s",data[i],i+1<ndata?" ":"\n");
    }
    else printf("Failure.\n");
  }
  return 0;
}
#endif

#if defined(RS_TEST_ROOTS)
#include <stdio.h>

/*Exhaustively test the root finder.*/
int main(void){
  rs_gf256 gf;
  int      a;
  int      b;
  int      c;
  int      d;
  rs_gf256_init(&gf,QR_PPOLY);
  for(a=0;a<256;a++)for(b=0;b<256;b++)for(c=0;c<256;c++)for(d=0;d<256;d++){
    unsigned char x[4];
    unsigned char r[4];
    unsigned      x2;
    unsigned      e[5];
    int           nroots;
    int           mroots;
    int           i;
    int           j;
    nroots=rs_quartic_solve(&gf,a,b,c,d,x);
    for(i=0;i<nroots;i++){
      x2=rs_gmul(&gf,x[i],x[i]);
      e[0]=rs_gmul(&gf,x2,x2)^rs_gmul(&gf,a,rs_gmul(&gf,x[i],x2))^
       rs_gmul(&gf,b,x2)^rs_gmul(&gf,c,x[i])^d;
      if(e[0]){
        printf("Invalid root: (0x%02X)**4 ^ 0x%02X*(0x%02X)**3 ^ "
         "0x%02X*(0x%02X)**2 ^ 0x%02X(0x%02X) ^ 0x%02X = 0x%02X\n",
         x[i],a,x[i],b,x[i],c,x[i],d,e[0]);
      }
      for(j=0;j<i;j++)if(x[i]==x[j]){
        printf("Repeated root %i=%i: (0x%02X)**4 ^ 0x%02X*(0x%02X)**3 ^ "
         "0x%02X*(0x%02X)**2 ^ 0x%02X(0x%02X) ^ 0x%02X = 0x%02X\n",
         i,j,x[i],a,x[i],b,x[i],c,x[i],d,e[0]);
      }
    }
    mroots=0;
    for(j=1;j<256;j++){
      int logx;
      int logx2;
      logx=gf.log[j];
      logx2=gf.log[gf.exp[logx<<1]];
      e[mroots]=d^rs_hgmul(&gf,c,logx)^rs_hgmul(&gf,b,logx2)^
       rs_hgmul(&gf,a,gf.log[gf.exp[logx+logx2]])^gf.exp[logx2<<1];
      if(!e[mroots])r[mroots++]=j;
    }
    /*We only care about missing roots if the quartic had 4 distinct, non-zero
       roots.*/
    if(mroots==4)for(j=0;j<mroots;j++){
      for(i=0;i<nroots;i++)if(x[i]==r[j])break;
      if(i>=nroots){
        printf("Missing root: (0x%02X)**4 ^ 0x%02X*(0x%02X)**3 ^ "
         "0x%02X*(0x%02X)**2 ^ 0x%02X(0x%02X) ^ 0x%02X = 0x%02X\n",
         r[j],a,r[j],b,r[j],c,r[j],d,e[j]);
      }
    }
  }
  return 0;
}
#endif
