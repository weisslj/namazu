/*
 * 
 * codeconv.c -
 * 
 * $Id: codeconv.c,v 1.33 2004-02-20 20:21:31 opengl2772 Exp $
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
 * 
 */

/*
 * Kanji encoding converting routine
 * this routine refers source code of K.Sato-san's QKC [1997-08-31]
 * (Quick KANJI code Converter  C Version 1.0) 
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#ifdef HAVE_SUPPORT_H
#  include "support.h"
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

/* #define TEST_UTF8 */

#ifdef TEST_UTF8
#    include <iconv.h>
#endif /* TEST_UTF8 */

#include "libnamazu.h"
#include "codeconv.h"
#include "util.h"
#include "i18n.h"
#include "l10n-ja.h"

static uchar kanji2nd;

#ifdef TEST_UTF8
static int nmz_codeconv_internal_EX(char *buffer, int bufferSize);
#endif /* TEST_UTF8 */

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


/*
 *
 * Private functions
 *
 */

static uchar jmstojis ( uchar c1, uchar c2 );
static void jistoeuc ( uchar * s );
static void sjistoeuc ( uchar * s );
static uchar jistojms ( uchar c1, uchar c2 );
static void euctosjis ( uchar *s );
static void euctojis ( uchar *p );
static void zen2han ( char *str );

static uchar 
jmstojis(uchar c1, uchar c2)
{
    c1 -= (c1 <= 0x9f) ? 0x70 : 0xb0;
    c1 <<= 1;
    if (c2 < 0x9f) {
	c2 -= (c2 < 0x7f) ? 0x1f : 0x20;
	c1--;
    } else
	c2 -= 0x7e;
    kanji2nd = c2;
    return c1;
}


static void 
jistoeuc(uchar * s)
{
    uchar c, c2;
    int state, i = 0, j = 0;

    state = 0;
    if (!(c = *(s + j++)))
	return;
    while (1) {
	if (c == ESC) {
	    if (!(c = *(s + j++))) {
		*(s + i) = ESC;
		return;
	    }
	    switch (c) {
	    case '$':
		if (!(c2 = *(s + j++))) {
		    *(s + i++) = ESC;
		    *(s + i) = c;
		    return;
		}
		if (c2 == 'B' || c2 == '@')
		    state = 1;
		else {
		    *(s + i++) = ESC;
		    *(s + i++) = c;
		    c = c2;
		    continue;
		}
		break;

	    case '(':
		if (!(c2 = *(s + j++))) {
		    *(s + i++) = ESC;
		    *(s + i) = c;
		    return;
		}
		if (c2 == 'J' || c2 == 'B' || c2 == 'H') {
		    state = 0;
		} else {
		    *(s + i++) = ESC;
		    *(s + i++) = c;
		    c = c2;
		    continue;
		}
		break;

	    case 'K':
		state = 1;
		break;

	    case 'H':
		state = 0;
		break;

	    default:
		*(s + i++) = c;
		continue;
	    }
	} else if (c <= 0x20 || c == 0x7f) {
	    *(s + i++) = c;
	} else if (state) {
	    if (!(c2 = *(s + j++))) {
		*(s + i++) = c;
		*(s + i) = '\0';
		return;
	    }
	    if (c2 <= 0x20 || c2 == 0x7f) {
		*(s + i++) = c;
		c = c2;
		continue;
	    }
	    *(s + i++) = c | 0x80;
	    *(s + i++) = c2 | 0x80;
	} else {
	    *(s + i++) = c;
	}
	if (!(c = *(s + j++))) {
	    *(s + i) = '\0';
	    return;
	}
    }
}

