/*Copyright (C) 2008-2010  Timothy B. Terriberry (tterribe@xiph.org)
  You can redistribute this library and/or modify it under the terms of the
   GNU Lesser General Public License as published by the Free Software
   Foundation; either version 2.1 of the License, or (at your option) any later
   version.*/
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include "qrcode.h"
#include "qrdec.h"
#include "util.h"
#include "image.h"
#include "decoder.h"
#include "error.h"
#include "img_scanner.h"

static int text_is_ascii(const unsigned char *_text,int _len){
  int i;
  for(i=0;i<_len;i++)if(_text[i]>=0x80)return 0;
  return 1;
}

static int text_is_latin1(const unsigned char *_text,int _len){
  int i;
  for(i=0;i<_len;i++){
    /*The following line fails to compile correctly with gcc 3.4.4 on ARM with
       any optimizations enabled.*/
    if(_text[i]>=0x80&&_text[i]<0xA0)return 0;
  }
  return 1;
}

static void enc_list_mtf(iconv_t _enc_list[3],iconv_t _enc){
  int i;
  for(i=0;i<3;i++)if(_enc_list[i]==_enc){
    int j;
    for(j=i;j-->0;)_enc_list[j+1]=_enc_list[j];
    _enc_list[0]=_enc;
    break;
  }
}

