/*

  util.h -

  $Author: satoru $
  $Date: 1999-11-23 09:46:18 $
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

/*
 *
 * Function declarations
 *
 */

extern unsigned long scan_hex();
extern unsigned long scan_oct();

extern void *xmalloc(unsigned long);
extern void *xrealloc(void*, unsigned long);
extern void tr(char*, char*, char*);
extern void chomp(char*);

extern void strlower();
extern void delete_backslashes();
extern int get_unpackw();
extern int read_unpackw(FILE *, int *, int);

extern char *lastc();
size_t freadx();
extern long getidxptr(FILE* , long);
extern int issymbol(int);
#ifndef die /* for perl XSUB... */
extern void die(char*, ...);
#endif
extern void diewithmsg();
extern void warnf(char*, ...);
extern void debug_printf(char*, ...);
extern void pathcat(char*, char*);

extern int  isnumstr(char *);
extern void commas(char *);

extern char *strcasestr(char *, char *);
extern int strprefixcmp(char *, char *);
extern int strsuffixcmp(char *, char *);
extern int strprefixcasecmp(char *, char *);
extern int strsuffixcasecmp(char *, char *);
extern char *readfile(char*);
extern void subst(char*, char*, char*);
extern void cat(char*);

extern char *safe_getenv(char *s);

extern void print(char*);
extern void wprint(char*);

#endif /* _UTIL_H */

