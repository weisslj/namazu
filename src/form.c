/*
 * 
 * form.c -
 * 
 * $Id: form.c,v 1.15 1999-09-06 01:13:10 satoru Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "namazu.h"
#include "util.h"
#include "form.h"
#include "output.h"
#include "field.h"
#include "hlist.h"

/************************************************************
 *
 * Private functions
 *
 ************************************************************/

int cmp_element(uchar*, uchar*);
int replace_query_value(uchar*, uchar*);
int replace_action(uchar*);
void delete_str(uchar*, uchar*);
void get_value(uchar*, uchar*);
void get_select_name(uchar*, uchar*);
int select_option(uchar*, uchar*, uchar*);
int check_checkbox(uchar*);
void treat_tag(uchar*, uchar*, uchar*, uchar *, uchar *);

/* compare the element
 * some measure of containing LF or redundant spaces are acceptable.
 * igonore cases
 */
int cmp_element(uchar *s1, uchar *s2)
{
    for (; *s1 && *s2; s1++, s2++) {
        if (*s2 == ' ') {
            while (*s1 == ' ' || *s1 == '\t' || *s1 == '\n' || *s1 == '\r') {
                s1++;
            }
            s2++;
        }
        if (toupper(*s1) != toupper(*s2)) {
            break;
        }
    }
    if (*s2 == '\0') {
        return 0;
    } else {
        return 1;
    }
}

#define iseuc(c)  ((c) >= 0xa1 && (c) <= 0xfe)

/* replace <input type="text" name="query"  value="hogehoge"> */
int replace_query_value(uchar *p, uchar *orig_query)
{
    uchar query[BUFSIZE];
    
    strcpy(query, orig_query);

    if (!cmp_element(p, (uchar *)"input type=\"text\" name=\"query\"")) {
        for (; *p; p++)
            fputc(*p, stdout);
        printf(" value=\"");
        fputx(query, stdout); 
        fputs("\"", stdout);
        return 0;
    }
    return 1;
}

/* replace <form method="get" action="/somewhere/namazu.cgi"> */
int replace_action(uchar *p)
{
    if (!cmp_element(p, (uchar *)"form method=\"get\"")) {
        char *script_name = getenv("SCRIPT_NAME");
        if (script_name) {
            printf("form method=\"get\" action=\"%s\"", script_name);
            return 0;
        } else {
            return 1;
        }
    }
    return 1;
}

/* delete string (case insensitive) */
void delete_str(uchar *s, uchar *d)
{
    int l;
    uchar *tmp;

    l = strlen(d);
    for (tmp = s; *tmp; tmp++) {
        if (strncasecmp(tmp, d, l) == 0) {
            strcpy(tmp, tmp + l);
            tmp--;
        }
    }
    chomp(s);
}

void get_value(uchar *s, uchar *v)
{
    *v = '\0';
    for (; *s; s++) {
        if (strprefixcasecmp(s, "value=\"") == 0) {
            for (s += strlen("value=\""); *s && *s != '"'; s++, v++) 
	    {
                *v = *s;
            }
            *v = '\0';
            return;
        }
    }
}

void get_select_name(uchar *s, uchar *v)
{
    *v = '\0';
    for (; *s; s++) {
        if (!cmp_element(s, (uchar *)"select name=\"")) {
            s = (uchar *)strchr(s, '"') + 1;
            for (; *s && *s != (uchar)'"'; s++, v++) {
                *v = *s;
            }
            *v = '\0';
            return;
        }
    }
}

int select_option(uchar *s, uchar *name, uchar *subquery)
{
    uchar value[BUFSIZE];

    if (!cmp_element(s, (uchar *)"option")) {
        delete_str(s, (uchar *)"selected ");
        fputs(s, stdout);
        get_value(s, value);
        if (strcasecmp(name, "result") == 0) {
            if (strcasecmp(value, Template) == 0) {
                fputs(" selected", stdout);
            }
        } else if (strcasecmp(name, "sort") == 0) {
            if ((strcasecmp(value, "date:late") == 0) && 
		SortMethod    == SORT_BY_DATE &&
		SortOrder == DESCENDING) 
	    {
                fputs(" selected", stdout);
            } else if ((strcasecmp(value, "date:early") == 0) && 
		SortMethod    == SORT_BY_DATE &&
		SortOrder == ASCENDING)
            {
                fputs(" selected", stdout);
            } else if ((strcasecmp(value, "score") == 0) && 
		       SortMethod  == SORT_BY_SCORE) 
	    {
                fputs(" selected", stdout);
            } else if ((strprefixcasecmp(value, "field:") == 0) && 
		       SortMethod  == SORT_BY_FIELD) 
	    {
		uchar *p;
		int n, order = DESCENDING;
		uchar field[BUFSIZE];

		p = value + strlen("field:");
		n = strspn(p, FIELD_SAFE_CHARS);
		strncpy(field, p, n);
		field[n] = '\0'; /* Hey, don't forget this after strncpy()! */
		p += n;

		if (strprefixcasecmp(p, ":ascending") == 0) {
		    order = ASCENDING;
		} else if (strprefixcasecmp(p, ":descending") == 0) {
		    order = DESCENDING;
		}

		if (strcmp(field, get_sort_field()) == 0 && 
		    order == SortOrder) {
		    fputs(" selected", stdout);
		}
            }

        } else if (strcasecmp(name, "lang") == 0) {
            if (strcasecmp(value, Lang) == 0) {
                fputs(" selected", stdout);
            }
        } else if (strcasecmp(name, "idxname") == 0) {
            if (*Idx.names[0] && strsuffixcmp(value, Idx.names[0]) == 0) {
                fputs(" selected", stdout);
            }
        } else if (strcasecmp(name, "subquery") == 0) {
            if (strcasecmp(value, subquery)  == 0) {
                fputs(" selected", stdout);
            }
        } else if (strcasecmp(name, "max") == 0) {
            if (atoi(value) == HListMax) {
                fputs(" selected", stdout);
            }
        }
        return 0;
    }
    return 1;
}

