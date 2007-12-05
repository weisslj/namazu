/*
 * 
 * codeconv.c -
 * 
 * $Id: codeconv.c,v 1.44 2007-12-05 15:56:35 opengl2772 Exp $
 * 
 * Copyright (C) 1997-1999 Satoru Takabayashi All rights reserved.
 * Copyright (C) 2000-2007 Namazu Project All rights reserved.
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#ifdef HAVE_SUPPORT_H
#  include "support.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_ERRNO_H
#  include <errno.h>
#endif

#ifdef HAVE_STRING_H
#  include <string.h>
#else
#  include <strings.h>
#endif

#include <iconv.h>

#include "libnamazu.h"
#include "codeconv.h"
#include "util.h"
#include "i18n.h"
#include "l10n-ja.h"


/*
 *
 * Private functions
 *
 */

static void utf8_zen2han ( char *str );

static int nmz_codeconv_jp(char *buffer, int bufferSize);


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

char *
nmz_get_external_encoding(void)
{
    char *env;
    static char *external_encoding = NULL;
    static char cache[BUFSIZE] = "";
    char buffer[BUFSIZE] = "";
    char *pLanguage, *pEncoding, *pModifier;
    char *p;


    if ((env = nmz_get_lang()) == NULL) {
        return NULL;
    }

    if (cache[0] != '\0' && !strcmp(cache, env)) {
        nmz_debug_printf("nmz_get_external_encoding: cache [%s] hit!\n", 
            external_encoding);
        return external_encoding;
    }

    strncpy(cache, env, BUFSIZE - 1);
    cache[BUFSIZE - 1] = '\0';

    strncpy(buffer, env, BUFSIZE - 1);
    buffer[BUFSIZE - 1] = '\0';

    pLanguage = buffer;
    pEncoding = pModifier = NULL;
    for(p = buffer; *p != '\0'; p++) {
        if (*p == '.') {
            *p = '\0';
            pEncoding = p + 1;
        } else if (*p == '@') {
            *p = '\0';
            pModifier = p + 1;
        }
    }

    external_encoding = NULL;
    if (pEncoding) {
        if (!strcasecmp(pEncoding, "utf8")) {
            external_encoding = "UTF-8";
        } else if (!strcasecmp(pEncoding, "eucJP") ||
            !strcasecmp(pEncoding, "ujis"))
        {
            external_encoding = "EUC-JP";
        } else if (!strcasecmp(pEncoding, "SJIS")) {
            external_encoding = "Shift_JIS";
        } else if (!strcasecmp(pEncoding, "ISO2022JP") ||
            !strcasecmp(pEncoding, "ISO-2022-JP"))
        {
            external_encoding = "ISO-2022-JP";
        } else if (!strcasecmp(pEncoding, "ISO88591") ||
            !strcasecmp(pEncoding, "ISO-8859-1")) 
        {
            external_encoding = "ISO-8859-1";
        } else if (!strcasecmp(pEncoding, "ISO885915") ||
            !strcasecmp(pEncoding, "ISO-8859-15")) 
        {
            external_encoding = "ISO-8859-15";
        } else if (!strcasecmp(pEncoding, "ISO88592") ||
            !strcasecmp(pEncoding, "ISO-8859-2")) 
        {
            external_encoding = "ISO-8859-2";
        }
    }

    if (external_encoding == NULL && pModifier) {
        if (!strcasecmp(pModifier, "euro")) {   /* xx_XX@euro */
            external_encoding = "ISO-8859-15";
        }
    }

    if (external_encoding == NULL && pLanguage) {
        if (!strncasecmp(pLanguage, "ja_", 3) ||
            !strcasecmp(pLanguage, "ja") ||
            !strcasecmp(pLanguage, "japanese"))
        {
            external_encoding = "EUC-JP";
        } else if (!strncasecmp(pLanguage, "en_", 3) ||
            !strcasecmp(pLanguage, "en") ||

            !strncasecmp(pLanguage, "fr_", 3) ||
            !strcasecmp(pLanguage, "fr") ||
            !strcasecmp(pLanguage, "french") ||

            !strncasecmp(pLanguage, "de_", 3) ||
            !strcasecmp(pLanguage, "de") ||
            !strcasecmp(pLanguage, "deutsch") ||
            !strcasecmp(pLanguage, "german") ||

            !strncasecmp(pLanguage, "es_", 3) ||
            !strcasecmp(pLanguage, "es") ||
            !strcasecmp(pLanguage, "spanish"))
        {
            external_encoding = "ISO-8859-1";
        } else if (!strncasecmp(pLanguage, "pl_", 3) ||
            !strcasecmp(pLanguage, "pl") ||
            !strcasecmp(pLanguage, "polish"))
        {
            external_encoding = "ISO-8859-2";
        }
    }

    return external_encoding;
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
nmz_codeconv_external (const char *str)
{
    char *tmp, *encoding;

    tmp = strdup(str);
    if (tmp == NULL) {
	nmz_set_dyingmsg(nmz_msg("%s", strerror(errno)));
	return NULL;
    }

    if ((encoding = nmz_get_external_encoding())) {
        int tmpsize;

        if (!strcmp(encoding, "UTF-8")) {
            ;
        } else if (!strcmp(encoding, "ISO-2022-JP")) {
	    /*
	     * Prepare enough memory for ISO-2022-JP encoding.
	     * FIXME: It's not space-efficient.
	     */
            tmpsize = strlen(tmp) * 5 + 1;
	    tmp = realloc(tmp, tmpsize);
	    if (tmp == NULL) {
	        nmz_set_dyingmsg(nmz_msg("%s", strerror(errno)));
	        return NULL;
	    }
            nmz_from_to(tmp, tmpsize, "UTF-8", "ISO-2022-JP");
        } else {
            tmpsize = strlen(tmp) + 1;
            nmz_from_to(tmp, tmpsize, "UTF-8", encoding);
        }
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
	    default:
		nmz_debug_printf("iconv ERR UNKNOWN\n");
		break;
	}
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