int qr_code_data_list_extract_text(const qr_code_data_list *_qrlist,
                                   zbar_image_scanner_t *iscn,
                                   zbar_image_t *img)
{
  iconv_t              sjis_cd;
  iconv_t              utf8_cd;
  iconv_t              latin1_cd;
  const qr_code_data  *qrdata;
  int                  nqrdata;
  unsigned char       *mark;
  int                  ntext;
  int                  i;
  qrdata=_qrlist->qrdata;
  nqrdata=_qrlist->nqrdata;
  mark=(unsigned char *)calloc(nqrdata,sizeof(*mark));
  ntext=0;
  /*This is the encoding the standard says is the default.*/
  latin1_cd=iconv_open("UTF-8","ISO8859-1");
  /*But this one is often used, as well.*/
  sjis_cd=iconv_open("UTF-8","SJIS");
  /*This is a trivial conversion just to check validity without extra code.*/
  utf8_cd=iconv_open("UTF-8","UTF-8");
  for(i=0;i<nqrdata;i++)if(!mark[i]){
    const qr_code_data       *qrdataj;
    const qr_code_data_entry *entry;
    iconv_t                   enc_list[3];
    iconv_t                   eci_cd;
    int                       sa[16];
    int                       sa_size;
    char                     *sa_text;
    size_t                    sa_ntext;
    size_t                    sa_ctext;
    int                       fnc1;
    int                       fnc1_2ai;
    int                       has_kanji;
    int                       eci;
    int                       err;
    int                       j;
    int                       k;
    zbar_symbol_t *syms = NULL, **sym = &syms;
    qr_point dir;
    int horiz;

    /*Step 0: Collect the other QR codes belonging to this S-A group.*/
    if(qrdata[i].sa_size){
      unsigned sa_parity;
      sa_size=qrdata[i].sa_size;
      sa_parity=qrdata[i].sa_parity;
      for(j=0;j<sa_size;j++)sa[j]=-1;
      for(j=i;j<nqrdata;j++)if(!mark[j]){
        /*TODO: We could also match version, ECC level, etc. if size and
           parity alone are too ambiguous.*/
        if(qrdata[j].sa_size==sa_size&&qrdata[j].sa_parity==sa_parity&&
         sa[qrdata[j].sa_index]<0){
          sa[qrdata[j].sa_index]=j;
          mark[j]=1;
        }
      }
      /*TODO: If the S-A group is complete, check the parity.*/
    }
    else{
      sa[0]=i;
      sa_size=1;
    }

    sa_ctext=0;
    fnc1=0;
    fnc1_2ai=0;
    has_kanji=0;
    /*Step 1: Detect FNC1 markers and estimate the required buffer size.*/
    for(j=0;j<sa_size;j++)if(sa[j]>=0){
      qrdataj=qrdata+sa[j];
      for(k=0;k<qrdataj->nentries;k++){
        int shift;
        entry=qrdataj->entries+k;
        shift=0;
        switch(entry->mode){
          /*FNC1 applies to the entire code and ignores subsequent markers.*/
          case QR_MODE_FNC1_1ST:{
            if(!fnc1)fnc1=MOD(ZBAR_MOD_GS1);
          }break;
          case QR_MODE_FNC1_2ND:{
            if(!fnc1){
              fnc1=MOD(ZBAR_MOD_AIM);
              fnc1_2ai=entry->payload.ai;
              sa_ctext+=2;
            }
          }break;
          /*We assume at most 4 UTF-8 bytes per input byte.
            I believe this is true for all the encodings we actually use.*/
          case QR_MODE_KANJI:has_kanji=1;
          case QR_MODE_BYTE:shift=2;
          default:{
            /*The remaining two modes are already valid UTF-8.*/
            if(QR_MODE_HAS_DATA(entry->mode)){
              sa_ctext+=entry->payload.data.len<<shift;
            }
          }break;
        }
      }
    }

    /*Step 2: Convert the entries.*/
    sa_text=(char *)malloc((sa_ctext+1)*sizeof(*sa_text));
    sa_ntext=0;
    /*Add the encoded Application Indicator for FNC1 in the second position.*/
    if(fnc1==MOD(ZBAR_MOD_AIM)){
      if(fnc1_2ai<100){
        /*The Application Indicator is a 2-digit number.*/
        sa_text[sa_ntext++]='0'+fnc1_2ai/10;
        sa_text[sa_ntext++]='0'+fnc1_2ai%10;
      }
      /*The Application Indicator is a single letter.
        We already checked that it lies in one of the ranges A...Z, a...z
         when we decoded it.*/
      else sa_text[sa_ntext++]=(char)(fnc1_2ai-100);
    }
    eci=-1;
    /*
    enc_list[0]=sjis_cd;
    enc_list[1]=latin1_cd;
    enc_list[2]=utf8_cd;
*   */
    enc_list[1]=sjis_cd;
    enc_list[2]=latin1_cd;
    enc_list[0]=utf8_cd;

    eci_cd=(iconv_t)-1;
    err=0;
    for(j = 0; j < sa_size && !err; j++, sym = &(*sym)->next) {
      *sym = _zbar_image_scanner_alloc_sym(iscn, ZBAR_QRCODE, 0);
      (*sym)->datalen = sa_ntext;
      if(sa[j]<0){
        /* generic placeholder for unfinished results */
        (*sym)->type = ZBAR_PARTIAL;

        /*Skip all contiguous missing segments.*/
        for(j++;j<sa_size&&sa[j]<0;j++);
        /*If there aren't any more, stop.*/
        if(j>=sa_size)break;

        /* mark break in data */
        sa_text[sa_ntext++]='\0';
        (*sym)->datalen = sa_ntext;

        /* advance to next symbol */
        sym = &(*sym)->next;
        *sym = _zbar_image_scanner_alloc_sym(iscn, ZBAR_QRCODE, 0);
      }

      qrdataj=qrdata+sa[j];
      /* expose bounding box */
      sym_add_point(*sym, qrdataj->bbox[0][0], qrdataj->bbox[0][1]);
      sym_add_point(*sym, qrdataj->bbox[2][0], qrdataj->bbox[2][1]);
      sym_add_point(*sym, qrdataj->bbox[3][0], qrdataj->bbox[3][1]);
      sym_add_point(*sym, qrdataj->bbox[1][0], qrdataj->bbox[1][1]);

      /* approx symbol "up" direction */
      dir[0] = (qrdataj->bbox[0][0] - qrdataj->bbox[2][0] +
                qrdataj->bbox[1][0] - qrdataj->bbox[3][0]);
      dir[1] = (qrdataj->bbox[2][1] - qrdataj->bbox[0][1] +
                qrdataj->bbox[3][1] - qrdataj->bbox[1][1]);
      horiz = abs(dir[0]) > abs(dir[1]);
      (*sym)->orient = horiz + 2 * (dir[1 - horiz] < 0);

      for(k=0;k<qrdataj->nentries&&!err;k++){
        size_t              inleft;
        size_t              outleft;
        char               *in;
        char               *out;
        entry=qrdataj->entries+k;
        switch(entry->mode){
          case QR_MODE_NUM:{
            if(sa_ctext-sa_ntext>=(size_t)entry->payload.data.len){
              memcpy(sa_text+sa_ntext,entry->payload.data.buf,
               entry->payload.data.len*sizeof(*sa_text));
              sa_ntext+=entry->payload.data.len;
            }
            else err=1;
          }break;
          case QR_MODE_ALNUM:{
            char *p;
            in=(char *)entry->payload.data.buf;
            inleft=entry->payload.data.len;
            /*FNC1 uses '%' as an escape character.*/
            if(fnc1)for(;;){
              size_t plen;
              char   c;
              p=memchr(in,'%',inleft*sizeof(*in));
              if(p==NULL)break;
              plen=p-in;
              if(sa_ctext-sa_ntext<plen+1)break;
              memcpy(sa_text+sa_ntext,in,plen*sizeof(*in));
              sa_ntext+=plen;
              /*Two '%'s is a literal '%'*/
              if(plen+1<inleft&&p[1]=='%'){
                c='%';
                plen++;
                p++;
              }
              /*One '%' is the ASCII group separator.*/
              else c=0x1D;
              sa_text[sa_ntext++]=c;
              inleft-=plen+1;
              in=p+1;
            }
            else p=NULL;
            if(p!=NULL||sa_ctext-sa_ntext<inleft)err=1;
            else{
              memcpy(sa_text+sa_ntext,in,inleft*sizeof(*sa_text));
              sa_ntext+=inleft;
            }
          }break;
          /*TODO: This will not handle a multi-byte sequence split between
             multiple data blocks.
            Does such a thing occur?
            Is it allowed?
            It requires copying buffers around to handle correctly.*/
          case QR_MODE_BYTE:
          case QR_MODE_KANJI:{
            in=(char *)entry->payload.data.buf;
            inleft=entry->payload.data.len;
            out=sa_text+sa_ntext;
            outleft=sa_ctext-sa_ntext;
            /*If we have no specified encoding, attempt to auto-detect it.*/
            if(eci<0){
              int ei;
              /*If there was data encoded in kanji mode, assume it's SJIS.*/
              if(has_kanji)enc_list_mtf(enc_list,sjis_cd);
              /*Otherwise check for the UTF-8 BOM.
                UTF-8 is rarely specified with ECI, and few decoders
                 currently support doing so, so this is the best way for
                 encoders to reliably indicate it.*/
              else if(inleft>=3&&
               in[0]==(char)0xEF&&in[1]==(char)0xBB&&in[2]==(char)0xBF){
                in+=3;
                inleft-=3;
                /*Actually try converting (to check validity).*/
                err=utf8_cd==(iconv_t)-1||
                 iconv(utf8_cd,&in,&inleft,&out,&outleft)==(size_t)-1;
                if(!err){
                  sa_ntext=out-sa_text;
                  enc_list_mtf(enc_list,utf8_cd);
                  continue;
                }
                in=(char *)entry->payload.data.buf;
                inleft=entry->payload.data.len;
                out=sa_text+sa_ntext;
                outleft=sa_ctext-sa_ntext;
              }
              /*If the text is 8-bit clean, prefer UTF-8 over SJIS, since
                 SJIS will corrupt the backslashes used for DoCoMo formats.*/
              else if(text_is_ascii((unsigned char *)in,inleft)){
                enc_list_mtf(enc_list,utf8_cd);
              }
              /*Try our list of encodings.*/
              for(ei=0;ei<3;ei++)if(enc_list[ei]!=(iconv_t)-1){
                /*According to the 2005 version of the standard,
                   ISO/IEC 8859-1 (one hyphen) is supposed to be used, but
                   reality is not always so (and in the 2000 version of the
                   standard, it was JIS8/SJIS that was the default).
                  It's got an invalid range that is used often with SJIS
                   and UTF-8, though, which makes detection easier.
                  However, iconv() does not properly reject characters in
                   those ranges, since ISO-8859-1 (two hyphens) defines a
                   number of seldom-used control code characters there.
                  So if we see any of those characters, move this
                   conversion to the end of the list.*/
                if(ei<2&&enc_list[ei]==latin1_cd&&
                 !text_is_latin1((unsigned char *)in,inleft)){
                  int ej;
                  for(ej=ei+1;ej<3;ej++)enc_list[ej-1]=enc_list[ej];
                  enc_list[2]=latin1_cd;
                }
                err=iconv(enc_list[ei],&in,&inleft,&out,&outleft)==(size_t)-1;
                if(!err){
                  sa_ntext=out-sa_text;
                  enc_list_mtf(enc_list,enc_list[ei]);
                  break;
                }
                in=(char *)entry->payload.data.buf;
                inleft=entry->payload.data.len;
                out=sa_text+sa_ntext;
                outleft=sa_ctext-sa_ntext;
              }
            }
            /*We were actually given a character set; use it.
              The spec says that in this case, data should be treated as if it
               came from the given character set even when encoded in kanji
               mode.*/
            else{
              err=eci_cd==(iconv_t)-1||
               iconv(eci_cd,&in,&inleft,&out,&outleft)==(size_t)-1;
              if(!err)sa_ntext=out-sa_text;
            }
          }break;
          /*Check to see if a character set was specified.*/
          case QR_MODE_ECI:{
            const char *enc;
            char        buf[16];
            unsigned    cur_eci;
            cur_eci=entry->payload.eci;
            if(cur_eci<=QR_ECI_ISO8859_16&&cur_eci!=14){
              if(cur_eci!=QR_ECI_GLI0&&cur_eci!=QR_ECI_CP437){
                sprintf(buf,"ISO8859-%i",QR_MAXI(cur_eci,3)-2);
                enc=buf;
              }
              /*Note that CP437 requires an iconv compiled with
                 --enable-extra-encodings, and thus may not be available.*/
              else enc="CP437";
            }
            else if(cur_eci==QR_ECI_SJIS)enc="SJIS";
            else if(cur_eci==QR_ECI_UTF8)enc="UTF-8";
            /*Don't know what this ECI code specifies, but not an encoding that
               we recognize.*/
            else continue;
            eci=cur_eci;
            eci_cd=iconv_open("UTF-8",enc);
          }break;
          /*Silence stupid compiler warnings.*/
          default:break;
        }
      }
      /*If eci should be reset between codes, do so.*/
      if(eci<=QR_ECI_GLI1){
        eci=-1;
        if(eci_cd!=(iconv_t)-1)iconv_close(eci_cd);
      }
    }
    if(eci_cd!=(iconv_t)-1)iconv_close(eci_cd);
    if(!err){
      zbar_symbol_t *sa_sym;
      sa_text[sa_ntext++]='\0';
      if(sa_ctext+1>sa_ntext){
        sa_text=(char *)realloc(sa_text,sa_ntext*sizeof(*sa_text));
      }

      if(sa_size == 1)
          sa_sym = syms;
      else {
          /* cheap out w/axis aligned bbox for now */
          int xmin = img->width, xmax = -2;
          int ymin = img->height, ymax = -2;

          /* create "virtual" container symbol for composite result */
          sa_sym = _zbar_image_scanner_alloc_sym(iscn, ZBAR_QRCODE, 0);
          sa_sym->syms = _zbar_symbol_set_create();
          sa_sym->syms->head = syms;

          /* fixup data references */
          for(; syms; syms = syms->next) {
              int next;
              _zbar_symbol_refcnt(syms, 1);
              if(syms->type == ZBAR_PARTIAL)
                  sa_sym->type = ZBAR_PARTIAL;
              else
                  for(j = 0; j < syms->npts; j++) {
                      int u = syms->pts[j].x;
                      if(xmin >= u) xmin = u - 1;
                      if(xmax <= u) xmax = u + 1;
                      u = syms->pts[j].y;
                      if(ymin >= u) ymin = u - 1;
                      if(ymax <= u) ymax = u + 1;
                  }
              syms->data = sa_text + syms->datalen;
              next = (syms->next) ? syms->next->datalen : sa_ntext;
              assert(next > syms->datalen);
              syms->datalen = next - syms->datalen - 1;
          }
          if(xmax >= -1) {
              sym_add_point(sa_sym, xmin, ymin);
              sym_add_point(sa_sym, xmin, ymax);
              sym_add_point(sa_sym, xmax, ymax);
              sym_add_point(sa_sym, xmax, ymin);
          }
      }
      sa_sym->data = sa_text;
      sa_sym->data_alloc = sa_ntext;
      sa_sym->datalen = sa_ntext - 1;
      sa_sym->modifiers = fnc1;

      _zbar_image_scanner_add_sym(iscn, sa_sym);
    }
    else {
        _zbar_image_scanner_recycle_syms(iscn, syms);
        free(sa_text);
    }
  }
  if(utf8_cd!=(iconv_t)-1)iconv_close(utf8_cd);
  if(sjis_cd!=(iconv_t)-1)iconv_close(sjis_cd);
  if(latin1_cd!=(iconv_t)-1)iconv_close(latin1_cd);
  free(mark);
  return ntext;
}