/* mark CHECKBOX of idxname with CHECKED */
int check_checkbox(uchar *s)
{
    uchar value[BUFSIZE];
    int i;

    if (!cmp_element(s, (uchar *)"input type=\"checkbox\" name=\"idxname\"")) {
        uchar *pp;
        int db_count, searched;

        delete_str(s, (uchar *)"checked");
        fputs(s, stdout);
        get_value(s, value);
        for (pp = value, db_count = searched = 0 ; *pp ;db_count++) {
            uchar name[BUFSIZE], *x;
            if ((x = (uchar *)strchr(pp, (int)','))) {
                *x = '\0';
                strcpy(name, pp);
                pp = x + 1;
            } else {
                strcpy(name, pp);
                pp += strlen(pp);
            }
            for (i = 0; i < Idx.num; i++) {
                if (strsuffixcmp(name, Idx.names[i]) == 0) {
                    searched++;
                    break;
                }
            }
        }
        if (db_count == searched) {
            printf(" checked");
        }
        return 0;
    }
    return 1;
}

/* treat an HTML tag */
void treat_tag(uchar *p, uchar *q, uchar *query, 
               uchar *select_name, uchar *subquery)
{
    uchar tmp[BUFSIZE];
    int l;

    l = q - p + 1;
    if (l < BUFSIZE - 1) {
        strncpy(tmp, p, l);
        tmp[l] = '\0';
        if (!replace_query_value(tmp, query))
            return;
        if (!replace_action(tmp))
            return;
        if (!select_option(tmp, select_name, subquery))
            return;
        if (!check_checkbox(tmp))
            return;
        get_select_name(tmp, select_name);
    }
    fputs(tmp, stdout);
}

/************************************************************
 *
 * Public functions
 *
 ************************************************************/

/* display header or footer file.
 * very ad hoc.
 */
void print_headfoot(uchar * fname, uchar * query, uchar *subquery)
{
    uchar *buf, *p, *q, name[BUFSIZE] = "";
    int f, f2;
    buf = readfile(fname);
    if (buf == NULL) {
        return;
    }

    for (p = buf, f = f2 = 0; *p; p++) {
        if (BASE_URI[0] && (strprefixcasecmp(p, "\n</head>") == 0)) {
            printf("\n<base href=\"%s\">", BASE_URI);
        }

        if (f == 0 && *p == '<') {
            if (strprefixcasecmp(p, "</title>") == 0) {
		if (*query != '\0') {
		    printf(": &lt;");
		    fputx(query, stdout);
		    printf("&gt;");
		}
		printf("</title>\n");
                p = (uchar *)strchr(p, '>');
                continue;
            }

            if (!IsCGI && !ForcePrintForm && 
		(strprefixcasecmp(p, "<form ") == 0)) 
	    {
		f2 = 1;
	    }

            if (!IsCGI && !ForcePrintForm && 
		(strprefixcasecmp(p, "</form>") == 0)) 
            {
		f2 = 0; 
		p += 6; continue;
	    }
            if (f2) {
		continue;
	    }

            /* In case of file's encoding is ISO-2022-JP, 
               the problem occurs if JIS X 208 characters in element */
            q = (uchar *)strchr(p, (int)'>');
            fputs("<", stdout);
            treat_tag(p + 1, q - 1, query, name, subquery);
            fputs(">", stdout);
            p = q;
        } else {
            if ((strncmp(p, "\x1b$", 2) == 0)
                && (*(p + 2) == 'B' || *(p + 2) == '@')) 
            {
                f = 1;
            } else if (!strncmp(p, "\x1b(", 2) &&
                       (*(p + 2) == 'J' || *(p + 2) == 'B' || *(p + 2) == 'H'))
            {
                f = 0;
            }
            if (f2) continue;
            fputc(*p, stdout);
        }
    }
    free(buf);
}





