/*Copyright (C) 2008-2009  Timothy B. Terriberry (tterribe@xiph.org)
  You can redistribute this library and/or modify it under the terms of the
   GNU Lesser General Public License as published by the Free Software
   Foundation; either version 2.1 of the License, or (at your option) any later
   version.*/
#if !defined(_qrdec_H)
# define _qrdec_H (1)

#include <zbar.h>

typedef struct qr_code_data_entry qr_code_data_entry;
typedef struct qr_code_data       qr_code_data;
typedef struct qr_code_data_list  qr_code_data_list;

typedef enum qr_mode{
  /*Numeric digits ('0'...'9').*/
  QR_MODE_NUM=1,
  /*Alphanumeric characters ('0'...'9', 'A'...'Z', plus the punctuation
     ' ', '$', '%', '*', '+', '-', '.', '/', ':').*/
  QR_MODE_ALNUM,
  /*Structured-append header.*/
  QR_MODE_STRUCT,
  /*Raw 8-bit bytes.*/
  QR_MODE_BYTE,
  /*FNC1 marker (for more info, see http://www.mecsw.com/specs/uccean128.html).
    In the "first position" data is formatted in accordance with GS1 General
     Specifications.*/
  QR_MODE_FNC1_1ST,
  /*Mode 6 reserved?*/
  /*Extended Channel Interpretation code.*/
  QR_MODE_ECI=7,
  /*SJIS kanji characters.*/
  QR_MODE_KANJI,
  /*FNC1 marker (for more info, see http://www.mecsw.com/specs/uccean128.html).
    In the "second position" data is formatted in accordance with an industry
     application as specified by AIM Inc.*/
  QR_MODE_FNC1_2ND
}qr_mode;

/*Check if a mode has a data buffer associated with it.
  Currently this is only modes with exactly one bit set.*/
#define QR_MODE_HAS_DATA(_mode) (!((_mode)&(_mode)-1))

/*ECI may be used to signal a character encoding for the data.*/
typedef enum qr_eci_encoding{
  /*GLI0 is like CP437, but the encoding is reset at the beginning of each
     structured append symbol.*/
  QR_ECI_GLI0,
  /*GLI1 is like ISO8859_1, but the encoding is reset at the beginning of each
     structured append symbol.*/
  QR_ECI_GLI1,
  /*The remaining encodings do not reset at the start of the next structured
     append symbol.*/
  QR_ECI_CP437,
  /*Western European.*/
  QR_ECI_ISO8859_1,
  /*Central European.*/
  QR_ECI_ISO8859_2,
  /*South European.*/
  QR_ECI_ISO8859_3,
  /*North European.*/
  QR_ECI_ISO8859_4,
  /*Cyrillic.*/
  QR_ECI_ISO8859_5,
  /*Arabic.*/
  QR_ECI_ISO8859_6,
  /*Greek.*/
  QR_ECI_ISO8859_7,
  /*Hebrew.*/
  QR_ECI_ISO8859_8,
  /*Turkish.*/
  QR_ECI_ISO8859_9,
  /*Nordic.*/
  QR_ECI_ISO8859_10,
  /*Thai.*/
  QR_ECI_ISO8859_11,
  /*There is no ISO/IEC 8859-12.*/
  /*Baltic rim.*/
  QR_ECI_ISO8859_13=QR_ECI_ISO8859_11+2,
  /*Celtic.*/
  QR_ECI_ISO8859_14,
  /*Western European with euro.*/
  QR_ECI_ISO8859_15,
  /*South-Eastern European (with euro).*/
  QR_ECI_ISO8859_16,
  /*ECI 000019 is reserved?*/
  /*Shift-JIS.*/
  QR_ECI_SJIS=20,
  /*UTF-8.*/
  QR_ECI_UTF8=26
}qr_eci_encoding;


/*A single unit of parsed QR code data.*/
struct qr_code_data_entry{
  /*The mode of this data block.*/
  qr_mode mode;
  union{
    /*Data buffer for modes that have one.*/
    struct{
      unsigned char *buf;
      int            len;
    }data;
    /*Decoded "Extended Channel Interpretation" data.*/
    unsigned eci;
    /*Decoded "Application Indicator" for FNC1 in 2nd position.*/
    int      ai;
    /*Structured-append header data.*/
    struct{
      unsigned char sa_index;
      unsigned char sa_size;
      unsigned char sa_parity;
    }sa;
  }payload;
};



/*Low-level QR code data.*/
struct qr_code_data{
  /*The decoded data entries.*/
  qr_code_data_entry *entries;
  int                 nentries;
  /*The code version (1...40).*/
  unsigned char       version;
  /*The ECC level (0...3, corresponding to 'L', 'M', 'Q', and 'H').*/
  unsigned char       ecc_level;
  /*Structured-append information.*/
  /*The index of this code in the structured-append group.
    If sa_size is zero, this is undefined.*/
  unsigned char       sa_index;
  /*The size of the structured-append group, or 0 if there was no S-A header.*/
  unsigned char       sa_size;
  /*The parity of the entire structured-append group.
    If sa_size is zero, this is undefined.*/
  unsigned char       sa_parity;
  /*The parity of this code.
    If sa_size is zero, this is undefined.*/
  unsigned char       self_parity;
  /*An approximate bounding box for the code.
    Points appear in the order up-left, up-right, down-left, down-right,
     relative to the orientation of the QR code.*/
  qr_point            bbox[4];
};


struct qr_code_data_list{
  qr_code_data *qrdata;
  int           nqrdata;
  int           cqrdata;
};


/*Extract symbol data from a list of QR codes and attach to the image.
  All text is converted to UTF-8.
  Any structured-append group that does not have all of its members is decoded
   as ZBAR_PARTIAL with ZBAR_PARTIAL components for the discontinuities.
  Note that isolated members of a structured-append group may be decoded with
   the wrong character set, since the correct setting cannot be propagated
   between codes.
  Return: The number of symbols which were successfully extracted from the
   codes; this will be at most the number of codes.*/
int qr_code_data_list_extract_text(const qr_code_data_list *_qrlist,
                                   zbar_image_scanner_t *iscn,
                                   zbar_image_t *img);


/*TODO: Parse DoCoMo standard barcode data formats.
  See http://www.nttdocomo.co.jp/english/service/imode/make/content/barcode/function/application/
   for details.*/

#endif
