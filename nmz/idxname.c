/*
 * 
 * idxname.c - Idx handling routines.
 *
 * $Id: idxname.c,v 1.6 1999-12-06 09:15:15 satoru Exp $
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

#include <stdlib.h>
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "namazu.h"
#include "libnamazu.h"
#include "var.h"
#include "idxname.h"

enum nmz_stat add_index(char *idxname)
{
    int newidxnum = Idx.num;
    if (newidxnum >= INDEX_MAX)
	return FAILURE;
    Idx.names[newidxnum] = malloc(strlen(idxname) + 1);
    if (Idx.names[newidxnum] == NULL)
	return FAILURE;
    strcpy(Idx.names[newidxnum], idxname);
    Idx.pr[newidxnum] = NULL;
    Idx.num = newidxnum + 1;

    return SUCCESS;
}

int getidxnum()
{
    return Idx.num;
}
