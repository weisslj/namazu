/*
 * 
 * form.c -
 * 
 * $Id: form.c,v 1.5 1999-08-23 10:40:52 satoru Exp $
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
#ifdef __EMX__
#include <sys/types.h>
#endif
#include <sys/stat.h>
#include "namazu.h"
#include "util.h"

/* load the whole of file */
uchar *load_headfoot(uchar *fname)
{
    uchar *buf;
    FILE *fp;
    struct stat fstatus;

    stat(fname, &fstatus);
    fp = fopen(fname, "rb");
    if (fp == NULL) {
        fprintf(stderr, "warning: Can't open %s\n", fname);
        return 0;
    }
    buf = (uchar *) malloc(fstatus.st_size + 1);
    if (buf == NULL) {
	 error("print_headfoot(malloc)");
    }
    if (fread(buf, sizeof(uchar), fstatus.st_size, fp) == 0)
        error("print_headfoot(fread)");
    *(buf + fstatus.st_size) = '\0';
    fclose(fp);
    return buf;
}

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

/* replace <input type="text" name="key"  value="hogehoge"> */
int replace_key_value(uchar *p, uchar *orig_query)
{
    uchar query[BUFSIZE];
    
    strcpy(query, orig_query);

    if (!cmp_element(p, (uchar *)"input type=\"text\" name=\"key\"")) {
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

/* delete string */
void delete_str(uchar *s, uchar *d)
{
    int l;
    uchar *tmp;

    l = strlen(d);
    for (tmp = s; *tmp; tmp++) {
        if (!strncmp(tmp, d, l)) {
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
        if (!strncmp(s, "value=\"", 7)) {
            for (s += 7; *s && *s != '"'; s++, v++) {
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

int str_backward_cmp(uchar *str1, uchar *str2)
{
    uchar *p, *q;

    p = str1 + strlen(str1) -1;
    q = str2 + strlen(str2) -1;

    for (; p >= str1 && q >= str2; p--, q--) {
        if (*p != *q) {
            return 1;
        }
    }
    return 0;
}


int select_option(uchar *s, uchar *name, uchar *subquery)
{
    uchar value[BUFSIZE];

    if (!cmp_element(s, (uchar *)"option")) {
        delete_str(s, (uchar *)"selected ");
        fputs(s, stdout);
        get_value(s, value);
        if (!strcmp(name, "format")) {
            if (!strcmp(value, "short") && ShortFormat) {
                fputs(" selected", stdout);
            } else if (!strcmp(value, "long") && (!ShortFormat)) {
                fputs(" selected", stdout);
            } 
        } else if (!strcmp(name, "sort")) {
            if (!strcmp(value, "later") && LaterOrder && !ScoreSort) {
                fputs(" selected", stdout);
            } else if (!strcmp(value, "earlier")  && !LaterOrder && !ScoreSort)
            {
                fputs(" selected", stdout);
            } else if (!strcmp(value, "score") && ScoreSort) {
                fputs(" selected", stdout);
            }
        } else if (!strcmp(name, "lang")) {
            if (!strcmp(value, Lang)) {
                fputs(" selected", stdout);
            }
        } else if (!strcmp(name, "dbname")) {
            if (*DbNames[0] && !str_backward_cmp(value, DbNames[0])) {
                fputs(" selected", stdout);
            }
        } else if (!strcmp(name, "subquery")) {
            if (!strcmp(value, subquery)) {
                fputs(" selected", stdout);
            }
        } else if (!strcmp(name, "max")) {
            if (atoi(value) == HListMax) {
                fputs(" selected", stdout);
            }
        }
        return 0;
    }
    return 1;
}

/* mark CHECKBOX of dbname with CHECKED */
int check_checkbox(uchar *s)
{
    uchar value[BUFSIZE];
    int i;

    if (!cmp_element(s, (uchar *)"input type=\"checkbox\" name=\"dbname\"")) {
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
            for (i = 0; i < DbNumber; i++) {
                if (!str_backward_cmp(name, DbNames[i])) {
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
        if (!replace_key_value(tmp, query))
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

/* display header or footer file.
 * very foolish
 */
void print_headfoot(uchar * fname, uchar * query, uchar *subquery)
{
    uchar *buf, *p, *q, name[BUFSIZE] = "";
    int f, f2;
    buf = load_headfoot(fname);
    if (buf == NULL) {
        return;
    }

    for (p = buf, f = f2 = 0; *p; p++) {
        if (BASE_URI[0] && !strncmp(p, "\n</head>", 8)) {
            printf("\n<base href=\"%s\">", BASE_URI);
        }

        if (!f && *p == '<') {
            if (!strncmp(p, "</title>", 8)) {
                printf(": &lt;");
                fputx(query, stdout);
                printf("&gt;</title>\n");
                p = (uchar *)strchr(p, '>');
                continue;
            }

            if (!IsCGI && !ForcePrintForm && !strncmp(p, "<form ",  6)) f2 = 1;
            if (!IsCGI && !ForcePrintForm && !strncmp(p, "</form>", 7)) 
            {f2 = 0; p += 6; continue;}
            if (f2) continue;
            /* In case of file's encoding is ISO-2022-JP, 
               the problem occurs if JIS X 208 characters in element */
            q = (uchar *)strchr(p, (int)'>');
            fputs("<", stdout);
            treat_tag(p + 1, q - 1, query, name, subquery);
            fputs(">", stdout);
            p = q;
        } else {
            if (!strncmp(p, "\x1b$", 2) 
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


