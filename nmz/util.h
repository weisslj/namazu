#ifndef _UTIL_H
#define _UTIL_H

#include <stdio.h>     /* for FILE struct */
#include "libnamazu.h" /* for enum nmz_stat struct */

/* FIXME: Will be moved.  */
#ifdef HAVE__VSNPRINTF
#define vsnprintf(str,n,format,ap) _vsnprintf(str,n,format,ap)
#endif /* HAVE__VSNPRINTF */

/*
 *
 * Data Structures.
 *
 */

/* Simple string list. */
struct nmz_strlist {
    char *value;
    struct nmz_strlist *next;
};

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
#define nmz_iseuc(c)  ((uchar)(c) >= 0xa1 && (uchar)(c) <= 0xfe)
#define nmz_iskanji(c)  (nmz_iseuc(*(c)) && nmz_iseuc(*(c + 1)))


extern unsigned long nmz_scan_oct ( char *start, int len, int *retlen );
extern unsigned long nmz_scan_hex ( char *start, int len, int *retlen );
extern void * nmz_xmalloc ( unsigned long size );
extern void * nmz_xrealloc ( void *ptr, unsigned long size );
extern void nmz_tr ( char *str, const char *lstr, const char *rstr );
extern void nmz_chomp ( char * str );
extern size_t nmz_fread ( void *ptr, size_t size, size_t nmemb, FILE *stream );
extern int nmz_get_unpackw ( FILE *fp, int *data );
extern int nmz_read_unpackw ( FILE *fp, int *buf, int size );
extern long nmz_getidxptr ( FILE * fp, long point );
extern int nmz_issymbol ( int c );
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
extern char * nmz_getenv ( const char *str );
extern void nmz_decode_uri ( char * str );
extern char * nmz_strerror ( enum nmz_stat errnumt );
struct nmz_strlist* nmz_push_strlist(struct nmz_strlist *list, const char *arg);
void nmz_free_strlist(struct nmz_strlist *list);

#endif /* _UTIL_H */