static void 
sjistoeuc(uchar * s)
{
    uchar c, c2;
    int i = 0, j = 0;

    if (!(c = *(s + j++)))
	return;
    while (1) {
	if (c < 0x80)
	    *(s + i++) = c;
	else if (iskanji1st(c)) {
	    if (!(c2 = *(s + j++))) {
		*(s + i) = c;
		return;
	    }
	    if (iskanji2nd(c2)) {
		c = jmstojis(c, c2);
		*(s + i++) = (c | 0x80);
		*(s + i++) = (kanji2nd | 0x80);
	    } else {
		*(s + i++) = c;
		*(s + i++) = c2;
	    }
	} else
	    *(s + i++) = c;
	if (!(c = *(s + j++)))
	    return;
    }
}

static uchar 
jistojms(uchar c1, uchar c2)
{
    if (c1 & 1) {
	c1 = (c1 >> 1) + 0x71;
	c2 += 0x1f;
	if (c2 >= 0x7f)
	    c2++;
    } else {
	c1 = (c1 >> 1) + 0x70;
	c2 += 0x7e;
    }
    if (c1 > 0x9f)
	c1 += 0x40;
    kanji2nd = c2;
    return c1;
}

static void 
euctosjis(uchar *s)
{
    uchar c, c2;
    int i = 0, j = 0;

    while ((int)(c = *(s + j++))) {
	if (nmz_iseuc(c)) {
	    /* G1 for JIS X 0208 KANJI */
	    if (!(c2 = *(s + j++))) {
		*(s + i++) = c;
		break;
	    }
	    if (nmz_iseuc(c2)) {
		c = jistojms(c & 0x7f, c2 & 0x7f);
		*(s + i++) = c;
		*(s + i++) = kanji2nd;
	    } else {
		*(s + i++) = c;
		*(s + i++) = c2;
	    }
	} else if (nmz_iseuc_kana1st(c)) {
	    /* G2 for JIS X 0201 KATAKANA */
	    if (!(c2 = *(s + j++))) {
		*(s + i++) = c;
		break;
	    }
	    *(s + i++) = c2;
	} else if (nmz_iseuc_hojo1st(c)) {
	    /* G3 for JIS X 0212 HOJO KANJI */
	    if (!(c2 = *(s + j++))) {
		*(s + i++) = c;
		break;
	    }
	    *(s + i++) = 0x81;
	    if (!(c2 = *(s + j++)))
		break;
	    *(s + i++) = 0xac;
	} else
	    /* G0 for ASCII or JIS X 0201 ROMAN SET */
	    *(s + i++) = c;
    }
    *(s + i) = 0;
}

/*
 * FIXME: This function give no consideration for buffer overflow, 
 * so you should prepare enough memory for `p'.
 * In the future, code conversion functions will be replaced with iconv(3)
 */
static void 
euctojis(uchar *p)
{
    int c, c2, state = 0;
    uchar *alloc, *s;
    
    alloc = (uchar *)strdup((char *)p);
    s = alloc;
    if (s == NULL) { /* */
	nmz_set_dyingmsg(nmz_msg("%s", strerror(errno)));
	return;
    }

    if (!(c = (int) *(s++))) {
	return;
    }
    while (1) {
	if (c < 0x80) {
	    if (state) {
		*p = ESC; p++; *p = '('; p++; *p = 'B'; p++;
		state = 0;
	    }
	    *p = c;
	    p++;
	} else if (nmz_iseuc(c)) {
	    if (!(c2 = (int) *(s++))) {
		*p = c;
		p++;
		break;
	    }
	    if (!state) {
		*p = ESC; p++; *p = '$'; p++; *p = 'B'; p++;
		state = 1;
	    }
	    if (nmz_iseuc(c2)) {
		*p = c & 0x7f;
		p++;
		*p = c2 & 0x7f;
		p++;
	    } else {
		*p = c;
		p++;
		*p = ESC; p++; *p = '('; p++; *p = 'B'; p++;
		state = 0;
		*p = c2;
		p++;
	    }
	} else {
	    if (state) {
		*p = ESC; p++; *p = '('; p++; *p = 'B'; p++;
		state = 0;
	    }
	    *p = c;
	    p++;
	}
	if (!(c = (int) *(s++))) {
	    if (state) {
		*p = ESC; p++; *p = '('; p++; *p = 'B'; p++;
	    }
	    *p = '\0';
	    break;
	}
    }
    *p = '\0';
    free(alloc);
}


