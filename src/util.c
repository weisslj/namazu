/*
   Imported scan_hex(), scan_oct(), xmalloc(), xrealloc() from 
   Ruby b19's"util.c" and "gc.c". Thanks to Matsumoto-san for consent!
   */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "namazu.h"

unsigned long
scan_oct(start, len, retlen)
char *start;
int len;
int *retlen;
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
scan_hex(start, len, retlen)
char *start;
int len;
int *retlen;
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
xmalloc(size)
    unsigned long size;
{
    void *mem;

    if (size == 0) size = 1;
    malloc_memories += size;
    mem = malloc(size);

    return mem;
}

void *
xrealloc(ptr, size)
    void *ptr;
    unsigned long size;
{
    void *mem;

    if (!ptr) return xmalloc(size);
    mem = realloc(ptr, size);
    return mem;
}



void tr(uchar *str, uchar f, uchar t)
{
    while (*str) {
        if (*str == f)
            *str = t;
        str++;
    }
}

/* delete ending LF and spaces of string*/
void chop(uchar * s)
{
    int i;
    for (i = strlen(s) - 1; i >= 0; i--) {
	if (*(s + i) == '\n' || *(s + i) == '\r'
            || *(s + i) == ' ' || *(s + i) == '\t')
	    *(s + i) = '\0';
	else
	    break;
    }
}


/* a machine has no memmove() such as SunOS 4.1.x */
#if !defined(HAVE_MEMMOVE)

void *memmove(void *d, void *s, size_t n)
{
    size_t i;

    if (s > d) {
        for (i = 0; i < n; i++) {
            *((char *)d + i) = *((char *)s + i);
        }
    } else {
        for (i = n - 1; ; i--) {
            *((char *)d + i) = *((char *)s + i);
            if (i == 0)
                break;
        }
    }
    return d;
}

#endif

/* decoding URI encoded strings */
void decode_uri_string(uchar * s)
{
    int i, j;
    for (i = j = 0; s[i]; i++, j++) {
	if (s[i] == '%') {
	    s[j] = URIdecode(s[i + 1], s[i + 2]);
	    i += 2;
	} else if (s[i] == '+') {
	    s[j] = ' ';
	} else {
	    s[j] = s[i];
	}
    }
    s[j] = '\0';
}

void tolower_string(uchar *str)
{
    while (*str) {
        *str = tolower(*str);
        str++;
    }
}

void delete_backslashes(uchar *str)
{
    uchar *pos = str;

    while (*str) {
        if (*str == '\\' && *(str + 1) == '\\') {
            *pos = *str;
            pos++;
            str++;
            str++;
        } else if (*str == '\\') {
            str++;
        } else {
            *pos = *str;
            pos++;
            str++;
        }
    }
    *pos = '\0';
}

/* return with pointer to character at the end of string */
uchar *lastc(uchar *str)
{
    return (str + strlen(str) - 1);
}

/* fread with endian consideration */
size_t freadx(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t value;

    value = fread(ptr, size, nmemb, stream);
    if (OppositeEndian && size == sizeof(int)) {
        reverse_byte_order(ptr, nmemb);
    }
    return value;
}

int get_unpackw(FILE *fp, int *data) {
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

int read_unpackw(FILE *fp, int *buf, int size) {
    int i = 0,  n = 0;
    
    while (i < size) {
	int tmp = get_unpackw(fp, &buf[n]);
	n++;
	if (tmp == 0) {  /* error */
	    break;
	} else {
	    i += tmp;
	}
    }
    return  n;
}

LIST *add_list(LIST *ptr, uchar *str)
{
    LIST *tmp;
    
    tmp = malloc(sizeof *tmp);
    if (tmp == NULL) {
	 error("add_list_malloc");
    }
    tmp->str = malloc(strlen(str) + 1);
    if (tmp->str == NULL) {
	 error("add_list_malloc");
    }

    strcpy(tmp->str, str);
    tmp->next = ptr;
    return tmp;
}

