/*
 * 
 * adhoc.h - collection of very ad-hoc solutions :-p
 * 
 * $Id: adhoc.h,v 1.1 2000-08-20 21:19:33 rug Exp $
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

#ifndef _ADHOC_H
#define _ADHOC_H

/* Substitute for tolower(3) */
extern int adhoc_tolower (int c);

/* Case-insensitive brute force search */
extern char *adhoc_strcasestr (char *s1, char *s2);

#endif /* _ADHOC_H */
