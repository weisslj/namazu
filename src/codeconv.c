/*
 * 
 * codeconv.c -
 * 
 * Copyright (C) 1997-1999 Satoru Takabayashi  All rights reserved.
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
 * This file must be encoded in EUC-JP encoding.
 * 
 */

/*
 * Kanji encoding converting routine
 * this routine refers source code of K.Sato-san's QKC [1997-08-31]
 * (Quick KANJI code Converter  C Version 1.0) 
 */

#include <stdio.h>
#include "namazu.h"

uchar kanji2nd;

#define ESC 0x1b
#define iskanji1st(c) (((c) >= 0x81 && (c)) <= 0x9f ||\
                      ((c) >= 0xe0 && (c) <= 0xfc))
#define iskanji2nd(c) ((c) >= 0x40 && (c) <= 0xfc && (c) != 0x7f)
#define iseuc(c)  ((c) >= 0xa1 && (c) <= 0xfe)


uchar jmstojis(uchar c1, uchar c2)
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


void jistoeuc(uchar * s)
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

void sjistoeuc(uchar * s)
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

#if	defined(_WIN32) || defined(__EMX__)

uchar jistojms(uchar c1, uchar c2)
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

void euctosjis(uchar *s)
{
    uchar c, c2;
    int i = 0, j = 0;

    if (!(c = *(s + j++)))
	return;

    while (1) {
	if (iseuc(c)) {
	    if (!(c2 = *(s + j++))) {
		*(s + i++) = c;
		return;
	    }
	    if (iseuc(c2)) {
		c = jistojms(c & 0x7f, c2 & 0x7f);
		*(s + i++) = c;
		*(s + i++) = kanji2nd;
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

#endif  /* (_WIN32 || __EMX__) */


/* Character encoding auto detection (simple approach) */
int codeconv(uchar * in)
{
    int i, m, n, f;

    if (!is_lang_ja(Lang)) { /* Lang != ja */
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
 * 2 bytes 英数字・記号を 1 byte に変換するルーチン 
 * このコードはやまだあきらさんに頂きました。 [1997-09-28]
 */
char Z2H[] = "\0 \0\0,.\0:;?!\0\0'`\0^~_\0\0\0\0\0\0\0\0\0\0\0\0/\\\0\0|\0\0`'\"\"()\0\0[]{}<>\0\0\0\0\0\0\0\0+-\0\0\0=\0<>\0\0\0\0\0\0\0'\"\0\\$\0\0%#&*@";
void zen2han(uchar * s)
{
    int p = 0, q = 0, r;
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

