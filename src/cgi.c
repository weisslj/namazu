/*
 * 
 * cgi.c -
 * 
 * $Id: cgi.c,v 1.20 1999-10-11 04:25:23 satoru Exp $
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
#include <unistd.h>
#include <ctype.h>
#include "namazu.h"
#include "util.h"
#include "usage.h"
#include "message.h"
#include "cgi.h"
#include "output.h"
#include "field.h"
#include "hlist.h"
#include "i18n.h"

uchar *ScriptName    = (uchar *)"";
uchar *QueryString   = (uchar *)"";
uchar *ContentLength = (uchar *)"";

/************************************************************
 *
 * Private functions
 *
 ************************************************************/

static int validate_idxname(uchar* );
static int get_cgi_vars(uchar* , uchar*);

/* validate idxname (if it contain '/', it's invalid) */
static int validate_idxname(uchar * idxname)
{
    int win32 = 0;
#if  defined(_WIN32) || defined(__EMX__)
    win32 = 1;
#endif

    if (*idxname == '\0' || *idxname == '/' || (win32 && *idxname == '\\')) {
        print(MSG_MIME_HEADER);
	printf("%s : ", idxname);
        print(_("Invalid idxname."));
        exit(1);
    }
    while (*idxname) {
        if (strprefixcasecmp("../", idxname) == 0 ||
	    strcmp("..", idxname) == 0 ||
            (win32 && strprefixcasecmp("..\\", idxname) == 0)) 
        {
            print(MSG_MIME_HEADER);
	    printf("%s : ", idxname);
            print(_("Invalid idxname."));
            exit(1);
        }
	/* Skip until next '/' */
	while (*idxname && *idxname != '/' && !(win32 && *idxname == '\\')) {
	    idxname++;
	}
	/* Skip '/' */
	if (*idxname) {
	    idxname++;
	}
    }
    idxname--;  /* remove ending slashes */
    while (*idxname == '/' || (win32 && *idxname == '\\')) {
        *idxname = '\0';
        idxname--;
    }
    return 1;
}


