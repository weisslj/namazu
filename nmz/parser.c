/*
 * 
 * parser.c -
 * 
 * $Id: parser.c,v 1.5 1999-11-23 14:18:34 satoru Exp $
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
 * 
 */

/*
 * recursive parser: expr, orop, term, andop, fator
 * original idea came from Programming Perl 1st edtion
 */

#include <stdio.h>
#include <string.h>
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "namazu.h"
#include "libnamazu.h"
#include "hlist.h"
#include "search.h"
#include "parser.h"
#include "var.h"

static int Cp = 0; /* variable that saves current position of parser */

/*
 *
 * Private functions
 *
 */

static int isop(char*);
static HLIST factor(int*);
static int andop(void);
static HLIST term(void);
static int orop(void);

/* check a character if metacharacter (operator) of not */
static int isop(char * c)
{
    if ((strcmp(c, AND_STRING) == 0 ) ||
	(strcmp(c, AND_STRING_ALT) == 0 ) ||
	(strcmp(c, OR_STRING) == 0 ) ||
	(strcmp(c, OR_STRING_ALT) == 0 ) ||
	(strcmp(c, NOT_STRING) == 0 ) ||
	(strcmp(c, NOT_STRING_ALT) == 0 ) ||
	(strcmp(c, LP_STRING) == 0 ) ||
	(strcmp(c, RP_STRING) == 0 ))
	return 1;
    return 0;
}


static HLIST factor(int *ignore)
{
    HLIST val;
    val.n    = 0;
    val.stat = SUCCESS;

    while (1) {
        if (Query.tab[Cp] == 0) {
            return val;
	}

        if (strcmp(Query.tab[Cp], LP_STRING) == 0) {
            Cp++;
            if (Query.tab[Cp] == NULL)
                return val;
            val = expr();
	    if (val.stat == ERR_FATAL)
	        return val;
            if (Query.tab[Cp] == NULL)
                return val;
            if (strcmp(Query.tab[Cp], RP_STRING) == 0)
                Cp++;
            break;
        } else if (!isop(Query.tab[Cp])) {
            val = do_search(Query.tab[Cp], val);
	    if (val.stat == ERR_FATAL)
	       return val;
            /*  ERR_TOO_MUCH_MATCH;
                ERR_TOO_MUCH_HIT;  case */
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

static int andop(void)
{
    if (Query.tab[Cp] == NULL)
	return 0;
    if ((strcmp(Query.tab[Cp], AND_STRING) == 0) ||
	(strcmp(Query.tab[Cp], AND_STRING_ALT) ==0)) 
    {
	Cp++;
	return AND_OP;
    }
    if ((strcmp(Query.tab[Cp], NOT_STRING) == 0)||
	(strcmp(Query.tab[Cp], NOT_STRING_ALT) == 0)) 
    {
	Cp++;
	return NOT_OP;
    }
    if (strcmp(Query.tab[Cp], LP_STRING) == 0)
	return AND_OP;
    if (isop(Query.tab[Cp]) == 0)
	return AND_OP;
    return 0;
}

static HLIST term(void)
{
    HLIST left, right;
    int ignore = 0, op;

    left = factor(&ignore);
    if (left.stat == ERR_FATAL)
        return left;
    while ((op = andop())) {
	right = factor(&ignore);
	if (right.stat == ERR_FATAL)
	    return right;
	if (op == AND_OP) {
	    left = andmerge(left, right, &ignore);
	} else if (op == NOT_OP) {
	    left = notmerge(left, right, &ignore);
	}
	ignore = 0;
    }
    return left;
}


static int orop(void)
{
    if (Query.tab[Cp] == NULL)
	return 0;
    if ((strcmp(Query.tab[Cp], OR_STRING) == 0)||
	(strcmp(Query.tab[Cp], OR_STRING_ALT) == 0)) 
    {
	Cp++;
	return 1;
    }
    return 0;
}


/*
 *
 * Public functions
 *
 */

HLIST expr(void)
{
    HLIST left, right;

    left = term();
    if (left.stat == ERR_FATAL)
        return left;
    while (orop()) {
	right = term();
	if (right.stat == ERR_FATAL)
	    return right;
	left = ormerge(left, right);
	if (left.stat == ERR_FATAL)
	    return left;
    }
    return left;
}

void init_parser(void)
{
    Cp = 0;
}
