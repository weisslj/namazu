/*
   Imported scan_hex(), scan_oct(), xmalloc(), xrealloc() from 
   Ruby b19's"util.c" and "gc.c". Thanks to Matsumoto-san for consent!
   */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#ifdef __EMX__
#include <sys/types.h>
#endif
#include <sys/stat.h>
#include "namazu.h"
#include "util.h"

/************************************************************
 *
 * Private functions
 *
 ************************************************************/

/* reverse byte order */
static void reverse_byte_order (int *p, int n)
{
    int i, j;
    uchar *c, tmp;

    for (i = 0; i < n; i++) {
        c = (uchar *)(p + i);
        for (j = 0; j < (sizeof(int) / 2); j++) {
            tmp = *(c + j);
            *(c + j)= *(c + sizeof(int) - 1 - j);
            *(c + sizeof(int) - 1 - j) = tmp;
        }
    }
}

static int is_little_endian(void)
{
#ifdef WORDS_BIGENDIAN
    return 0;     /* big-endian */
#else
    return 1;     /* little-endian */
#endif

/*
    int  n = 1;
    char *c;
    
    c = (char *)&n;
    if (*c == 1) {
	return 1;
    } else {
	return 0;
    }
*/
}

/************************************************************
 *
 * Public functions
 *
 ************************************************************/

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



void tr(uchar *str, uchar *lstr, uchar *rstr)
{
    while (*str) {
	uchar *idx = strchr(lstr, *str);
	if (idx != NULL) { /* found */
	    *str = *(idx - lstr + rstr);
	}
        str++;
    }
}

/* delete ending LF and spaces of string*/
void chomp(uchar * s)
{
    int i;
    for (i = strlen(s) - 1; i >= 0; i--) {
	if (*(s + i) == '\n' || *(s + i) == '\r'
            || *(s + i) == ' ' || *(s + i) == '\t') {
	    *(s + i) = '\0';
	} else {
	    break;
	}
    }
}


void strlower(uchar *str)
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
    if (size == sizeof(int) && is_little_endian()) {
        reverse_byte_order(ptr, nmemb);
    }
    return value;
}

int get_unpackw(FILE *fp, int *data)
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

/* read index and return with value */
long getidxptr(FILE * fp, long p)
{
    int val;

    fseek(fp, p * sizeof(int), 0);
    freadx(&val, sizeof(int), 1, fp);
    return (long) val;
}

int issymbol(int c)
{
    if (c < 0x80 && !isalnum(c)) {
        return 1;
    } else {
	return 0;
    }
}

/* error messaging function */
void die(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    vsnprintf(Dyingmessage, BUFSIZE, fmt, args);
    va_end(args);

    diewithmsg();

    exit(2);
}

void diemsg(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    vsnprintf(Dyingmessage, BUFSIZE, fmt, args);
    va_end(args);
}

void diewithmsg()
{
    FILE *output;

    fflush(stderr);

    if (IsCGI) {
        output = stdout;
    } else {
        output = stderr;
    }

    if (HtmlOutput) {
	print(MSG_MIME_HEADER);
    }

    fprintf(output, "%s: ", PACKAGE);

    fprintf(output, "%s", Dyingmessage);

    fprintf(output, "\n");

    exit(2);
}

/* warning messaging function */
void warnf(char *fmt, ...)
{
    va_list args;

    fflush(stdout);

    fprintf(stderr, "%s: ", PACKAGE);

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fflush(stderr);
}

/* debug messaging function */
void debug_printf(char *fmt, ...)
{
    va_list args;

    if (Debug == 0) {
	return;
    }

    fflush(stdout);

    fprintf(stderr, "%s(debug): ", PACKAGE);

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fflush(stderr);
}

