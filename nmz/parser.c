/*
 * 
 * parser.c -
 * 
 * $Id: parser.c,v 1.18 2000-01-10 09:07:47 satoru Exp $
 * 
 * Copyright (C) 1997-2000 Satoru Takabayashi  All rights reserved.
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
#include "libnamazu.h"
#include "hlist.h"
#include "search.h"
#include "parser.h"
#include "var.h"
#include "query.h"

static int Cp = 0; /* variable that saves current position of parser */

/*
 *
 * Private functions
 *
 */

static NmzResult factor(int*);
static int andop(void);
static NmzResult term(void);
static int orop(void);

static NmzResult 
factor(int *ignore)
{
    NmzResult val;
    val.num = 0;

    while (1) {
	char *token = nmz_get_querytoken(Cp);
        if (token == NULL) {
            return val;
	}

        if (strcmp(token, LP_STRING) == 0) {
            Cp++;
            if (nmz_get_querytoken(Cp) == NULL) {
		val.stat = ERR_INVALID_QUERY;
		nmz_set_dyingmsg("foobar");
                return val;
	    }
            val = nmz_expr();
	    if (val.stat == ERR_FATAL)
	        return val;
            if (nmz_get_querytoken(Cp) == NULL) {
		val.stat = ERR_INVALID_QUERY;
                return val;
	    }
            if (strcmp(nmz_get_querytoken(Cp), RP_STRING) == 0)
                Cp++;
            break;
        } else if (!nmz_is_query_op(token)) {
            val = nmz_do_searching(token, val);
	    if (val.stat == ERR_FATAL)
	       return val;
            if (val.stat == ERR_TOO_MUCH_MATCH ||
		val.stat == ERR_TOO_MUCH_MATCH)
	    {
                *ignore = 1;
                val.num = 0;   /* assign 0 - this is important */
            }

            Cp++;
            break;
        } else {
	    val.stat = ERR_INVALID_QUERY;
            Cp++;
        }
    }
    return val;
}

static int 
andop(void)
{
    char *token = nmz_get_querytoken(Cp);

    if (token == NULL)
	return 0;
    if ((strcmp(token, AND_STRING) == 0) ||
	(strcmp(token, AND_STRING_ALT) ==0)) 
    {
	Cp++;
	return AND_OP;
    }
    if ((strcmp(token, NOT_STRING) == 0)||
	(strcmp(token, NOT_STRING_ALT) == 0)) 
    {
	Cp++;
	return NOT_OP;
    }
    if (strcmp(token, LP_STRING) == 0)
	return AND_OP;
    if (!nmz_is_query_op(token))
	return AND_OP;
    return 0;
}

static NmzResult 
term(void)
{
    NmzResult left, right;
    int ignore = 0, op;

    left = factor(&ignore);
    if (left.stat == ERR_FATAL)
        return left;
    while ((op = andop())) {
	right = factor(&ignore);
	if (right.stat == ERR_FATAL)
	    return right;
	if (op == AND_OP) {
	    left = nmz_andmerge(left, right, &ignore);
	} else if (op == NOT_OP) {
	    left = nmz_notmerge(left, right, &ignore);
	}
	ignore = 0;
    }
    return left;
}


static int 
orop(void)
{
    char *token = nmz_get_querytoken(Cp);

    if (token == NULL)
	return 0;
    if ((strcmp(token, OR_STRING) == 0)|| 
	(strcmp(token, OR_STRING_ALT) == 0)) 
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

NmzResult 
nmz_expr(void)
{
    NmzResult left, right;

    left = term();
    if (left.stat == ERR_FATAL)
        return left;
    while (orop()) {
	right = term();
	if (right.stat == ERR_FATAL)
	    return right;
	left = nmz_ormerge(left, right);
	if (left.stat == ERR_FATAL)
	    return left;
    }
    return left;
}

void 
nmz_init_parser(void)
{
    Cp = 0;
}

/*
 * Check the character `c' whether metacharacter (operator) of not.
 */
int 
nmz_is_query_op(const char * c)
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


