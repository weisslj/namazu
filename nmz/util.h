/************************************************

  util.h -

  $Author: kenzo- $
  $Date: 1999-11-14 22:54:02 $
  created at: Thu Mar  9 11:55:53 JST 1995

  Copyright (C) 1993-1998 Yukihiro Matsumoto

  Modifications for Namazu 
  by satoru-t@is.aist-nara.ac.jp

************************************************/

#ifndef _UTIL_H
#define _UTIL_H

#ifdef HAVE__VSNPRINTF
#define vsnprintf(str,n,format,ap) _vsnprintf(str,n,format,ap)
#endif /* HAVE__VSNPRINTF */

/************************************************************
 *
 * Macros
 *
 ************************************************************/

#define iskanji1st(c) (((c) >= 0x81 && (c)) <= 0x9f ||\
                      ((c) >= 0xe0 && (c) <= 0xfc))
#define iskanji2nd(c) ((c) >= 0x40 && (c) <= 0xfc && (c) != 0x7f)
#define iseuc(c)  ((c) >= 0xa1 && (c) <= 0xfe)
#ifndef iskanji /* for jperl */
#define iskanji(c)  (iseuc(*(c)) && iseuc(*(c + 1)))
#endif
#define ischoon(c) ((int)*(c) == 0xa1 && (int)*(c + 1) == 0xbc)
#define iseuc(c)  ((c) >= 0xa1 && (c) <= 0xfe)

/************************************************************
 *
 * Function declarations
 *
 ************************************************************/

unsigned long scan_hex();
unsigned long scan_oct();

void *xmalloc(unsigned long);
void *xrealloc(void*, unsigned long);
void tr(uchar*, uchar*, uchar*);
void chomp(uchar*);

void strlower();
void delete_backslashes();
int get_unpackw();
int read_unpackw(FILE *, int *, int);

uchar *lastc();
size_t freadx();
long getidxptr(FILE* , long);
int issymbol(int);
void die(char*, ...);
void diemsg(char*, ...);
void diewithmsg();
void warnf(char*, ...);
void debug_printf(char*, ...);
void pathcat(uchar*, uchar*);

int  isnumstr(char *);
void commas(char *);

uchar *strcasestr(uchar *, uchar *);
int strprefixcmp(uchar *, uchar *);
int strsuffixcmp(uchar *, uchar *);
int strprefixcasecmp(uchar *, uchar *);
int strsuffixcasecmp(uchar *, uchar *);
uchar *readfile(uchar*);
void subst(uchar*, uchar*, uchar*);
void cat(uchar*);

char *safe_getenv(char *s);

void print(uchar*);
void wprint(uchar*);

#endif /* _UTIL_H */

