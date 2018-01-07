/*Copyright (C) 2008-2009  Timothy B. Terriberry (tterribe@xiph.org)
  You can redistribute this library and/or modify it under the terms of the
   GNU Lesser General Public License as published by the Free Software
   Foundation; either version 2.1 of the License, or (at your option) any later
   version.*/
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "util.h"
#include "image.h"
#include "binarize.h"

#if 0
/*Binarization based on~\cite{GPP06}.
  @ARTICLE{GPP06,
    author="Basilios Gatos and Ioannis E. Pratikakis and Stavros J. Perantonis",
    title="Adaptive Degraded Document Image Binarization",
    journal="Pattern Recognition",
    volume=39,
    number=3,
    pages="317-327",
    month=Mar,
    year=2006
  }*/

#if 0
/*Applies a 5x5 Wiener filter to the image, in-place, emphasizing differences
   where the local variance is small, and de-emphasizing them where it is
   large.*/
void qr_wiener_filter(unsigned char *_img,int _width,int _height){
  unsigned           *m_buf[8];
  unsigned           *sn2_buf[8];
  unsigned char       g;
  int                 x;
  int                 y;
  if(_width<=0||_height<=0)return;
  m_buf[0]=(unsigned *)malloc((_width+4<<3)*sizeof(*m_buf));
  sn2_buf[0]=(unsigned *)malloc((_width+4<<3)*sizeof(*sn2_buf));
  for(y=1;y<8;y++){
    m_buf[y]=m_buf[y-1]+_width+4;
    sn2_buf[y]=sn2_buf[y-1]+_width+4;
  }
  for(y=-4;y<_height;y++){
    unsigned *pm;
    unsigned *psn2;
    int       i;
    int       j;
    pm=m_buf[y+2&7];
    psn2=sn2_buf[y+2&7];
    for(x=-4;x<_width;x++){
      unsigned m;
      unsigned m2;
      m=m2=0;
      if(y>=0&&y<_height-4&&x>=0&&x<_width-4)for(i=0;i<5;i++)for(j=0;j<5;j++){
        g=_img[(y+i)*_width+x+j];
        m+=g;
        m2+=g*g;
      }
      else for(i=0;i<5;i++)for(j=0;j<5;j++){
        g=_img[QR_CLAMPI(0,y+i,_height-1)*_width+QR_CLAMPI(0,x+j,_width-1)];
        m+=g;
        m2+=g*g;
      }
      pm[x+4]=m;
      psn2[x+4]=(m2*25-m*m);
    }
    pm=m_buf[y&7];
    if(y>=0)for(x=0;x<_width;x++){
      int sn2;
      sn2=sn2_buf[y&7][x+2];
      if(sn2){
        int vn3;
        int m;
        /*Gatos et al. give the expression
            mu+(s2-v2)*(g-mu)/s2 ,
           which we reduce to
            mu+(s2-v2)*g/s2-(s2-v2)*mu/s2 ,
            g-(v2/s2)*g+(v2/s2)*mu ,
            g+(mu-g)*(v2/s2) .
           However, s2 is much noisier than v2, and dividing by it often gives
            extremely large adjustments, causing speckle near edges.
           Therefore we limit the ratio (v2/s2) to lie between 0 and 1.*/
        vn3=0;
        for(i=-2;i<3;i++){
          psn2=sn2_buf[y+i&7];
          for(j=0;j<5;j++)vn3+=psn2[x+j];
        }
        m=m_buf[y&7][x+2];
        vn3=vn3+1023>>10;
        sn2=25*sn2+1023>>10;
        if(vn3<sn2){
          int a;
          g=_img[y*_width+x];
          a=(m-25*g)*vn3;
          sn2*=25;
          _img[y*_width+x]=QR_CLAMP255(g+QR_DIVROUND(a,sn2));
        }
        else _img[y*_width+x]=(unsigned char)(((m<<1)+25)/50);
      }
    }
  }
  free(sn2_buf[0]);
  free(m_buf[0]);
}

#else
/*Applies a 3x3 Wiener filter to the image, in-place, emphasizing differences
   where the local variance is small, and de-emphasizing them where it is
   large.*/
