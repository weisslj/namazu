/*
 * 
 * idxname.c - Idx handling routines.
 *
 * $Id: idxname.c,v 1.1 1999-10-13 08:30:42 knok Exp $
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

#include "namazu.h"
#include "var.h"
#include "idxname.h"

int add_index(uchar *idxname)
{
  int newidxnum = Idx.num;
  if (newidxnum >= INDEX_MAX)
    return ERR_INDEX_MAX;
  if ((Idx.names[newidxnum] = malloc(strlen(idxname) + 1) == NULL))
    return ERR_MALLOC;
  strcpy(Idx.names[newidxnum], idxname);
  Idx.pr[newidxnum] = NULL;
  Idx.num = newidxnum + 1;
  return Idx.num;
}

int getidxnum()
{
  return Idx.num;
}
