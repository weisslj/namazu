/*

  util.h -

  $Author: rug $
  $Date: 1999-12-12 14:09:04 $
  created at: Thu Mar  9 11:55:53 JST 1995

  Copyright (C) 1993-1998 Yukihiro Matsumoto

  Modifications for Namazu 
  by satoru-t@is.aist-nara.ac.jp

*/

#ifndef _UTIL_H
#define _UTIL_H

#include <stdio.h> /* for FILE struct */

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


/* regex library related functions.  */
extern unsigned long scan_oct ( char *start, int len, int *retlen );
extern unsigned long scan_hex ( char *start, int len, int *retlen );
extern void * xmalloc ( unsigned long size );
extern void * xrealloc ( void *ptr, unsigned long size );

extern void nmz_tr ( char *str, char *lstr, char *rstr );
extern void nmz_chomp ( char * s );
extern size_t nmz_fread ( void *ptr, size_t size, size_t nmemb, FILE *stream );
extern int nmz_get_unpackw ( FILE *fp, int *data );
extern int nmz_read_unpackw ( FILE *fp, int *buf, int size );
extern long nmz_getidxptr ( FILE * fp, long p );
extern int nmz_issymbol ( int c );
extern void nmz_die ( char *fmt, ... );
extern void nmz_die_with_msg ( void );
extern void nmz_warn_printf ( char *fmt, ... );
extern void nmz_debug_printf ( char *fmt, ... );
extern void nmz_pathcat ( char *base, char *name );
extern int nmz_isnumstr ( char *str );
extern void nmz_commas ( char *str );
extern void nmz_strlower ( char *str );
extern char *nmz_strcasestr ( char *haystack, char *needle );
extern int nmz_strprefixcasecmp ( char *str1, char *str2 );
extern int nmz_strprefixcmp ( char *str1, char *str2 );
extern int nmz_strsuffixcmp ( char *str1, char *str2 );
extern char *nmz_readfile ( char *fname );
extern void nmz_subst ( char *p, char *pat, char *rep );
extern void nmz_cat ( char *fname );
extern char *nmz_getenv ( char *s );
extern void nmz_decode_uri ( char * s );
extern char *nmz_get_errmsg ( enum nmz_stat stat );
extern void nmz_print ( char *s );

#endif /* _UTIL_H */