void qr_wiener_filter(unsigned char *_img,int _width,int _height){
  unsigned           *m_buf[4];
  unsigned           *sn2_buf[4];
  unsigned char       g;
  int                 x;
  int                 y;
  if(_width<=0||_height<=0)return;
  m_buf[0]=(unsigned *)malloc((_width+2<<2)*sizeof(*m_buf));
  sn2_buf[0]=(unsigned *)malloc((_width+2<<2)*sizeof(*sn2_buf));
  for(y=1;y<4;y++){
    m_buf[y]=m_buf[y-1]+_width+2;
    sn2_buf[y]=sn2_buf[y-1]+_width+2;
  }
  for(y=-2;y<_height;y++){
    unsigned *pm;
    unsigned *psn2;
    int       i;
    int       j;
    pm=m_buf[y+1&3];
    psn2=sn2_buf[y+1&3];
    for(x=-2;x<_width;x++){
      unsigned m;
      unsigned m2;
      m=m2=0;
      if(y>=0&&y<_height-2&&x>=0&&x<_width-2)for(i=0;i<3;i++)for(j=0;j<3;j++){
        g=_img[(y+i)*_width+x+j];
        m+=g;
        m2+=g*g;
      }
      else for(i=0;i<3;i++)for(j=0;j<3;j++){
        g=_img[QR_CLAMPI(0,y+i,_height-1)*_width+QR_CLAMPI(0,x+j,_width-1)];
        m+=g;
        m2+=g*g;
      }
      pm[x+2]=m;
      psn2[x+2]=(m2*9-m*m);
    }
    pm=m_buf[y&3];
    if(y>=0)for(x=0;x<_width;x++){
      int sn2;
      sn2=sn2_buf[y&3][x+1];
      if(sn2){
        int m;
        int vn3;
        /*Gatos et al. give the expression
            mu+(s2-v2)*(g-mu)/s2 ,
           which we reduce to
            mu+(s2-v2)*g/s2-(s2-v2)*mu/s2 ,
            g-(v2/s2)*g+(v2/s2)*mu ,
            g+(mu-g)*(v2/s2) .
           However, s2 is much noisier than v2, and dividing by it often gives
            extremely large adjustments, causing speckle near edges.
           Therefore we limit the ratio (v2/s2) to lie between 0 and 1.*/
        vn3=0;
        for(i=-1;i<2;i++){
          psn2=sn2_buf[y+i&3];
          for(j=0;j<3;j++)vn3+=psn2[x+j];
        }
        m=m_buf[y&3][x+1];
        vn3=vn3+31>>5;
        sn2=9*sn2+31>>5;
        if(vn3<sn2){
          int a;
          g=_img[y*_width+x];
          a=m-9*g;
          sn2*=9;
          _img[y*_width+x]=QR_CLAMP255(g+QR_DIVROUND(a,sn2));
        }
        else _img[y*_width+x]=(unsigned char)(((m<<1)+9)/18);
      }
    }
  }
  free(sn2_buf[0]);
  free(m_buf[0]);
}
#endif

/*Computes a (conservative) foreground mask using the adaptive binarization
   threshold given in~\cite{SP00}, but knocking the threshold parameter down to
   k=0.2.
  Note on dynamic range: we assume _width*_height<=0x1000000 (24 bits).
  Returns the average background value.
  @ARTICLE{SP00,
    author="Jaakko J. Sauvola and Matti Pietik\"{a}inen",
    title="Adaptive Document Image Binarization",
    volume=33,
    number=2,
    pages="225--236",
    month=Feb,
    year=2000
  }*/
