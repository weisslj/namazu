/************************************************

  util.h -

  $Author: satoru $
  $Date: 1999-11-18 02:46:02 $
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

#define iskanji1st(c) (((unsigned int)(c) >= 0x81 && \
                       (unsigned int)(c)) <= 0x9f || \
                       ((unsigned int)(c) >= 0xe0 && \
                       (unsigned int)(c) <= 0xfc))
#define iskanji2nd(c) (((unsigned int)(c) >= 0x40 && \
                       (unsigned int)(c) <= 0xfc && \
                       (unsigned int)(c) != 0x7f))
#define iseuc(c)  ((unsigned int)(c) >= 0xa1 && (unsigned int)(c) <= 0xfe)
#ifndef iskanji /* for jperl */
#define iskanji(c)  (iseuc(*(c)) && iseuc(*(c + 1)))
#endif
#define ischoon(c) ((unsigned int)*(c) == 0xa1 && \
                    (unsigned int)*(c + 1) == 0xbc)
#define iseuc(c)  ((unsigned int)(c) >= 0xa1 && (unsigned int)(c) <= 0xfe)

/************************************************************
 *
 * Function declarations
 *
 ************************************************************/

unsigned long scan_hex();
unsigned long scan_oct();

void *xmalloc(unsigned long);
void *xrealloc(void*, unsigned long);
void tr(char*, char*, char*);
void chomp(char*);

void strlower();
void delete_backslashes();
int get_unpackw();
int read_unpackw(FILE *, int *, int);

char *lastc();
size_t freadx();
long getidxptr(FILE* , long);
int issymbol(int);
#ifndef die /* for perl XSUB... */
void die(char*, ...);
#endif
void diemsg(char*, ...);
void diewithmsg();
void warnf(char*, ...);
void debug_printf(char*, ...);
void pathcat(char*, char*);

int  isnumstr(char *);
void commas(char *);

char *strcasestr(char *, char *);
int strprefixcmp(char *, char *);
int strsuffixcmp(char *, char *);
int strprefixcasecmp(char *, char *);
int strsuffixcasecmp(char *, char *);
char *readfile(char*);
void subst(char*, char*, char*);
void cat(char*);

char *safe_getenv(char *s);

void print(char*);
void wprint(char*);

#endif /* _UTIL_H */

