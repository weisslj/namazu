/*
 * 
 * alias.c - 
 *
 * $Id: alias.c,v 1.2 1999-12-09 02:44:51 satoru Exp $
 * 
 * Copyright (C) 1997-1999 Satoru Takabayashi  All rights reserved.
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
#include "alias.h"
#include "libnamazu.h"
#include "i18n.h"

static struct nmz_alias  *aliases  = NULL;

/*
 *
 * Public functions
 *
 */

enum nmz_stat add_alias(char *alias, char *real)
{
    struct nmz_alias *newp;

    newp = malloc(sizeof(struct nmz_alias));
    if (newp == NULL) {
	 set_dyingmsg("add_alias_malloc");
	 return FAILURE;
    }
    newp->alias = malloc(strlen(alias) + 1);
    if (newp->alias == NULL) {
	 set_dyingmsg("add_alias_malloc");
	 return FAILURE;
    }

    newp->real = malloc(strlen(real) + 1);
    if (newp->real == NULL) {
	 set_dyingmsg("add_alias_malloc");
	 return FAILURE;
    }

    strcpy(newp->alias, alias);
    strcpy(newp->real, real);
    newp->next = aliases;
    aliases = newp;
    return SUCCESS;
}

void free_aliases(void)
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

/* it's very dangerous to use! */
struct nmz_alias *get_aliases(void)
{
    return aliases;
}

void show_aliases(void)
{
    struct nmz_alias *list = aliases;

    while (list) {
	printf(_("Alias:   \"%s\" -> \"%s\"\n"), 
	       list->alias, list->real);
	list = list->next;
    }
}
