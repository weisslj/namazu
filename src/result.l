/*
 * result.l -
 * -*- C -*-
 * $Id: result.l,v 1.7 1999-08-23 11:28:10 satoru Exp $
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
%{
#include "namazu.h"
#include "util.h"
#include <string.h>
	void get_field_data(char *);
	static int Fid;
	static uchar Buf[BUFSIZE * 16];
	static int Score, Counter;
%}

%%
\$\{([-_a-zA-Z]+|namazu::[-_a-zA-Z]+)\} 	       get_field_data(yytext);
.|\n                           strcat(Buf, yytext);
%%

void make_fullpathname(int n)
{
    uchar *base;

    base = DbNames[n];

    pathcat(base, FIELDINFO);
    pathcat(base, RESULTFILE);
}

void encode_entity(uchar *str)
{
    int i;
    uchar tmp[BUFSIZE];

    strcpy(tmp, str);
    strcpy(str, "");
    for (i = 0; tmp[i]; i++) {
	if (tmp[i] == '<') {
	    strcat(str, "&lt;");
	} else if (tmp[i] == '>') {
	    strcat(str, "&gt;");
	} else if (tmp[i] == '&') {
	    strcat(str, "&amp;");
	} else if (tmp[i] == '"') {
	    strcat(str, "&quot;");
	} else {
	    strncat(str, tmp + i, 1);
	}
    }
}

/* inefficient algorithm but it works */
void emphasize(uchar *str) {
    int i;

    for (i = 0; KeyItem[i] != NULL; i++) {
	uchar *ptr = str;
	int keylen = strlen(KeyItem[i]);

	do {
	    ptr = strcasestr(ptr, KeyItem[i]);
	    if (ptr != NULL) {
		memmove(ptr + 2, ptr, strlen(ptr) + 1);
		memmove(ptr + 1, ptr + 2, keylen);
		*ptr = EMPHASIZING_START;
		*(ptr + keylen + 1) = EMPHASIZING_END;
		ptr += 2;
	    }
	} while (ptr != NULL);
    }
}

void get_field_data(char *key) 
{
    uchar field[BUFSIZE], file_name[BUFSIZE];
    FILE *fp_field, *fp_field_idx;

    /* 8 is length of '&quot;' + 2 (for emphasizing). 
       It's a consideration for buffer overflow (overkill?) */
    uchar buf[BUFSIZE * 8];  

    strcpy(field, key + 2);
    *(field + strlen(field) - 1) = '\0';
    apply_field_alias(field);

    if (strcmp(field, "namazu::score") == 0) {
	sprintf(buf, "%d", Score);
	strcat(Buf, buf);
	return;
    } else if (strcmp(field, "namazu::counter") == 0) {
	sprintf(buf, "%d", Counter);
	strcat(Buf, buf);
	return;
    }


    /* make a pathname */
    strcpy(file_name, FIELDINFO);
    strcat(file_name, field);
    
    fp_field = fopen(file_name, "rb");
    if (fp_field == NULL) {
        fprintf(stderr, "%s: cannot open file.\n", file_name);
    }

    strcat(file_name, ".i");
    fp_field_idx = fopen(file_name, "rb");
    if (fp_field_idx == NULL) {
        fprintf(stderr, "%s: cannot open file.\n", file_name);
    }

    /* You can rely on that length of one field is shorter than 
       BUFSIZE (1024) because its length is restricted at 
       put_field_index() in mknmz.
     */
    fseek(fp_field, get_index_pointer(fp_field_idx, Fid), 0);
    fgets(buf, BUFSIZE, fp_field);


    if (!strcmp(field, "uri")) {
	replace_uri(buf);
    } else {
	/* do not emphasize in URI */
	emphasize(buf);
    }

    chomp(buf);
    encode_entity(buf);
    strcat(Buf, buf);
    fclose(fp_field);
    fclose(fp_field_idx);
}

/* display the hlist */
void put_hlist (HLIST hlist)
{
    int i;

    if (hlist.n <= 0 || HListMax == 0)
	return;

    for (i = HListWhence; i < hlist.n; i++) {
	strcpy(Buf, "\t");  /* '\t' has an important role cf. fputx() */
	Score   = hlist.scr[i];
	Counter = i + 1;
	Fid = hlist.fid[i];
	make_fullpathname(hlist.did[i]);
	if (!AllList && (i >= HListWhence + HListMax))
	    break;
	if (MoreShortFormat) {
	    get_field_data("${uri}");
	} else {
	    if ((yyin = fopen(RESULTFILE, "r")) == NULL) {
		error("put_hlist");
		exit(-1);
	    }
	    yylex();
	    fclose(yyin);
	}
	fputx(Buf, stdout);
	fputx("\n", stdout);
    }
}

