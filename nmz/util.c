/*
 * 
 * $Id: util.c,v 1.64 2000-02-25 13:31:40 satoru Exp $
 * 
 * Copyright (C) 1997-1999 Satoru Takabayashi All rights reserved.
 * Copyright (C) 2000 Namazu Project All rights reserved.
 * This is free software with ABSOLUTELY NO WARRANTY.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA
 * 
 * nmz_scan_hex(), nmz_scan_oct(), nmz_xmalloc(), nmz_xrealloc() are 
 * originally imported from Ruby b19's "util.c" and "gc.c". 
 * Thanks to Mr. Yukihiro Matsumoto <matz@netlab.co.jp>.
 *
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>
#ifdef __EMX__
#include <sys/types.h>
#endif
#include <sys/stat.h>
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "libnamazu.h"
#include "util.h"
#include "i18n.h"
#include "var.h"
#include "system.h"

/*
 *
 * Private functions
 *
 */

static void reverse_byte_order (void*, int, int);
static char decode_uri_sub(char c1, char c2);
static int nmz_tolower(int c);

/* 
 * Reverse byte order. It's type independent.
 */
static void 
reverse_byte_order (void *p, int n, int size)
{
    int i, j;
    char *pp, tmp;

    pp = (char *)p;
    for (i = 0; i < n; i++) {
        char *c = (pp + (i * size));
        for (j = 0; j < (size / 2); j++) {
            tmp = *(c + j);
            *(c + j)= *(c + size - 1 - j);
            *(c + size - 1 - j) = tmp;
        }
    }
}

static char 
decode_uri_sub(char c1, char c2)
{
    char c;

    c =  ((c1 >= 'A' ? (toupper(c1) - 'A') + 10 : (c1 - '0'))) * 16;
    c += ( c2 >= 'A' ? (toupper(c2) - 'A') + 10 : (c2 - '0'));
    return c;
}

/* 
 * Substitute for tolower(3).
 */
static int 
nmz_tolower(int c)
{
    if (c >= 'A' && c <= 'Z') {
	c = 'a' + c - 'A';
	return c;
    }
    return c;
}

/*
 *
 * Public functions
 *
 */

unsigned long
nmz_scan_oct(char *start, int len, int *retlen)
{
    register char *s = start;
    register unsigned long retval = 0;

    while (len-- && *s >= '0' && *s <= '7') {
	retval <<= 3;
	retval |= *s++ - '0';
    }

    *retlen = s - start;
    return retval;
}

unsigned long
nmz_scan_hex(char *start, int len, int *retlen)
{
    static char hexdigit[] = "0123456789abcdef0123456789ABCDEFx";
    register char *s = start;
    register unsigned long retval = 0;
    char *tmp;

    while (len-- && *s && (tmp = strchr(hexdigit, *s))) {
	retval <<= 4;
	retval |= (tmp - hexdigit) & 15;
	s++;
    }
    *retlen = s - start;
    return retval;
}


static unsigned long malloc_memories = 0;

void *
nmz_xmalloc(unsigned long size)
{
    void *mem;

    if (size == 0) size = 1;
    malloc_memories += size;
    mem = malloc(size);

    return mem;
}

void *
nmz_xrealloc(void *ptr, unsigned long size)
{
    void *mem;

    if (!ptr) return nmz_xmalloc(size);
    mem = realloc(ptr, size);
    return mem;
}



void 
nmz_tr(char *str, const char *lstr, const char *rstr)
{
    while (*str) {
	char *idx = strchr(lstr, *str);
	if (idx != NULL) { /* found */
	    *str = *(idx - lstr + rstr);
	}
        str++;
    }
}

/* 
 * Delete ending white spaces in the str.
 */
void 
nmz_chomp(char *str)
{
    char *p = str + strlen(str) - 1;

    for (; p >= str; p--) {
	if (*p == '\n' || *p == '\r' || *p == ' ' || *p == '\t') {
	    *p = '\0';
	} else {
	    break;
	}
    }
}


/* 
 * Do fread with endian consideration.
 */
size_t 
nmz_fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t value;

    value = fread(ptr, size, nmemb, stream);
/*
 * FIXME: Please tell me if you know more better way.
 */
#ifndef WORDS_BIGENDIAN
    reverse_byte_order(ptr, nmemb, size);
#endif
    return value;
}

int 
nmz_get_unpackw(FILE *fp, int *data)
{
    int val = 0, i = 0;

    while (1) {
	int tmp = getc(fp);
	i++;
	if (tmp == EOF) {
	    return 0;
	}
	if (tmp < 128) {
	    val += tmp;
	    *data = val;
	    return i;
	} else {
	    tmp &= 0x7f;
	    val += tmp;
	    val <<= 7;
	}
    }
}

int 
nmz_read_unpackw(FILE *fp, int *buf, int size) {
    int i = 0,  n = 0;
    
    while (i < size) {
	int tmp = nmz_get_unpackw(fp, &buf[n]);
	n++;
	if (tmp == 0) {  /* Error */
	    break;
	} else {
	    i += tmp;
	}
    }
    return  n;
}

/* 
 * Read an index and return its value which is a pointer to another file.
 */
long 
nmz_getidxptr(FILE * fp, long point)
{
    int val;

    fseek(fp, point * sizeof(int), 0);
    nmz_fread(&val, sizeof(int), 1, fp);
    return (long) val;
}

/* 
 * Warning messaging function.
 */
void 
nmz_warn_printf(const char *fmt, ...)
{
    va_list args;

    fflush(stdout);

    fprintf(stderr, "%s: ", PACKAGE);

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    if (fmt[strlen(fmt) - 1] != '\n') {
	fprintf(stderr, "\n");
    }

    fflush(stderr);
}

