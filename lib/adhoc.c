/*
 * 
 * adhoc.c - collection of very ad-hoc solutions :-p
 * 
 * $Id: adhoc.c,v 1.1 2000-08-20 21:19:33 rug Exp $
 * 
 * Copyright (C) 1989, 1990 Free Software Foundation, Inc.
 * Copyright (C) 1999 Satoru Takabayashi All rights reserved.
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

#include "adhoc.h"


/* 
 * Substitute for tolower(3).
 */

int 
adhoc_tolower(int c)
{
    if (c >= 'A' && c <= 'Z') {
	c = 'a' + c - 'A';
	return c;
    }
    return c;
}

/* 
 * Case-insensitive brute force search
 */

char *
adhoc_strcasestr (s1, s2)
     char *s1;
     char *s2;
{
    int i;
    char *p1;
    char *p2;
    char *s = s1;

    for (p2 = s2, i = 0; *s; p2 = s2, i++, s++) {
	for (p1 = s; *p1 && *p2 && (adhoc_tolower(*p1) == adhoc_tolower(*p2)); p1++, p2++)
	    ;
	if (!*p2)
	    break;
    }
    if (!*p2)
	return s1 + i;

    return 0;
}