/*
 * Converting 2 bytes Alnum and Symbol into 1 byte one.
 * This code was contributed by Akira Yamada <akira@linux.or.jp> [1997-09-28]
 */
static char Z2H[] = 
"\0 \0\0,.\0:;?!\0\0'`\0^~_\0\0\0\0\0\0\0\0\0\0\0\0/\\\0\0|\0\0`'\"\"()\0\0[]{}<>\0\0\0\0\0\0\0\0+-\0\0\0=\0<>\0\0\0\0\0\0\0'\"\0\\$\0\0%#&*@";

static void 
zen2han(char *str)
{
    int p = 0, q = 0, r;
    uchar *s;
    
    s = (uchar *)str;

    while (*(s + p)) {
	if (*(s + p) == 0xa1) {
	    r = *(s + p + 1) - 0xa0;
	    if (r <= sizeof(Z2H) && Z2H[r] != '\0') {
		p++;
		*(s + p) = Z2H[r];
	    } else {
		*(s + q) = *(s + p);
		q++;
		p++;
	    }
	} else if (*(s + p) == 0xa3) {
	    p++;
	    *(s + p) = *(s + p) - 128;
	} else if (*(s + p) & 0x80) {
	    *(s + q) = *(s + p);
	    p++;
	    q++;
	}
	*(s + q) = *(s + p);
	p++;
	q++;
    }
    *(s + q) = '\0';
}


/*
 *
 * Public functions
 *
 */


/*
 * Detect character encoding (with simple approach) 
 * and convert "in" into EUC-JP
 * Supported encodings: EUC-JP, ISO-2022-JP, Shift_JIS
 */
int 
nmz_codeconv_internal(char *s)
{
    int i, m, n, f;
    uchar *in;

    in = (uchar *)s;

    if (!nmz_is_lang_ja()) { /* Lang != ja */
        return 0;
    }
    for (i = 0, m = 0, n = 0, f = 0; *(in + i); i++) {
	if (*(in + i) == ESC) {
	    jistoeuc(in);
	    return 1;
	}
	if (*(in + i) > (uchar) '\x80')
	    m++, f = f ? 0 : 1;
	else if (f) {
	    sjistoeuc(in);
	    return 1;
	}
	if (*(in + i) > (uchar) '\xa0')
	    n++;
    }
    if (m != n) {
	sjistoeuc(in);
	return 1;
    }
    if (n)
	return 1;
    return 0;
}

/*
 * Convert character encoding from internal one to external one.
 * Return a pointer of converted string.
 *
 * NOTES: Current internal encoding is EUC-JP for Japanese and
 * ISO-8859-* for others. In future, internal encoding of Namazu 
 * will be UTF-8.
 *
 */
char *
nmz_codeconv_external (const char *str) {
    char *tmp, *lang;

    tmp = strdup(str);
    if (tmp == NULL) {
	nmz_set_dyingmsg(nmz_msg("%s", strerror(errno)));
	return NULL;
    }

    lang = nmz_get_lang();
    if (strcasecmp(lang, "japanese") == 0 || 
	strcasecmp(lang, "ja") == 0 || 
	strcasecmp(lang, "ja_JP.EUC") == 0 || 
	strcasecmp(lang, "ja_JP.ujis") == 0 ||
	strcasecmp(lang, "ja_JP.eucJP") == 0)  /* EUC-JP */
    {
	;
    } else if (strcasecmp(lang, "ja_JP.SJIS") == 0) { /* Shift_JIS */
	euctosjis((uchar *)tmp);
    } else if (strcasecmp(lang, "ja_JP.ISO-2022-JP") == 0) { /* ISO-2022-JP */
	/*
	 * Prepare enough memory for ISO-2022-JP encoding.
	 * FIXME: It's not space-efficient. In the future, 
	 * code conversion functions will be replaced with iconv(3)
	 */
	tmp = realloc(tmp, strlen(str) * 5);
	if (tmp == NULL) {
	    nmz_set_dyingmsg(nmz_msg("%s", strerror(errno)));
	    return NULL;
	}
	euctojis((uchar *)tmp);
    }
    return (char *)tmp;
}