static void qr_sauvola_mask(unsigned char *_mask,unsigned *_b,int *_nb,
 const unsigned char *_img,int _width,int _height){
  unsigned  b;
  int       nb;
  b=0;
  nb=0;
  if(_width>0&&_height>0){
    unsigned *col_sums;
    unsigned *col2_sums;
    int       logwindw;
    int       logwindh;
    int       windw;
    int       windh;
    int       y0offs;
    int       y1offs;
    unsigned  g;
    unsigned  g2;
    int       x;
    int       y;
    /*We keep the window size fairly large to ensure it doesn't fit completely
       inside the center of a finder pattern of a version 1 QR code at full
       resolution.*/
    for(logwindw=4;logwindw<8&&(1<<logwindw)<(_width+7>>3);logwindw++);
    for(logwindh=4;logwindh<8&&(1<<logwindh)<(_height+7>>3);logwindh++);
    windw=1<<logwindw;
    windh=1<<logwindh;
    col_sums=(unsigned *)malloc(_width*sizeof(*col_sums));
    col2_sums=(unsigned *)malloc(_width*sizeof(*col2_sums));
    /*Initialize sums down each column.*/
    for(x=0;x<_width;x++){
      g=_img[x];
      g2=g*g;
      col_sums[x]=(g<<logwindh-1)+g;
      col2_sums[x]=(g2<<logwindh-1)+g2;
    }
    for(y=1;y<(windh>>1);y++){
      y1offs=QR_MINI(y,_height-1)*_width;
      for(x=0;x<_width;x++){
        g=_img[y1offs+x];
        col_sums[x]+=g;
        col2_sums[x]+=g*g;
      }
    }
    for(y=0;y<_height;y++){
      unsigned m;
      unsigned m2;
      int      x0;
      int      x1;
      /*Initialize the sums over the window.*/
      m=(col_sums[0]<<logwindw-1)+col_sums[0];
      m2=(col2_sums[0]<<logwindw-1)+col2_sums[0];
      for(x=1;x<(windw>>1);x++){
        x1=QR_MINI(x,_width-1);
        m+=col_sums[x1];
        m2+=col2_sums[x1];
      }
      for(x=0;x<_width;x++){
        int d;
        /*Perform the test against the threshold T = (m/n)*(1+k*(s/R-1)),
           where n=windw*windh, s=sqrt((m2-(m*m)/n)/n), and R=128.
          We don't actually compute the threshold directly, as that would
           require a square root.
          Instead we perform the equivalent test:
           (m/n)*(m/n)*(m2/n-(m/n)*(m/n))/16 > (((1/k)*g-((1-k)/k)*(m/n))*32)**2
          R is split up across each side of the inequality to maximize the
           dynamic range available for the right hand side, which requires
           31 bits in the worst case.*/
        /*(m/n)*(1+(1/5)*(sqrt((m2-m*m/n)/n)/128-1)) > g
          m*(1+(1/5)*(sqrt((m2-m*m/n)/n)/128-1)) > g*n
          m*sqrt((m2-m*m/n)/n) > 5*g*n-4*m<<7
          m*m*(m2*n-m*m) > (5*g*n-4*m<<7)**2*n*n || 5*g*n-4*m < 0 */
        g=_img[y*_width+x];
        d=(5*g<<logwindw+logwindh)-4*m;
        if(d>=0){
          unsigned mm;
          unsigned mms2;
          unsigned d2;
          mm=(m>>logwindw)*(m>>logwindh);
          mms2=(m2-mm>>logwindw+logwindh)*(mm>>logwindw+logwindh)+15>>4;
          d2=d>>logwindw+logwindh-5;
          d2*=d2;
          if(d2>=mms2){
            /*Update the background average.*/
            b+=g;
            nb++;
            _mask[y*_width+x]=0;
          }
          else _mask[y*_width+x]=0xFF;
        }
        else _mask[y*_width+x]=0xFF;
        /*Update the window sums.*/
        if(x+1<_width){
          x0=QR_MAXI(0,x-(windw>>1));
          x1=QR_MINI(x+(windw>>1),_width-1);
          m+=col_sums[x1]-col_sums[x0];
          m2+=col2_sums[x1]-col2_sums[x0];
        }
      }
      /*Update the column sums.*/
      if(y+1<_height){
        y0offs=QR_MAXI(0,y-(windh>>1))*_width;
        y1offs=QR_MINI(y+(windh>>1),_height-1)*_width;
        for(x=0;x<_width;x++){
          g=_img[y0offs+x];
          col_sums[x]-=g;
          col2_sums[x]-=g*g;
          g=_img[y1offs+x];
          col_sums[x]+=g;
          col2_sums[x]+=g*g;
        }
      }
    }
    free(col2_sums);
    free(col_sums);
  }
  *_b=b;
  *_nb=nb;
}