/* 
 * Debug messaging function.
 */
void 
nmz_debug_printf(const char *fmt, ...)
{
    va_list args;

    if (!nmz_is_debugmode()) {
	return;
    }

    fflush(stdout);

    fprintf(stderr, "%s(debug): ", PACKAGE);

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    if (fmt[strlen(fmt) - 1] != '\n') {
	fprintf(stderr, "\n");
    }

    fflush(stderr);
}

void 
nmz_pathcat(const char *base, char *name)
{
    char work[BUFSIZE];
    int i;

    for (i = strlen(name) - 1; i >= 0; i--) {
        if (name[i] == '/') {
            strcpy(name, name + i + 1);
            break;
        }
    }
    strcpy(work, base);
    strcat(work, "/");
    strcat(work, name);
    strcpy(name, work);
}

int 
nmz_isnumstr(const char *str)
{
    const unsigned char *p;

    if (strlen(str) > 10) {  /* Too large number */
	return 0;
    }

    for (p = (const unsigned char *)str; *p != '\0'; p++) {
	if (! isdigit((int)*p)) {
	    return 0;
	}
    }

    return 1;
}

void 
nmz_strlower(char *str)
{
    while (*str) {
	/* Using ascii dependent lower same as mknmz.  */
        *str = nmz_tolower(*str);
        str++;
    }
}

int 
nmz_strprefixcasecmp(const char *str1, const char *str2)
{
    int leng1, leng2;

    leng1 = strlen(str1);
    leng2 = strlen(str2);

    if (leng1 > leng2) {
	return strncasecmp(str1, str2, leng2);
    } else {
	return strncasecmp(str2, str1, leng1);
    }
}

int 
nmz_strprefixcmp(const char *str1, const char *str2)
{
    int leng1, leng2;

    leng1 = strlen(str1);
    leng2 = strlen(str2);

    if (leng1 > leng2) {
	return strncmp(str1, str2, leng2);
    } else {
	return strncmp(str2, str1, leng1);
    }
}

int 
nmz_strsuffixcmp(const char *str1, const char *str2)
{
    int leng1, leng2;

    leng1 = strlen(str1);
    leng2 = strlen(str2);

    if (leng1 > leng2) {
	return strcmp(str1 + leng1 - leng2, str2);
    } else {					     
	return strcmp(str2 + leng2 - leng1, str1);
    }
}

/*
 * Load the whole of file.
 */
char *
nmz_readfile(const char *fname)
{
    char *buf;
    FILE *fp;
    struct stat fstatus;

    errno = 0; /* errno must be initialized. */

    stat(fname, &fstatus);
    fp = fopen(fname, "rb");
    if (fp == NULL) {
        nmz_warn_printf("%s: %s", fname, strerror(errno));
        return NULL;
    }
    buf = malloc(fstatus.st_size + 1);
    if (buf == NULL) {
	nmz_set_dyingmsg(nmz_msg("%s: %s", fname, strerror(errno)));
	return NULL;
    }
    if (fread(buf, sizeof(char), fstatus.st_size, fp) == 0) {
        nmz_set_dyingmsg(nmz_msg("%s: %s", fname, strerror(errno)));
	return NULL;
    }
    *(buf + fstatus.st_size) = '\0';
    fclose(fp);
    return buf;
}

/*
 * Safe version of getenv. 
 */
char *
nmz_getenv(const char *s)
{
    char *cp;
    return (cp = getenv(s)) ? cp : "";
}

/*
 * Decoding URI encoded strings
 */
void 
nmz_decode_uri(char *str)
{
    int i, j;
    for (i = j = 0; str[i]; i++, j++) {
	if (str[i] == '%') {
	    str[j] = decode_uri_sub(str[i + 1], str[i + 2]);
	    i += 2;
	} else if (str[i] == '+') {
	    str[j] = ' ';
	} else {
	    str[j] = str[i];
	}
    }
    str[j] = '\0';
}

/*
 * Returns a string describing the libnmz error code passed
 * in the argument errnum just like strerror().
 */
char *
nmz_strerror(enum nmz_stat errnum)
{
    char *msg = NULL;

    switch (errnum) {
    case ERR_FATAL:
	msg = _("Fatal error occered!");
	break;
    case ERR_TOO_LONG_QUERY:
        msg = _("Too long query");
	break;
    case ERR_INVALID_QUERY:
	msg = _("Invalid query");
	break;
    case ERR_TOO_MANY_TOKENS:
	msg = _("Too many query tokens");
	break;
    case  ERR_TOO_MUCH_MATCH:
	msg = _("Too many words matched. Ignored");
	break;
    case ERR_TOO_MUCH_HIT:
	msg = _("Too many pages hit. Ignored");
	break;
    case ERR_REGEX_SEARCH_FAILED:
	msg = _("cannot open regex index");
	break;
    case ERR_PHRASE_SEARCH_FAILED:
	msg = _("cannot open phrase index");
	break;
    case ERR_FIELD_SEARCH_FAILED:
	msg = _("cannot open field index");
	break;
    case ERR_CANNOT_OPEN_INDEX:
	msg = _("cannot open this index");
	break;
    case ERR_CANNOT_OPEN_RESULT_FORMAT_FILE:
	msg = _("cannot open result format file");
	break;
    case ERR_NO_PERMISSION:
	msg = _("You don\'t have a permission to access the index");
	break;
    default:
	msg = _("Unknown error. Report bug!");
	break;
    } 

    assert(msg != NULL);

    return msg;
}

/*
 * Check wheter the file fname exists or not.
 */
int
nmz_is_file_exists(const char *fname)
{
    struct stat fstatus;

    return stat(fname, &fstatus) == 0;
}


