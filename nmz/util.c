/*
 * $Id: util.c,v 1.33 2000-01-05 10:30:45 satoru Exp $
 *
 * Imported scan_hex(), scan_oct(), xmalloc(), xrealloc() from 
 * Ruby b19's"util.c" and "gc.c". Thanks to Matsumoto-san for consent!
 *
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
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "namazu.h"
#include "libnamazu.h"
#include "message.h"
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
 * Delete ending LF and spaces of string.
 */
void 
nmz_chomp(char * s)
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
 * Read index and return with value.
 */
long 
nmz_getidxptr(FILE * fp, long p)
{
    int val;

    fseek(fp, p * sizeof(int), 0);
    nmz_fread(&val, sizeof(int), 1, fp);
    return (long) val;
}

int 
nmz_issymbol(int c)
{
    if (c >= 0x00 && c < 0x80 && !isalnum(c)) {
        return 1;
    } else {
	return 0;
    }
}

/* 
 * Error messaging function.
 */
void 
nmz_die(const char *fmt, ...)
{
    va_list args;

    fflush(stdout);
    fflush(stderr);

    fprintf(stderr, "%s: ", PACKAGE);

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "%s", get_dyingmsg());

    if (fmt[strlen(fmt) - 1] != '\n') {
	printf("\n");
    }

    exit(EXIT_FAILURE);
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

    if (!is_debugmode()) {
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
    int i, nonnum = 0;

    if (strlen(str) > 10) {  /* Too large number */
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

void 
nmz_commas(char *str)
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

void 
nmz_strlower(char *str)
{
    while (*str) {
	/* 
	 * Cannot use tolower(3) because of some system's
	 * poor implementention. maybe locale problem?
	 * [namazu-dev 1058]
	 */
        *str = nmz_tolower(*str);
        str++;
    }
}

/* 
 * Case-insensitive brute force search.
 * (with consideration for EUC encoding schemes)
 */
char *
nmz_strcasestr(const char *haystack, const char *needle)
{
    int n = strlen(needle);
    int euc_mode = 0;

    if (is_lang_ja()) {
	euc_mode = 1;
    }

    for (; *haystack != '\0'; haystack++) {
	if (strncasecmp(haystack, needle, n) == 0) {
	    return (char *)haystack;
	}
	if (euc_mode && iseuc(*haystack)) {
	    haystack++;
	}
    }
    return NULL;
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

    stat(fname, &fstatus);
    fp = fopen(fname, "rb");
    if (fp == NULL) {
        nmz_warn_printf("can't open %s\n", fname);
        return NULL;
    }
    buf = malloc(fstatus.st_size + 1);
    if (buf == NULL) {
	 set_dyingmsg("readfile(malloc)");
	 return NULL;
    }
    if (fread(buf, sizeof(char), fstatus.st_size, fp) == 0) {
        set_dyingmsg("readfile(fread)");
	return NULL;
    }
    *(buf + fstatus.st_size) = '\0';
    fclose(fp);
    return buf;
}

/* 
 * Substitute pat with rep in str at without memory size consideration.
 */
void 
nmz_subst(char *str, const char *pat, const char *rep)
{
    int patlen, replen;
    patlen = strlen(pat);
    replen = strlen(rep);

    if (patlen == replen) {
	memmove(str, rep, replen);
    } else if (patlen < replen) {
	/* + 1 for including '\0' */
	memmove(str + replen, str + patlen, strlen(str) - patlen + 1);
	memmove(str, rep, replen);
    } else if (patlen > replen) {
	memmove(str, rep, replen);
	/* + 1 for including '\0' */
	memmove(str + replen, str + patlen, strlen(str) - patlen + 1);
    }
}

/* 
 * Output contents of file.
 */
void 
nmz_cat(const char *fname)
{
    char buf[BUFSIZE];
    FILE *fp;

    if ((fp = fopen(fname, "rb"))) {
	while (fgets(buf, BUFSIZE, fp)) {
	    fputs(buf, stdout);
	}
	fclose(fp);
    }
    nmz_warn_printf("can't open %s\n", fname);
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
nmz_decode_uri(char * s)
{
    int i, j;
    for (i = j = 0; s[i]; i++, j++) {
	if (s[i] == '%') {
	    s[j] = decode_uri_sub(s[i + 1], s[i + 2]);
	    i += 2;
	} else if (s[i] == '+') {
	    s[j] = ' ';
	} else {
	    s[j] = s[i];
	}
    }
    s[j] = '\0';
}

char *
nmz_get_errmsg(enum nmz_stat stat)
{
    char *msg = "If you see this message, please report.";

    switch (stat) {
    case ERR_FATAL:
	msg = _("Fatal error occered!");
	break;
    case ERR_TOO_LONG_QUERY:
        msg = _(MSG_TOO_LONG_QUERY);
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
    case ERR_CANNOT_OPEN_INDEX:
	msg = _("cannot open this index");
	break;
    case ERR_NO_PERMISSION:
	msg = _("You don\'t have a permission to access the index");
	break;
    default:
	msg = _("Unknown error. Report bug!");
	break;
    } 

    return msg;
}

/*
 * FIXME: Tell me if you know more better function name (or better way).
 */
void 
nmz_print(const char *s) {
    fputs(s, stdout);
}


/*
 * The following functions are commented as a reminder.
 */

/*
 * void nmz_delete_backslashes(char *str)
 * {
 *     char *pos = str;
 * 
 *     while (*str) {
 *         if (*str == '\\' && *(str + 1) == '\\') {
 *             *pos = *str;
 *             pos++;
 *             str++;
 *             str++;
 *         } else if (*str == '\\') {
 *             str++;
 *         } else {
 *             *pos = *str;
 *             pos++;
 *             str++;
 *         }
 *     }
 *     *pos = '\0';
 * }
 */

/*
 * int nmz_strsuffixcasecmp(char *str1, char *str2)
 * {
 *     int leng1, leng2;
 * 
 *     leng1 = strlen(str1);
 *     leng2 = strlen(str2);
 * 
 *     if (leng1 > leng2) {
 * 	return strcasecmp(str1 + leng1 - leng2, str2);
 *     } else {					     
 * 	return strcasecmp(str2 + leng2 - leng1, str1);
 *     }
 * }
 */