/*Interpolates a background image given the source and a conservative
   foreground mask.
  If the current window contains no foreground pixels, the average background
   value over the whole image is used.
  Note on dynamic range: we assume _width*_height<=0x8000000 (23 bits).
  Returns the average difference between the foreground and the interpolated
   background.*/
static void qr_interpolate_background(unsigned char *_dst,
 int *_delta,int *_ndelta,const unsigned char *_img,const unsigned char *_mask,
 int _width,int _height,unsigned _b,int _nb){
  int delta;
  int ndelta;
  delta=ndelta=0;
  if(_width>0&&_height>0){
    unsigned *col_sums;
    unsigned *ncol_sums;
    int       logwindw;
    int       logwindh;
    int       windw;
    int       windh;
    int       y0offs;
    int       y1offs;
    unsigned  b;
    unsigned  g;
    int       x;
    int       y;
    b=_nb>0?((_b<<1)+_nb)/(_nb<<1):0xFF;
    for(logwindw=4;logwindw<8&&(1<<logwindw)<(_width+15>>4);logwindw++);
    for(logwindh=4;logwindh<8&&(1<<logwindh)<(_height+15>>4);logwindh++);
    windw=1<<logwindw;
    windh=1<<logwindh;
    col_sums=(unsigned *)malloc(_width*sizeof(*col_sums));
    ncol_sums=(unsigned *)malloc(_width*sizeof(*ncol_sums));
    /*Initialize sums down each column.*/
    for(x=0;x<_width;x++){
      if(!_mask[x]){
        g=_img[x];
        col_sums[x]=(g<<logwindh-1)+g;
        ncol_sums[x]=(1<<logwindh-1)+1;
      }
      else col_sums[x]=ncol_sums[x]=0;
    }
    for(y=1;y<(windh>>1);y++){
      y1offs=QR_MINI(y,_height-1)*_width;
      for(x=0;x<_width;x++)if(!_mask[y1offs+x]){
        col_sums[x]+=_img[y1offs+x];
        ncol_sums[x]++;
      }
    }
    for(y=0;y<_height;y++){
      unsigned n;
      unsigned m;
      int      x0;
      int      x1;
      /*Initialize the sums over the window.*/
      m=(col_sums[0]<<logwindw-1)+col_sums[0];
      n=(ncol_sums[0]<<logwindw-1)+ncol_sums[0];
      for(x=1;x<(windw>>1);x++){
        x1=QR_MINI(x,_width-1);
        m+=col_sums[x1];
        n+=ncol_sums[x1];
      }
      for(x=0;x<_width;x++){
        if(!_mask[y*_width+x])g=_img[y*_width+x];
        else{
          g=n>0?((m<<1)+n)/(n<<1):b;
          delta+=(int)g-_img[y*_width+x];
          ndelta++;
        }
        _dst[y*_width+x]=(unsigned char)g;
        /*Update the window sums.*/
        if(x+1<_width){
          x0=QR_MAXI(0,x-(windw>>1));
          x1=QR_MINI(x+(windw>>1),_width-1);
          m+=col_sums[x1]-col_sums[x0];
          n+=ncol_sums[x1]-ncol_sums[x0];
        }
      }
      /*Update the column sums.*/
      if(y+1<_height){
        y0offs=QR_MAXI(0,y-(windh>>1))*_width;
        y1offs=QR_MINI(y+(windh>>1),_height-1)*_width;
        for(x=0;x<_width;x++){
          if(!_mask[y0offs+x]){
            col_sums[x]-=_img[y0offs+x];
            ncol_sums[x]--;
          }
          if(!_mask[y1offs+x]){
            col_sums[x]+=_img[y1offs+x];
            ncol_sums[x]++;
          }
        }
      }
    }
    free(ncol_sums);
    free(col_sums);
  }
  *_delta=delta;
  *_ndelta=ndelta;
}

