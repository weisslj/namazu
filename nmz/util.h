/*

  util.h -

  $Author: satoru $
  $Date: 2000-01-08 01:09:24 $
  created at: Thu Mar  9 11:55:53 JST 1995

  Copyright (C) 1993-1998 Yukihiro Matsumoto

  Modifications for Namazu 
  by satoru-t@is.aist-nara.ac.jp

*/

#ifndef _UTIL_H
#define _UTIL_H

#include <stdio.h>     /* for FILE struct */
#include "libnamazu.h" /* for enum nmz_stat struct */

#ifdef HAVE__VSNPRINTF
#define vsnprintf(str,n,format,ap) _vsnprintf(str,n,format,ap)
#endif /* HAVE__VSNPRINTF */

/*
 *
 * Macros
 *
 */

#define iskanji1st(c) (((uchar)(c) >= 0x81 && \
                       (uchar)(c)) <= 0x9f || \
                       ((uchar)(c) >= 0xe0 && \
                       (uchar)(c) <= 0xfc))
#define iskanji2nd(c) (((uchar)(c) >= 0x40 && \
                       (uchar)(c) <= 0xfc && \
                       (uchar)(c) != 0x7f))
#define iseuc(c)  ((uchar)(c) >= 0xa1 && (uchar)(c) <= 0xfe)
#ifndef iskanji /* for jperl */
#define iskanji(c)  (iseuc(*(c)) && iseuc(*(c + 1)))
#endif
#define ischoon(c) ((uchar)*(c) == 0xa1 && \
                    (uchar)*(c + 1) == 0xbc)
#define iseuc(c)  ((uchar)(c) >= 0xa1 && (uchar)(c) <= 0xfe)


extern unsigned long scan_oct ( char *start, int len, int *retlen );
extern unsigned long scan_hex ( char *start, int len, int *retlen );
extern void * xmalloc ( unsigned long size );
extern void * xrealloc ( void *ptr, unsigned long size );
extern void nmz_tr ( char *str, const char *lstr, const char *rstr );
extern void nmz_chomp ( char * str );
extern size_t nmz_fread ( void *ptr, size_t size, size_t nmemb, FILE *stream );
extern int nmz_get_unpackw ( FILE *fp, int *data );
extern int nmz_read_unpackw ( FILE *fp, int *buf, int size );
extern long nmz_getidxptr ( FILE * fp, long point );
extern int nmz_issymbol ( int c );
extern void nmz_die ( const char *fmt, ... );
extern void nmz_die_with_msg ( void );
extern void nmz_warn_printf ( const char *fmt, ... );
extern void nmz_debug_printf ( const char *fmt, ... );
extern void nmz_pathcat ( const char *base, char *name );
extern int nmz_isnumstr ( const char *str );
extern void nmz_commas ( char *str );
extern void nmz_strlower ( char *str );
extern char * nmz_strcasestr ( const char *haystack, const char *needle );
extern int nmz_strprefixcasecmp ( const char *str1, const char *str2 );
extern int nmz_strprefixcmp ( const char *str1, const char *str2 );
extern int nmz_strsuffixcmp ( const char *str1, const char *str2 );
extern char * nmz_readfile ( const char *fname );
extern void nmz_subst ( char *str, const char *pat, const char *rep );
extern void nmz_cat ( const char *fname );
extern char * nmz_getenv ( const char *str );
extern void nmz_decode_uri ( char * str );
extern char * nmz_strerrror ( enum nmz_stat errnumt );
extern void nmz_print ( const char *str );

#endif /* _UTIL_H */