void pathcat(uchar *base, uchar *name)
{
    uchar work[BUFSIZE];
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

int isnumstr(char *str)
{
    int i, nonnum = 0;

    if (strlen(str) > 10) {  /* too large number */
	return 0;
    }

    for (i = 0; str[i] != '\0'; i++) {
	if (! isdigit((int)str[i])) {
	    nonnum = 1;
	    return 0;
	}
    }
    return 1;
}

void commas(char *str)
{
    int i, n;
    int leng = strlen(str);

    n = leng + (leng - 1) / 3;
    str[n] = '\0';
    n--;
    for (i = 0; i < leng; i++, n--) {
	str[n] = str[leng - 1 - i];
	if (i % 3 == 2 && n != 0) {
	    n--;
	    str[n] = ',';
	}
    }
}

/* 
 * case-insensitive brute force search  
 * (with consideration for EUC encoding schemes)
 */
uchar *strcasestr(uchar *haystack, uchar *needle)
{
    int n = strlen(needle);
    int euc_mode = 0;

    if (is_lang_ja()) {
	euc_mode = 1;
    }

    for (; *haystack != '\0'; haystack++) {
	if (strncasecmp(haystack, needle, n) == 0) {
	    return haystack;
	}
	if (euc_mode && iseuc(*haystack)) {
	    haystack++;
	}
    }
    return NULL;
}


int strprefixcasecmp(uchar *str1, uchar *str2)
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

int strsuffixcasecmp(uchar *str1, uchar *str2)
{
    int leng1, leng2;

    leng1 = strlen(str1);
    leng2 = strlen(str2);

    if (leng1 > leng2) {
	return strcasecmp(str1 + leng1 - leng2, str2);
    } else {					     
	return strcasecmp(str2 + leng2 - leng1, str1);
    }
}

int strprefixcmp(uchar *str1, uchar *str2)
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

int strsuffixcmp(uchar *str1, uchar *str2)
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

/* load the whole of file */
uchar *readfile(uchar *fname)
{
    uchar *buf;
    FILE *fp;
    struct stat fstatus;

    stat(fname, &fstatus);
    fp = fopen(fname, "rb");
    if (fp == NULL) {
        warnf("can't open %s\n", fname);
        return 0;
    }
    buf = (uchar *) malloc(fstatus.st_size + 1);
    if (buf == NULL) {
	 diemsg("readfile(malloc)");
	 return NULL;
    }
    if (fread(buf, sizeof(uchar), fstatus.st_size, fp) == 0) {
        diemsg("readfile(fread)");
	return NULL;
    }
    *(buf + fstatus.st_size) = '\0';
    fclose(fp);
    return buf;
}

/* subst: substitute pat with rep at without memory size consideration */
void subst(uchar *p, uchar *pat, uchar *rep)
{
    int patlen, replen;
    patlen = strlen(pat);
    replen = strlen(rep);

    if (patlen == replen) {
	memmove(p, rep, replen);
    } else if (patlen < replen) {
	/* + 1 for including '\0' */
	memmove(p + replen, p + patlen, strlen(p) - patlen + 1);
	memmove(p, rep, replen);
    } else if (patlen > replen) {
	memmove(p, rep, replen);
	/* + 1 for including '\0' */
	memmove(p + replen, p + patlen, strlen(p) - patlen + 1);
    }
}

/* output contents of file */
void cat(uchar *fname)
{
    uchar buf[BUFSIZE];
    FILE *fp;

    if ((fp = fopen(fname, "rb"))) {
	while (fgets(buf, BUFSIZE, fp)) {
	    fputs(buf, stdout);
	}
	fclose(fp);
    }
    warnf("can't open %s\n", fname);
}

char *safe_getenv(char *s)
{
    char *cp;
    return (cp = getenv(s)) ? cp : "";
}

void print(uchar *s) {
    fputs(s, stdout);
}

void wprint(uchar *s) {
    fflush(stdout);
    fprintf(stderr, "%s: ", PACKAGE);
    fputs(s, stderr);
    fflush(stderr);
}

