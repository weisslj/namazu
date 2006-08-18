/*
 * 
 * codeconv.c -
 * 
 * $Id: codeconv.c,v 1.37 2006-08-18 18:56:03 opengl2772 Exp $
 * 
 * Copyright (C) 1997-1999 Satoru Takabayashi All rights reserved.
 * Copyright (C) 2000,2004 Namazu Project All rights reserved.
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

#include <iconv.h>

#include "libnamazu.h"
#include "codeconv.h"
#include "util.h"
#include "i18n.h"
#include "l10n-ja.h"

static int nmz_codeconv_jp(char *buffer, int bufferSize);


/*
 *
 * Private functions
 *
 */

static void utf8_zen2han ( char *str );

static void
utf8_zen2han(char *str)
{
    int p = 0, q = 0;
    uchar *s;
    
    s = (uchar *)str;

    while (*(s + p)) {
	if (*(s + p) == 0xe3 && *(s + p + 1) == 0x80 && *(s + p + 2) == 0x80) {
	    p += 2;
	    *(s + p) = 0x20;
	} else if (*(s + p) == 0xef && *(s + p + 1) == 0xbc && 
		   (*(s + p + 2) >= 0x81 || *(s + p + 2) <= 0xbf)){
	    p += 2;
	    *(s + p) = *(s + p) - 0x60;
	} else if (*(s + p) == 0xef && *(s + p + 1) == 0xbd && 
		   (*(s + p + 2) >= 0x80 || *(s + p + 2) <= 0x9d)){
	    p += 2;
	    *(s + p) = *(s + p) - 0x20;
	} else if (*(s + p) == 0xef){
	    *(s + q) = *(s + p);
	    p++;
	    q++;
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
 * Convert character encoding from internal one to external one.
 * Return a pointer of converted string.
 *
 * NOTES: Current internal encoding is UTF-8.
 *
 */
char *
nmz_codeconv_external (const char *str) {
    char *tmp, *lang;
    int tmpsize;

    tmp = strdup(str);
    if (tmp == NULL) {
	nmz_set_dyingmsg(nmz_msg("%s", strerror(errno)));
	return NULL;
    }
    tmpsize = strlen(tmp)  + 1;

    lang = nmz_get_lang();
    if (strcasecmp(lang, "japanese") == 0 || 
	strcasecmp(lang, "ja") == 0 || 
	strcasecmp(lang, "ja_JP.EUC") == 0 || 
	strcasecmp(lang, "ja_JP.ujis") == 0 ||
	strcasecmp(lang, "ja_JP.eucJP") == 0)  /* EUC-JP */
    {
        nmz_from_to(tmp, tmpsize, "UTF-8", "EUC-JP");
    } else if (strcasecmp(lang, "ja_JP.SJIS") == 0) { /* Shift_JIS */
        nmz_from_to(tmp, tmpsize, "UTF-8", "SHIFT_JIS");
    } else if (strcasecmp(lang, "ja_JP.ISO-2022-JP") == 0) { /* ISO-2022-JP */
	/*
	 * Prepare enough memory for ISO-2022-JP encoding.
	 * FIXME: It's not space-efficient. In the future, 
	 * code conversion functions will be replaced with iconv(3)
	 */
        tmpsize = strlen(tmp) * 5  + 1;
	tmp = realloc(tmp, tmpsize);
	if (tmp == NULL) {
	    nmz_set_dyingmsg(nmz_msg("%s", strerror(errno)));
	    return NULL;
	}
        nmz_from_to(tmp, tmpsize, "UTF-8", "ISO-2022-JP");
    } else if  (strcasecmp(lang, "deutsch") == 0 || 
	strcasecmp(lang, "german") == 0 || 
	strcasecmp(lang, "de") == 0 || 
	strcasecmp(lang, "de_DE.ISO-8859-1") == 0 || 

	strcasecmp(lang, "french") == 0 || 
	strcasecmp(lang, "fr") == 0 || 
	strcasecmp(lang, "fr_FR.ISO-8859-1") == 0 || 

	strcasecmp(lang, "spanish") == 0 || 
	strcasecmp(lang, "es") == 0 || 
	strcasecmp(lang, "es_ES.ISO-8859-1") == 0)  /* ISO-8859-1 */
    {
        nmz_from_to(tmp, tmpsize, "UTF-8", "ISO-8859-1");
    } else if  (strcasecmp(lang, "polish") == 0 || 
	strcasecmp(lang, "german") == 0 || 
	strcasecmp(lang, "pl") == 0 || 
	strcasecmp(lang, "pl_PL.ISO-8859-2") == 0)  /* ISO-8859-2 */
    {
	nmz_from_to(tmp, tmpsize, "UTF-8", "ISO-8859-2");
    }

    return (char *)tmp;
}

void 
nmz_codeconv_query(char *query)
{
    if (nmz_is_lang_ja()) {
        if (nmz_codeconv_jp(query, BUFSIZE)) {
            utf8_zen2han(query);
            return;
        }
    }
}

enum code_type {
    ERROR = -1,
    TYPE_UNKNOWN = 0,
    TYPE_JIS,
    TYPE_UTF8,
    TYPE_EUCJP,
    TYPE_SJIS
};

static int 
nmz_codeconv_jp(char *buffer, int bufferSize)
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
    enum code_type resultType = TYPE_UNKNOWN;
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
        if (nmz_from_to(bufferJIS, bufferSize, "ISO-2022-JP", "UTF-8")) {
            strncpy(buffer, bufferJIS, bufferSize);
            free(bufferJIS);
            return 1;
        }
    }

    bufferUTF8 = bufferJIS;
    if ((bufferSJIS = calloc(sizeof(char), bufferSize)) == NULL) {
        free(bufferUTF8);
	nmz_set_dyingmsg(nmz_msg("%s", strerror(errno)));
        return 0;
    }
    bufferUTF8[bufferSize - 1] = '\0';

    /******************************************/
    /* init                                   */
    /******************************************/
    for(i = 0; i < 3; i++) {
       data[i].length = 0;
       data[i].bytes = 0;
    }
    data[0].type = TYPE_UTF8;
    data[1].type = TYPE_EUCJP;
    data[2].type = TYPE_SJIS;

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

    resultType = _data[0].type;

    /******************************************/

    if (resultType == TYPE_EUCJP) {
        nmz_from_to(buffer, bufferSize, "EUC-JP", "UTF-8"); 
    } else if (resultType == TYPE_SJIS) {
        nmz_from_to(buffer, bufferSize, "SHIFT_JIS", "UTF-8"); 
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

    errno = 0;
    status = iconv(cd, &from, &sz_from, &to, &sz_to);

    iconv_close(cd);

    if (status == -1) {
        if (toBuffer) {
            free(toBuffer);
        }
	switch (errno) {
	    case E2BIG:
		nmz_debug_printf("iconv ERR E2BIG\n");
		break;
	    case EILSEQ:
		nmz_debug_printf("iconv ERR EILSEQ\n");
		break;
	    case EINVAL:
		nmz_debug_printf("iconv ERR EINVAL\n");
		break;

	}
        return NULL;
    }
    strncpy(buffer, toBuffer, bufferSize - 1);

    buffer[bufferSize - 1] = '\0';

    free(toBuffer);

    return buffer;
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
           if ((ch >= 0xa1 && ch <= 0xfe) || ch == 0x8e) {
               mode = 1;
           } else if (ch == 0x8f) {
               mode = 2;
           } else if (ch >= 0x80) {
               return 0;
           }
       } else if (mode == 1) {              /* 2bytes char */
           if (ch >= 0xa1 && ch <= 0xfe) {
               mode = 0;
           } else {
               return 0;
           }
       } else if (mode == 2 || mode == 3) { /* 3bytes char */
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

