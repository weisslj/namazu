/************************************************

  util.h -

  $Author: satoru $
  $Date: 1999-06-12 14:29:32 $
  created at: Thu Mar  9 11:55:53 JST 1995

  Copyright (C) 1993-1998 Yukihiro Matsumoto

************************************************/

/* 
   Some small modifications for Namazu 
      by ccsatoru@vega.aichi-u.ac.jp [05/21/1998]
*/

#include "namazu.h"

#ifndef UTIL_H
#define UTIL_H

unsigned long scan_hex();
unsigned long scan_oct();

/*
  added by ccsatoru@aichi-u.ac.jp
 */

void *xrealloc();
void *xmalloc();
void tr();
void chop();

#if !defined(HAVE_MEMMOVE)
void *memmove();
#endif

void decode_uri_string();
void tolower_string();
void delete_backslashes();
int get_unpackw();
int read_unpackw();

uchar *lastc();
size_t freadx();
LIST *add_list();

#endif /* UTIL_H */