void 
nmz_codeconv_query(char *query)
{
    if (nmz_is_lang_ja()) {
#ifdef TEST_UTF8
		/* BUFSIZE = size of query */
        if (nmz_codeconv_internal_EX(query, BUFSIZE)) {
            zen2han(query);
            return;
        }
#endif /* TEST_UTF8 */
        if (nmz_codeconv_internal(query)) {
            zen2han(query);
        }
    }
}

#ifdef TEST_UTF8

enum code_type {
    CODE_TYPE_ERROR = -1,
    CODE_TYPE_UNKNOWN = 0,
    CODE_TYPE_JIS,
    CODE_TYPE_UTF8,
    CODE_TYPE_EUCJP,
    CODE_TYPE_SJIS
};

static int 
nmz_codeconv_internal_EX(char *buffer, int bufferSize)
{
    int i, j;
    char *bufferJIS;
    char *bufferUTF8;
    char *bufferSJIS;

    typedef struct {
        enum code_type type;
        int length, bytes;
    } CODE_DATA;

    CODE_DATA data[3], _data[3];
    int count;


    if ((bufferJIS = calloc(sizeof(char), bufferSize)) == NULL) {
	nmz_set_dyingmsg(nmz_msg("%s", strerror(errno)));
        return 0;
    }

    /******************************************/
    /* ISO-2022-JP                            */
    /******************************************/
    {
        strncpy(bufferJIS, buffer, bufferSize);
        if (nmz_from_to(bufferJIS, bufferSize, "ISO-2022-JP", "EUC-JP")) {
            strncpy(buffer, bufferJIS, bufferSize);
            free(bufferJIS);
            return 1;
        }
    }

    if ((bufferSJIS = calloc(sizeof(char), bufferSize)) == NULL) {
	nmz_set_dyingmsg(nmz_msg("%s", strerror(errno)));
        free(bufferJIS);
        return 0;
    }
    bufferUTF8 = bufferJIS;
    bufferUTF8[bufferSize - 1] = '\0';

    /******************************************/
    /* init                                   */
    /******************************************/
    for(i = 0; i < 3; i++) {
       data[i].length = 0;
       data[i].bytes = 0;
    }
    data[0].type = CODE_TYPE_UTF8;
    data[1].type = CODE_TYPE_EUCJP;
    data[2].type = CODE_TYPE_SJIS;

    /******************************************/
    /* UTF-8                                  */
    /******************************************/
    strncpy(bufferUTF8, buffer, bufferSize);
    if (nmz_from_to(bufferUTF8, bufferSize, "UTF-8", "EUC-JP")) {
        data[0].length = nmz_lengthEUCJP(bufferUTF8, strlen(bufferUTF8));
        if (data[0].length != 0) {
            data[0].bytes = strlen(bufferUTF8);
        }
    }

    /******************************************/
    /* EUC-JP                                 */
    /******************************************/
    data[1].length = nmz_lengthEUCJP(buffer, strlen(buffer));
    if (data[1].length) {
        data[1].bytes = strlen(buffer);
    }

    /******************************************/
    /* SHIFT_JIS                              */
    /******************************************/
    strncpy(bufferSJIS, buffer, bufferSize);
    if (nmz_from_to(bufferSJIS, bufferSize, "SHIFT_JIS", "EUC-JP")) {
        data[2].length = nmz_lengthEUCJP(bufferSJIS, strlen(bufferSJIS));
        if (data[2].length != 0) {
            data[2].bytes = strlen(bufferSJIS);
        }
    }

    /******************************************/

    for(i = 0 ; i < 3 - 1; i++) {
        for(j = i + 1 ; j < 3; j++) {
            if (data[i].length > data[j].length) {
                /* swap */
                CODE_DATA temp;

                temp = data[i];
                data[i] = data[j];
                data[j] = temp;
            }
        }
    }

    count = 0;
    for(i = 0; i < 3; i++) {
        if (data[i].length == 0) {
            continue;
        }
        _data[count++] = data[i];
    }

    if (count == 0) {
        free(bufferUTF8);
        free(bufferSJIS);
        return 0;
    }

    if (count >= 2) {
        if (_data[0].length == _data[1].length) {
            if (data[0].bytes > data[1].bytes) {
                _data[0] = _data[1];
            }
        }
    }

    /******************************************/

    if (_data[0].type == CODE_TYPE_UTF8) {
        strncpy(buffer, bufferUTF8, bufferSize);
    } else if (_data[0].type == CODE_TYPE_SJIS) {
        strncpy(buffer, bufferSJIS, bufferSize);
    }

    free(bufferUTF8);
    free(bufferSJIS);

    return 1;
}