/*  get CGI vars, very complicated ad hoc routine */
static int get_cgi_vars(uchar * query, uchar *subquery)
{
    int i;
    uchar *qs, tmp[BUFSIZE];
    int content_length;

    if ((QueryString = (uchar *)getenv("QUERY_STRING"))) {
        content_length = strlen(QueryString);
        if (content_length > CGI_QUERY_MAX) {
            print(MSG_MIME_HEADER);
            print(_("Too long QUERY_STRING"));
            exit(1);
        }
        if (!(ScriptName = (uchar *)getenv("SCRIPT_NAME"))) {
            return 1;
        }
    } else {
	if ((ContentLength = (uchar *)getenv("CONTENT_LENGTH"))) {
	    content_length = atoi(ContentLength);
            if (content_length > CGI_QUERY_MAX) {
                print(MSG_MIME_HEADER);
                print(_("Too long QUERY_STRING"));
                exit(1);
            }
            QueryString = (uchar *)malloc(content_length * sizeof(uchar) + 1);
	    if (QueryString == NULL) {
		die("QueryString( get_cgi_vars )");
	    }

	    fread(QueryString, sizeof(char), content_length, stdin);
	    HtmlOutput = 1;
	    HidePageIndex = 1;
	} else {
	    return 1;
	}
    }
    qs = QueryString;
    Idx.num = 0;

    /* note that CERN HTTPD would add empty PATH_INFO */
    if (getenv("PATH_INFO")) {
        uchar *path_info = (uchar *)getenv("PATH_INFO");
	path_info++; /* skip a beginning '/' character */
        if (strlen(path_info) > 0 && strlen(path_info) < 128) {
            validate_idxname(path_info);
            sprintf(tmp, "%s/%s", DEFAULT_INDEX, path_info);
            Idx.names[Idx.num] = (uchar *) malloc(strlen(tmp) + 1);
            if (Idx.names[Idx.num] == NULL) {
                die("cgi: malloc(idxname)");
            }
            strcpy(Idx.names[Idx.num], tmp);
            Idx.num++;
        }
    }

    while (*qs) {
        if (*qs == '&') {
            qs++;
            continue;
        } else if (strprefixcasecmp(qs, "query=") == 0) {
	    qs += strlen("query=");

	    for (i = 0; *qs && *qs != '&'; qs++, i++) {
                *(query + i) = *qs;
	    }
	    *(query + i) = '\0';
            decode_uri(query);
	    if (strlen(query) > QUERY_MAX) {
                print(MSG_MIME_HEADER);
		html_print(_(MSG_TOO_LONG_QUERY));
		exit(1);
	    }

#ifdef MSIE4MACFIX
#define MSIE4MAC "Mozilla/4.0 (compatible; MSIE 4.01; Mac"

            if (strprefixcasecmp(query, "%1B") == 0) {
                char *agent = getenv("HTTP_USER_AGENT");
                if (agent && strprefixcasecmp(agent, MSIE4MAC) == 0) {
                    decode_uri(query);
                }
            }
#endif MSIE4MACFIX

	} else if (strprefixcasecmp(qs, "subquery=") == 0) {
	    qs += strlen("subquery=");

	    for (i = 0; *qs && *qs != '&'; qs++, i++) {
                *(subquery + i) = *qs;
	    }
	    *(subquery + i) = '\0';
            decode_uri(subquery);
	    if (strlen(subquery) > QUERY_MAX) {
                print(MSG_MIME_HEADER);
		html_print(_(MSG_TOO_LONG_QUERY));
		exit(1);
	    }

#ifdef MSIE4MACFIX
#define MSIE4MAC "Mozilla/4.0 (compatible; MSIE 4.01; Mac"

            if (strprefixcasecmp(subquery, "%1B") == 0) {
                char *agent = getenv("HTTP_USER_AGENT");
                if (agent && strprefixcasecmp(agent, MSIE4MAC) == 0) {
                    decode_uri(subquery);
                }
            }
#endif MSIE4MACFIX

	} else if (strprefixcasecmp(qs, "format=short") == 0) {
	    strcpy(Template, "short");
	    qs += strlen("format=short");
	} else if (strprefixcasecmp(qs, "sort=") == 0) {
	    qs += strlen("sort=");
	    if (strprefixcasecmp(qs, "score") == 0) {
		SortMethod    = SORT_BY_SCORE;
		SortOrder = DESCENDING;
		qs += strlen("score");
	    } else if (strprefixcasecmp(qs, "date%3Alate") == 0) {
		SortMethod    = SORT_BY_DATE;
		SortOrder = DESCENDING;
		qs += strlen("date%3Alate");
	    } else if (strprefixcasecmp(qs, "date%3Aearly") == 0) {
		SortMethod    = SORT_BY_DATE;
		SortOrder = ASCENDING;
		qs += strlen("date%3Aearly");
	    } else if (strprefixcasecmp(qs, "field%3A") == 0) {
		int n;
		uchar field[BUFSIZE];

		qs += strlen("field%3A");
		n = strspn(qs, FIELD_SAFE_CHARS);
		strncpy(field, qs, n);
		field[n] = '\0';  /* Hey, don't forget this after strncpy()! */
		set_sort_field(field);
		qs += n;
		SortMethod    = SORT_BY_FIELD;
		if (strprefixcasecmp(qs, "%3Aascending") == 0) {
		    SortOrder = ASCENDING;
		    qs += strlen("%3Aascending");
		} else if (strprefixcasecmp(qs, "%3Adescending") == 0) {
		    SortOrder = DESCENDING;
		    qs += strlen("%3Adescending");
		}
	    } 
	    while (*qs && *qs != '&')
		qs++;
	} else if (strprefixcasecmp(qs, "max=") == 0) {
	    qs += strlen("max=");
	    sscanf(qs, "%d", &HListMax);
	    if (HListMax < 0)
		HListMax = 0;
	    if (HListMax > RESULT_MAX)
		HListMax = RESULT_MAX;
	    while (*qs && *qs != '&')
		qs++;
	} else if (strprefixcasecmp(qs, "whence=") == 0) {
	    qs += strlen("whence=");
	    sscanf(qs, "%d", &HListWhence);
	    if (HListWhence < 0)
		HListWhence = 0;
	    while (*qs && *qs != '&')
		qs++;
	} else if (strprefixcasecmp(qs, "lang=") == 0) {
	    qs += strlen("lang=");

	    for (i = 0; *qs && *qs != '&' && i < BUFSIZE - 1; 
		 i++, qs++)
	    {
		tmp[i] = *qs;
	    }
            tmp[i] = '\0';

            set_lang(tmp);
	    while (*qs && *qs != '&')
		qs++;
	} else if (strprefixcasecmp(qs, "result=") == 0) {
	    qs += strlen("result=");

	    for (i = 0; *qs && *qs != '&' && i <= CGI_RESULT_NAME_MAX; 
		 i++, qs++)
	    {
		tmp[i] = *qs;
	    }
            tmp[i] = '\0';

            strcpy(Template, tmp);
	    while (*qs && *qs != '&')
		qs++;
	} else if (strprefixcasecmp(qs, "reference=off") == 0) {
            NoReference = 1;
            qs += strlen("reference=off");
	} else if (strprefixcasecmp(qs, "idxname=") == 0) {
            uchar *pp;

	    qs += strlen("idxname=");
	    for (i = 0; *qs && *qs != '&' && i <= CGI_INDEX_NAME_MAX; 
		 i++, qs++)
	    {
		tmp[i] = *qs;
	    }
            tmp[i] = '\0';
            decode_uri(tmp);
            for (pp = tmp; *pp ;) {
                uchar name[BUFSIZE], *x;

                if ((x = (uchar *)strchr(pp, (int)','))) {
                    *x = '\0';
                    strcpy(name, pp);
                    pp = x + 1;
                } else {
                    strcpy(name, pp);
                    pp += strlen(pp);
                }
                if (Idx.num >= INDEX_MAX) { /* ignore too many indices */
                    continue;
                }
                Idx.names[Idx.num] = (uchar *)
                    malloc(strlen(DEFAULT_INDEX) + 1 + strlen(name) + 1);
                if (Idx.names[Idx.num] == NULL) {
                    die("cgi: malloc(idxname)");
                }
                validate_idxname(name);
                strcpy(Idx.names[Idx.num], DEFAULT_INDEX);
                strcat(Idx.names[Idx.num], "/");
                strcat(Idx.names[Idx.num], name);
                Idx.num++;
            }
	    while (*qs && *qs != '&')
		qs++;
	} else {
	    qs++;
        }
    }
    return 0;
}

/************************************************************
 *
 * Public functions
 *
 ************************************************************/

/* initialize CGI mode. actually, to be invoked from commandline
 * with no arguments also trhough this function */
void init_cgi(uchar * query, uchar *subquery)
{
    if (get_cgi_vars(query, subquery)) {
	show_mini_usage();	/* if it is NOT CGI, show usage and exit */
	exit(1);
    }
    if (Idx.num == 0) {
        Idx.names[Idx.num] = (uchar *) malloc(strlen(DEFAULT_INDEX) + 1);
	Idx.pr[Idx.num] = NULL;
        if (Idx.names[Idx.num] == NULL) {
            die("cgi_initialize: malloc(idxname)");
        }
        strcpy(Idx.names[Idx.num], DEFAULT_INDEX);
        Idx.num++;
    } 
}