/*Parameters of the logistic sigmoid function that defines the threshold based
   on the background intensity.
  They should all be between 0 and 1.*/
#define QR_GATOS_Q  (0.7)
#define QR_GATOS_P1 (0.5)
#define QR_GATOS_P2 (0.8)

/*Compute the final binarization mask according to Gatos et al.'s
   method~\cite{GPP06}.*/
static void qr_gatos_mask(unsigned char *_mask,const unsigned char *_img,
 const unsigned char *_background,int _width,int _height,
 unsigned _b,int _nb,int _delta,int _ndelta){
  unsigned thresh[256];
  unsigned g;
  double   delta;
  double   b;
  int      x;
  int      y;
  /*Construct a lookup table for the thresholds.
    This bit uses floating point, but doesn't need to do much calculation, so
     emulation should be fine.*/
  b=_nb>0?(_b+0.5)/_nb:0xFF;
  delta=_ndelta>0?(_delta+0.5)/_ndelta:0xFF;
  for(g=0;g<256;g++){
    double d;
    d=QR_GATOS_Q*delta*(QR_GATOS_P2+(1-QR_GATOS_P2)/
     (1+exp(2*(1+QR_GATOS_P1)/(1-QR_GATOS_P1)-4*g/(b*(1-QR_GATOS_P1)))));
    if(d<1)d=1;
    else if(d>0xFF)d=0xFF;
    thresh[g]=(unsigned)floor(d);
  }
  /*Apply the adaptive threshold.*/
  for(y=0;y<_height;y++)for(x=0;x<_width;x++){
    g=_background[y*_width+x];
    /*_background[y*_width+x]=thresh[g];*/
    _mask[y*_width+x]=(unsigned char)(-(g-_img[y*_width+x]>thresh[g])&0xFF);
  }
  /*{
    FILE *fout;
    fout=fopen("thresh.png","wb");
    image_write_png(_background,_width,_height,fout);
    fclose(fout);
  }*/
}

/*Binarizes a grayscale image.*/
void qr_binarize(unsigned char *_img,int _width,int _height){
  unsigned char *mask;
  unsigned char *background;
  unsigned       b;
  int            nb;
  int            delta;
  int            ndelta;
  /*qr_wiener_filter(_img,_width,_height);
  {
    FILE *fout;
    fout=fopen("wiener.png","wb");
    image_write_png(_img,_width,_height,fout);
    fclose(fout);
  }*/
  mask=(unsigned char *)malloc(_width*_height*sizeof(*mask));
  qr_sauvola_mask(mask,&b,&nb,_img,_width,_height);
  /*{
    FILE *fout;
    fout=fopen("foreground.png","wb");
    image_write_png(mask,_width,_height,fout);
    fclose(fout);
  }*/
  background=(unsigned char *)malloc(_width*_height*sizeof(*mask));
  qr_interpolate_background(background,&delta,&ndelta,
   _img,mask,_width,_height,b,nb);
  /*{
    FILE *fout;
    fout=fopen("background.png","wb");
    image_write_png(background,_width,_height,fout);
    fclose(fout);
  }*/
  qr_gatos_mask(_img,_img,background,_width,_height,b,nb,delta,ndelta);
  free(background);
  free(mask);
}

#else
/*The above algorithms are computationally expensive, and do not work as well
   as the simple algorithm below.
  Sauvola by itself does an excellent job of classifying regions outside the
   QR code as background, which greatly reduces the chance of false alarms.
  However, it also tends to over-shrink isolated black dots inside the code,
   making them easy to miss with even slight mis-alignment.
  Since the Gatos method uses Sauvola as input to its background interpolation
   method, it cannot possibly mark any pixels as foreground which Sauvola
   classified as background, and thus suffers from the same problem.
  The following simple adaptive threshold method does not have this problem,
   though it produces essentially random noise outside the QR code region.
  QR codes are structured well enough that this does not seem to lead to any
   actual false alarms in practice, and it allows many more codes to be
   detected and decoded successfully than the Sauvola or Gatos binarization
   methods.*/

