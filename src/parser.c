/*
 * 
 * parser.c -
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
 * recursive parser: expr, orop, term, andop, fator
 * original idea came from Programming Perl 1st edtion
 */

#include <stdio.h>
#include <string.h>
#include "namazu.h"

/* definitions of operator */
#define AND_STRING "&"
#define OR_STRING  "|"
#define NOT_STRING "!"
#define LP_STRING  "("
#define RP_STRING  ")"

/* also acceptable as word since v1.1.1 */
#define AND_STRING_ALT "and"
#define OR_STRING_ALT  "or"
#define NOT_STRING_ALT "not"

#define AND_OP 1
#define NOT_OP 2

static int Cp = 0; /* variable that saves current position of parser */


/* check a character if metacharacter (operator) of not */
int ismetastring(uchar * c)
{
    if ((!strcmp(c, AND_STRING)) ||
	(!strcmp(c, AND_STRING_ALT)) ||
	(!strcmp(c, OR_STRING)) ||
	(!strcmp(c, OR_STRING_ALT)) ||
	(!strcmp(c, NOT_STRING)) ||
	(!strcmp(c, NOT_STRING_ALT)) ||
	(!strcmp(c, LP_STRING)) ||
	(!strcmp(c, RP_STRING)))
	return 1;
    return 0;
}


HLIST factor(int *ignore)
{
    HLIST val;
    val.n = 0;

    while (1) {
        if (!KeyItem[Cp])
            return val;

        if (!strcmp(KeyItem[Cp], LP_STRING)) {
            Cp++;
            if (KeyItem[Cp] == NULL)
                return val;
            val = expr();
            if (KeyItem[Cp] == NULL)
                return val;
            if (!strcmp(KeyItem[Cp], RP_STRING))
                Cp++;
            break;
        } else if (!ismetastring(KeyItem[Cp])) {
            val = do_search(KeyItem[Cp], val);
            /*  MSG_TOO_MUCH_MATCH;
                MSG_TOO_MUCH_HIT;  case */
            if (val.n < 0) {
                *ignore = 1;
                val.n = 0;   /* assign 0 - note that is important */
            }

            Cp++;
            break;
        } else {
            Cp++;
        }
    }
    return val;
}

int andop(void)
{
    if (KeyItem[Cp] == NULL)
	return 0;
    if (!strcmp(KeyItem[Cp], AND_STRING) ||
	!strcmp(KeyItem[Cp], AND_STRING_ALT)) {
	Cp++;
	return AND_OP;
    }
    if (!strcmp(KeyItem[Cp], NOT_STRING) ||
	!strcmp(KeyItem[Cp], NOT_STRING_ALT)) {
	Cp++;
	return NOT_OP;
    }
    if (!strcmp(KeyItem[Cp], LP_STRING))
	return AND_OP;
    if (!ismetastring(KeyItem[Cp]))
	return AND_OP;
    return 0;
}

HLIST term(void)
{
    HLIST left, right;
    int ignore = 0, op;

    left = factor(&ignore);
    while ((op = andop())) {
	right = factor(&ignore);
	if (op == AND_OP) {
	    left = andmerge(left, right, &ignore);
	} else if (op == NOT_OP) {
	    left = notmerge(left, right, &ignore);
	}
	ignore = 0;
    }
    return left;
}


int orop(void)
{
    if (KeyItem[Cp] == NULL)
	return 0;
    if (!strcmp(KeyItem[Cp], OR_STRING) ||
	!strcmp(KeyItem[Cp], OR_STRING_ALT)) {
	Cp++;
	return 1;
    }
    return 0;
}

HLIST expr()
{
    HLIST left, right;

    left = term();
    while (orop()) {
	right = term();
	left = ormerge(left, right);
    }
    return left;
}

void initialize_parser(void)
{
    Cp = 0;
}