char *
nmz_from_to(char *buffer, int bufferSize,
    const char *fromCode, const char *toCode)
{
    iconv_t cd;
    size_t sz_from, sz_to;
    char *toBuffer = NULL;
    char *from, *to;
    size_t status;

    if (!buffer || bufferSize <= 0) {
        return NULL;
    }

    sz_from = strlen(buffer) + 1;
    sz_to = bufferSize;

    toBuffer = (char *)calloc(sizeof(char), sz_to);
    if (!toBuffer) {
	    nmz_set_dyingmsg(nmz_msg("%s", strerror(errno)));
        return NULL;
    }

    from = buffer;
    to = toBuffer;

    cd = iconv_open(toCode, fromCode);

    status = iconv(cd, &from, &sz_from, &to, &sz_to);

    iconv_close(cd);

    if (status == -1) {
        if (toBuffer) {
            free(toBuffer);
        }
        return NULL;
    }

    strncpy(buffer, toBuffer, bufferSize - 1);
    buffer[bufferSize - 1] = '\0';

    free(toBuffer);

    return buffer;
}

char *
nmz_codeconv(const char *fromCode, char *fromBuffer, int fromBufferSize,
    const char *toCode, char *toBuffer, int toBufferSize)
{
    iconv_t cd;
    size_t sz_from, sz_to;
    char *from, *to;
    size_t status;

    if (!fromBuffer || fromBufferSize <= 0
    || !toBuffer || toBufferSize <= 0) {
        return NULL;
    }

    sz_from = strlen(fromBuffer) + 1;
    sz_to = toBufferSize;

    from = fromBuffer;
    to = toBuffer;

    cd = iconv_open(toCode, fromCode);

    status = iconv(cd, &from, &sz_from, &to, &sz_to);

    iconv_close(cd);

    if (status == -1) {
        return NULL;
    }

    toBuffer[toBufferSize - 1] = '\0';

    return toBuffer;
}

int 
nmz_lengthEUCJP(const char *str, int length)
{
   int i, mode = 0;
   unsigned char ch;
   int count = 0;

   for(i = 0; i < length; i++) {
       ch = (unsigned char)str[i];
       if (mode == 0) {
           count++;
           if (ch >= 0xa1 && ch <= 0xfe) {
               mode = 3;
           } else if (ch == 0x8e) {
               mode = 1;
           } else if (ch == 0x8f) {
               mode = 2;
           } else if (ch >= 0x80) {
               return 0;
           }
       } else if (mode == 1) {              /* 2bytes char kana */
           if (ch >= 0xa1 && ch <= 0xdf) {
               mode = 0;
           } else {
               return 0;
           }
       } else if (mode == 2 || mode == 3) { /* 2,3bytes char */
           if (ch >= 0xa1 && ch <= 0xfe) {
               mode++;
               if (mode == 4) {
                   mode = 0;
               }
           } else {
               return 0;
           }
       }
   }

   if (mode != 0) {
       return 0;
   }

   return count;
}

#endif /* TEST_UTF8 */
