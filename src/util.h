/************************************************

  util.h -

  $Author: satoru $
  $Date: 1999-09-06 07:56:39 $
  created at: Thu Mar  9 11:55:53 JST 1995

  Copyright (C) 1993-1998 Yukihiro Matsumoto

  Modifications for Namazu 
  by satoru-t@is.aist-nara.ac.jp

************************************************/

#include "namazu.h"

#ifndef _UTIL_H
#define _UTIL_H

/************************************************************
 *
 * Macros
 *
 ************************************************************/

#define iskanji1st(c) (((c) >= 0x81 && (c)) <= 0x9f ||\
                      ((c) >= 0xe0 && (c) <= 0xfc))
#define iskanji2nd(c) ((c) >= 0x40 && (c) <= 0xfc && (c) != 0x7f)
#define iseuc(c)  ((c) >= 0xa1 && (c) <= 0xfe)
#define iskanji(c)  (iseuc(*(c)) && iseuc(*(c + 1)))
#define ischoon(c) ((int)*(c) == 0xa1 && (int)*(c + 1) == 0xbc)
#define iseuc(c)  ((c) >= 0xa1 && (c) <= 0xfe)

#define is_lang_ja(a) (strcmp((a),"ja") == 0)


/************************************************************
 *
 * Function declarations
 *
 ************************************************************/

#if !defined(HAVE_MEMMOVE)
void *memmove(void*, void*, size_t);
#endif

unsigned long scan_hex();
unsigned long scan_oct();

void *xmalloc(unsigned long);
void *xrealloc(void*, unsigned long);
void tr(uchar*, uchar*, uchar*);
void chomp(uchar*);

void decode_uri(uchar *);
void strlower();
void delete_backslashes();
int get_unpackw();
int read_unpackw(FILE *, int *, int);

uchar *lastc();
size_t freadx();
LIST *add_list();
long getidxptr(FILE* , long);
int issymbol(int);
void die(char*, ...);
size_t strlen2(uchar *, int);
void pathcat(uchar*, uchar*);
void setprogname(char *);

int  isnumstr(char *);
void commas(char *);

uchar *strcasestr(uchar *, uchar *);
int strprefixcmp(uchar *, uchar *);
int strsuffixcmp(uchar *, uchar *);
int strprefixcasecmp(uchar *, uchar *);
int strsuffixcasecmp(uchar *, uchar *);
uchar *readfile(uchar*);
void subst(uchar*, uchar*, uchar*);

#endif /* _UTIL_H */