/*A simplified adaptive thresholder.
  This compares the current pixel value to the mean value of a (large) window
   surrounding it.*/
unsigned char *qr_binarize(const unsigned char *_img,int _width,int _height){
  unsigned char *mask = NULL;
  if(_width>0&&_height>0){
    unsigned      *col_sums;
    int            logwindw;
    int            logwindh;
    int            windw;
    int            windh;
    int            y0offs;
    int            y1offs;
    unsigned       g;
    int            x;
    int            y;
    mask=(unsigned char *)malloc(_width*_height*sizeof(*mask));
    /*We keep the window size fairly large to ensure it doesn't fit completely
       inside the center of a finder pattern of a version 1 QR code at full
       resolution.*/
    for(logwindw=4;logwindw<8&&(1<<logwindw)<(_width+7>>3);logwindw++);
    for(logwindh=4;logwindh<8&&(1<<logwindh)<(_height+7>>3);logwindh++);
    windw=1<<logwindw;
    windh=1<<logwindh;
    col_sums=(unsigned *)malloc(_width*sizeof(*col_sums));
    /*Initialize sums down each column.*/
    for(x=0;x<_width;x++){
      g=_img[x];
      col_sums[x]=(g<<logwindh-1)+g;
    }
    for(y=1;y<(windh>>1);y++){
      y1offs=QR_MINI(y,_height-1)*_width;
      for(x=0;x<_width;x++){
        g=_img[y1offs+x];
        col_sums[x]+=g;
      }
    }
    for(y=0;y<_height;y++){
      unsigned m;
      int      x0;
      int      x1;
      /*Initialize the sum over the window.*/
      m=(col_sums[0]<<logwindw-1)+col_sums[0];
      for(x=1;x<(windw>>1);x++){
        x1=QR_MINI(x,_width-1);
        m+=col_sums[x1];
      }
      for(x=0;x<_width;x++){
        /*Perform the test against the threshold T = (m/n)-D,
           where n=windw*windh and D=3.*/
        g=_img[y*_width+x];
        mask[y*_width+x]=-(g+3<<logwindw+logwindh<m)&0xFF;
        /*Update the window sum.*/
        if(x+1<_width){
          x0=QR_MAXI(0,x-(windw>>1));
          x1=QR_MINI(x+(windw>>1),_width-1);
          m+=col_sums[x1]-col_sums[x0];
        }
      }
      /*Update the column sums.*/
      if(y+1<_height){
        y0offs=QR_MAXI(0,y-(windh>>1))*_width;
        y1offs=QR_MINI(y+(windh>>1),_height-1)*_width;
        for(x=0;x<_width;x++){
          col_sums[x]-=_img[y0offs+x];
          col_sums[x]+=_img[y1offs+x];
        }
      }
    }
    free(col_sums);
  }
#if defined(QR_DEBUG)
  {
    FILE *fout;
    fout=fopen("binary.png","wb");
    image_write_png(_img,_width,_height,fout);
    fclose(fout);
  }
#endif
  return(mask);
}
#endif

#if defined(TEST_BINARIZE)
#include <stdio.h>
#include "image.c"

int main(int _argc,char **_argv){
  unsigned char *img;
  int            width;
  int            height;
  int            x;
  int            y;
  if(_argc<2){
    fprintf(stderr,"usage: %s <image>.png\n",_argv[0]);
    return EXIT_FAILURE;
  }
  /*width=1182;
  height=1181;
  img=(unsigned char *)malloc(width*height*sizeof(*img));
  for(y=0;y<height;y++)for(x=0;x<width;x++){
    img[y*width+x]=(unsigned char)(-((x&1)^(y&1))&0xFF);
  }*/
  {
    FILE *fin;
    fin=fopen(_argv[1],"rb");
    image_read_png(&img,&width,&height,fin);
    fclose(fin);
  }
  qr_binarize(img,width,height);
  /*{
    FILE *fout;
    fout=fopen("binary.png","wb");
    image_write_png(img,width,height,fout);
    fclose(fout);
  }*/
  free(img);
  return EXIT_SUCCESS;
}
#endif
