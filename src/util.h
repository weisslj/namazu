/************************************************

  util.h -

  $Author: satoru $
  $Date: 1999-08-25 07:09:24 $
  created at: Thu Mar  9 11:55:53 JST 1995

  Copyright (C) 1993-1998 Yukihiro Matsumoto

************************************************/

/* 
 *   Some small modifications for Namazu 
 *    by satoru-t@is.aist-nara.ac.jp
 */

#include "namazu.h"

#ifndef _UTIL_H
#define _UTIL_H

unsigned long scan_hex();
unsigned long scan_oct();

/*
 * added by satoru-t@is.aist-nara.ac.jp
 */

#define iskanji1st(c) (((c) >= 0x81 && (c)) <= 0x9f ||\
                      ((c) >= 0xe0 && (c) <= 0xfc))
#define iskanji2nd(c) ((c) >= 0x40 && (c) <= 0xfc && (c) != 0x7f)
#define iseuc(c)  ((c) >= 0xa1 && (c) <= 0xfe)
#define iskanji(c)  (iseuc(*(c)) && iseuc(*(c + 1)))
#define ischoon(c) ((int)*(c) == 0xa1 && (int)*(c + 1) == 0xbc)
#define iseuc(c)  ((c) >= 0xa1 && (c) <= 0xfe)

void *xmalloc(unsigned long);
void *xrealloc(void*, unsigned long);
void tr(uchar*, uchar*, uchar*);
void chomp(uchar*);

#if !defined(HAVE_MEMMOVE)
void *memmove(void*, void*, size_t);
#endif

void decode_uri(uchar *);
void strlower();
void delete_backslashes();
int get_unpackw();
int read_unpackw(FILE *, int *, int);
uchar *strcasestr(uchar *, uchar *);

uchar *lastc();
size_t freadx();
LIST *add_list();
long getidxptr(FILE* , long);
int issymbol(int);
void die(char*, ...);
size_t strlen2(uchar *, int);
void pathcat(uchar*, uchar*);

#endif /* _UTIL_H */

