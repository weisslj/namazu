/*
 * 
 * form.c -
 * 
 * $Id: form.c,v 1.17 1999-10-11 04:25:24 satoru Exp $
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
#include "codeconv.h"
#include "i18n.h"

/************************************************************
 *
 * Private functions
 *
 ************************************************************/

static int cmp_element(uchar*, uchar*);
static int replace_query_value(uchar*, uchar*);
static uchar *read_headfoot(uchar*);
static void delete_str(uchar*, uchar*);
static void get_value(uchar*, uchar*);
static void get_select_name(uchar*, uchar*);
static int select_option(uchar*, uchar*, uchar*);
static int check_checkbox(uchar*);
static void handle_tag(uchar*, uchar*, uchar*, uchar *, uchar *);

/* compare the element
 * some measure of containing LF or redundant spaces are acceptable.
 * igonore cases
 */
static int cmp_element(uchar *s1, uchar *s2)
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
static int replace_query_value(uchar *p, uchar *query)
{
    uchar tmp_query[BUFSIZE];
    
    strcpy(tmp_query, query);

    if (cmp_element(p, (uchar *)"input type=\"text\" name=\"query\"") == 0) {
        for (; *p; p++)
            fputc(*p, stdout);
        print(" value=\"");
	conv_ext(tmp_query);
        print(tmp_query); 
        print("\"");
        return 0;
    }
    return 1;
}

/* delete string (case insensitive) */
static void delete_str(uchar *s, uchar *d)
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

static void get_value(uchar *s, uchar *v)
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

static void get_select_name(uchar *s, uchar *v)
{
    *v = '\0';
    for (; *s; s++) {
        if (cmp_element(s, (uchar *)"select name=\"") == 0) {
            s = (uchar *)strchr(s, '"') + 1;
            for (; *s && *s != (uchar)'"'; s++, v++) {
                *v = *s;
            }
            *v = '\0';
            return;
        }
    }
}

static int select_option(uchar *s, uchar *name, uchar *subquery)
{
    uchar value[BUFSIZE];

    if (cmp_element(s, (uchar *)"option") == 0) {
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
            if (strcasecmp(value, get_lang()) == 0) {
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
static int check_checkbox(uchar *s)
{
    uchar value[BUFSIZE];
    int i;

    if (cmp_element(s, "input type=\"checkbox\" name=\"idxname\"") == 0) {
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
                if (strcmp(name, 
			   Idx.names[i] + strlen(DEFAULT_INDEX) + 1) == 0) 
		{
                    searched++;
                    break;
                }
            }
        }
        if (db_count == searched) {
            print(" checked");
        }
        return 0;
    }
    return 1;
}

/* handle an HTML tag */
static void handle_tag(uchar *p, uchar *q, uchar *query, 
               uchar *select_name, uchar *subquery)
{
    uchar tmp[BUFSIZE];
    int l;

    l = q - p + 1;
    if (l < BUFSIZE - 1) {
        strncpy(tmp, p, l);
        tmp[l] = '\0';
        if (replace_query_value(tmp, query) == 0)
            return;
        if (select_option(tmp, select_name, subquery) == 0)
            return;
        if (check_checkbox(tmp) == 0)
            return;
        get_select_name(tmp, select_name);
    }
    fputs(tmp, stdout);
}

static uchar *read_headfoot(uchar *fname) 
{
    uchar *buf, *p, tmp_fname[BUFSIZE];
    char *script_name;

    strcpy(tmp_fname, fname);
    choose_msgfile(tmp_fname);

    buf = readfile(tmp_fname);
    if (buf == NULL) { /* failed */
	buf = readfile(fname); /* retry with plain fname */
	if (buf == NULL) {
	    return NULL;
	}
    }

    /* expand buf memory for replacing {cgi} */
    buf = (uchar *)realloc(buf, strlen(buf) + BUFSIZE);
    if (buf == NULL) {
        return NULL;
    }

    script_name= getenv("SCRIPT_NAME");

    /* can't determine script_name */
    if (script_name == NULL) {
	return buf;
    }

    /* replace {cgi} with a proper namazu.cgi location */
    while ((p = strstr(buf, "{cgi}")) != NULL) {
	subst(p, "{cgi}", script_name);
    }
    
    return buf;
}

/************************************************************
 *
 * Public functions
 *
 ************************************************************/

/* 
 * display header or footer file.
 * very ad hoc.
 */
void print_headfoot(uchar * fname, uchar * query, uchar *subquery)
{
    uchar *buf, *p, *q, name[BUFSIZE] = "";
    int f, f2;

    buf = read_headfoot(fname);
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
		    uchar tmp_query[BUFSIZE];
		    strcpy(tmp_query, query);
		    conv_ext(tmp_query);

		    print(": &lt;");
		    print(tmp_query);
		    print("&gt;");
		}
		print("</title>\n");
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
            handle_tag(p + 1, q - 1, query, name, subquery);
            fputs(">", stdout);
            p = q;
        } else {
            if ((strncmp(p, "\033$", 2) == 0)
                && (*(p + 2) == 'B' || *(p + 2) == '@')) 
            {
                f = 1;
            } else if ((strncmp(p, "\033(", 2) == 0) &&
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





