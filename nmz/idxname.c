/*
 * 
 * idxname.c - Idx handling routines.
 *
 * $Id: idxname.c,v 1.12 2000-01-07 01:21:56 satoru Exp $
 * 
 * Copyright (C) 1997-2000 Satoru Takabayashi  All rights reserved.
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

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "libnamazu.h"
#include "search.h"
#include "alias.h"
#include "var.h"
#include "idxname.h"

static struct nmz_indices indices = {0}; /* Initialize member `num' with 0 */

/*
 *
 * Public functions
 *
 */

enum nmz_stat 
add_index(const char *idxname)
{
    int newidxnum = indices.num;
    if (newidxnum >= INDEX_MAX)
	return FAILURE;
    indices.names[newidxnum] = malloc(strlen(idxname) + 1);
    if (indices.names[newidxnum] == NULL)
	return FAILURE;
    strcpy(indices.names[newidxnum], idxname);
    indices.hitnumlists[newidxnum] = NULL;
    indices.num = newidxnum + 1;

    return SUCCESS;
}

void 
free_idxnames(void)
{
    int i;
    for (i = 0; i < indices.num; i++) {
        free(indices.names[i]);
	free_hitnums(indices.hitnumlists[i]);
    }
    indices.num = 0;
}

/*
 * Except duplicated indices with a simple O(n^2) algorithm.
 */
void 
uniq_idxnames(void)
{
    int i, j, k;

    for (i = 0; i < indices.num - 1; i++) {
        for (j = i + 1; j < indices.num; j++) {
            if (strcmp(indices.names[i], indices.names[j]) == 0) {
                free(indices.names[j]);
                for (k = j + 1; k < indices.num; k++) {
                    indices.names[k - 1] = indices.names[k];
                }
                indices.num--;
                j--;
            }
        }
    }
}

/*
 * Expand index name aliases defined in namazurc.
 * e.g., Alias quux /home/foobar/NMZ/quux
 */
int 
expand_idxname_aliases(void)
{
    int i;

    for (i = 0; i < indices.num; i++) {
	struct nmz_alias *list = get_aliases();
	while (list) {
	    if (strcmp(indices.names[i], list->alias) == 0) {
		free(indices.names[i]);
		indices.names[i] = malloc(strlen(list->real) + 1);
		if (indices.names[i] == NULL) {
		    set_dyingmsg("expand_idxname_aliases: malloc()");
		    return FAILURE;
		}
		strcpy(indices.names[i], list->real);
            }
	    list = list->next;
	}
    }
    return 0;
}

/*
 * Complete an abbreviated index name to completed one.
 * e.g.,  +foobar -> (DEFAULT_INDEX)/foobar
 */
int 
complete_idxnames(void)
{
    int i;

    for (i = 0; i < indices.num; i++) {
 	if (*indices.names[i] == '+' && isalnum(*(indices.names[i] + 1))) {
	    char *tmp;
	    tmp = malloc(strlen(DEFAULT_INDEX) 
				  + 1 + strlen(indices.names[i]) + 1);
	    if (tmp == NULL) {
		set_dyingmsg("complete_idxnames: malloc()");
		return FAILURE;
	    }
	    strcpy(tmp, DEFAULT_INDEX);
	    strcat(tmp, "/");
	    strcat(tmp, indices.names[i] + 1);  /* +1 means '+' */
	    free(indices.names[i]);
	    indices.names[i] = tmp;
	}
    }
    return 0;
}

/*
 * Get the name of the index specified by id.
 */
char *
get_idxname(int id)
{
    return indices.names[id];
}

/*
 * Get the total number of indices to search.
 */
int 
get_idxnum()
{
    return indices.num;
}

/*
 * Set the total hit number of the index specified by id.
 */
void
set_idx_totalhitnum(int id, int hitnum)
{
    indices.totalhitnums[id] = hitnum;
}

/*
 * Get the total hit number of the index specified by id.
 */
int
get_idx_totalhitnum(int id)
{
    return indices.totalhitnums[id];
}

/*
 * Get the hitnumlist of the index specified by id.
 */
struct nmz_hitnumlist *
get_idx_hitnumlist(int id)
{
    return indices.hitnumlists[id];
}

void
set_idx_hitnumlist(int id, struct nmz_hitnumlist *hnlist)
{
    indices.hitnumlists[id] = hnlist;
}

/*
 * Push something and return pushed list.
 */
struct nmz_hitnumlist *
push_hitnum(struct nmz_hitnumlist *hn, 
	    const char *str,
	    int hitnum, 
	    enum nmz_stat stat
)
{
    struct nmz_hitnumlist *hnptr = hn, *prevhnptr = hn;
    while (hnptr != NULL) {
	prevhnptr = hnptr;
	hnptr = hnptr->next;
    }
    if ((hnptr = malloc(sizeof(struct nmz_hitnumlist))) == NULL) {
	set_dyingmsg("push_hitnum: malloc failed on hnptr");
	return NULL;
    }
    if (prevhnptr != NULL)
	prevhnptr->next = hnptr;
    hnptr->hitnum = hitnum;
    hnptr->stat  = stat;
    hnptr->phrase = NULL;
    hnptr->next  = NULL;
    if ((hnptr->word = malloc(strlen(str) +1)) == NULL) {
	set_dyingmsg("push_hitnum: malloc failed on str");
	return NULL;
    }
    strcpy(hnptr->word, str);

    if (hn == NULL)
	return hnptr;
    return hn;
}

