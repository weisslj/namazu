/*
 * 
 * alias.c - 
 *
 * $Id: alias.c,v 1.13 2000-01-29 04:58:18 satoru Exp $
 * 
 * Copyright (C) 1997-1999 Satoru Takabayashi All rights reserved.
 * Copyright (C) 2000 Namazu Project All rights reserved.
 * Copyright (C) 1999 NOKUBI Takatsugu All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "alias.h"
#include "util.h"
#include "libnamazu.h"
#include "i18n.h"

static struct nmz_alias  *aliases  = NULL;

/*
 *
 * Public functions
 *
 */

enum nmz_stat 
nmz_add_alias(const char *alias, const char *real)
{
    struct nmz_alias *newp;

    newp = malloc(sizeof(struct nmz_alias));
    if (newp == NULL) {
	 nmz_set_dyingmsg(nmz_msg("%s", strerror(errno)));
	 return FAILURE;
    }
    newp->alias = malloc(strlen(alias) + 1);
    if (newp->alias == NULL) {
	 nmz_set_dyingmsg(nmz_msg("%s", strerror(errno)));
	 return FAILURE;
    }

    newp->real = malloc(strlen(real) + 1);
    if (newp->real == NULL) {
	 nmz_set_dyingmsg(nmz_msg("%s", strerror(errno)));
	 return FAILURE;
    }

    strcpy(newp->alias, alias);
    strcpy(newp->real, real);
    newp->next = aliases;
    aliases = newp;
    return SUCCESS;
}

void 
nmz_free_aliases(void)
{
    struct nmz_alias *list, *next;
    list = aliases;

    while (list) {
	next = list->next;
	free(list->alias);
	free(list->real);
	free(list);
	list = next;
    }
}

/*
 * It's very dangerous to use!
 */
struct nmz_alias *
nmz_get_aliases(void)
{
    return aliases;
}
