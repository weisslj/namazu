/*
 * 
 * parser.c -
 * 
 * $Id: parser.c,v 1.5 1999-08-27 10:05:13 satoru Exp $
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
#include "hlist.h"
#include "search.h"
#include "parser.h"

static int Cp = 0; /* variable that saves current position of parser */

/************************************************************
 *
 * Private functions
 *
 ************************************************************/

int isoperator(uchar*);
HLIST factor(int*);
int andop(void);
HLIST term(void);
int orop(void);

/* check a character if metacharacter (operator) of not */
int isoperator(uchar * c)
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
        if (!Query.tab[Cp])
            return val;

        if (!strcmp(Query.tab[Cp], LP_STRING)) {
            Cp++;
            if (Query.tab[Cp] == NULL)
                return val;
            val = expr();
            if (Query.tab[Cp] == NULL)
                return val;
            if (!strcmp(Query.tab[Cp], RP_STRING))
                Cp++;
            break;
        } else if (!isoperator(Query.tab[Cp])) {
            val = do_search(Query.tab[Cp], val);
            /*  MSG_ERR_TOO_MUCH_MATCH;
                MSG_ERR_TOO_MUCH_HIT;  case */
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
    if (Query.tab[Cp] == NULL)
	return 0;
    if (!strcmp(Query.tab[Cp], AND_STRING) ||
	!strcmp(Query.tab[Cp], AND_STRING_ALT)) {
	Cp++;
	return AND_OP;
    }
    if (!strcmp(Query.tab[Cp], NOT_STRING) ||
	!strcmp(Query.tab[Cp], NOT_STRING_ALT)) {
	Cp++;
	return NOT_OP;
    }
    if (!strcmp(Query.tab[Cp], LP_STRING))
	return AND_OP;
    if (!isoperator(Query.tab[Cp]))
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
    if (Query.tab[Cp] == NULL)
	return 0;
    if (!strcmp(Query.tab[Cp], OR_STRING) ||
	!strcmp(Query.tab[Cp], OR_STRING_ALT)) {
	Cp++;
	return 1;
    }
    return 0;
}


/************************************************************
 *
 * Public functions
 *
 ************************************************************/

HLIST expr(void)
{
    HLIST left, right;

    left = term();
    while (orop()) {
	right = term();
	left = ormerge(left, right);
    }
    return left;
}

void init_parser(void)
{
    Cp = 0;
}
