/*
 * 
 * cgi.c -
 * 
 * $Id: cgi.c,v 1.8 1999-08-25 03:43:59 satoru Exp $
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
#include <unistd.h>
#include <ctype.h>
#include "namazu.h"
#include "util.h"
#include "usage.h"
#include "message.h"
#include "cgi.h"
#include "output.h"

uchar *ScriptName    = (uchar *)"";
uchar *QueryString   = (uchar *)"";
uchar *ContentLength = (uchar *)"";

/************************************************************
 *
 * Private functions
 *
 ************************************************************/

int validate_dbname(uchar* );
int get_cgi_vars(uchar* , uchar*);

/* validate dbname (if it contain '/', it's invalid) */
int validate_dbname(uchar * dbname)
{
    int win32 = 0;
#if  defined(_WIN32) || defined(__EMX__)
    win32 = 1;
#endif

    if (*dbname == '\0' || *dbname == '/' || (win32 && *dbname == '\\')) {
        fputs(MSG_MIME_HEADER, stdout);
        fputx(MSG_INVALID_DB_NAME, stdout);
        exit(1);
    }
    while (*dbname) {
        if (strncmp("../", dbname, 3) == 0 ||
	    strcmp("..", dbname) == 0 ||
            (win32 && strncmp("..\\", dbname, 3) == 0)) 
        {
            fputs(MSG_MIME_HEADER, stdout);
            fputx(MSG_INVALID_DB_NAME, stdout);
            exit(1);
        }
	/* Skip until next '/' */
	while (*dbname && *dbname != '/' && !(win32 && *dbname == '\\'))
	  dbname++;
	/* Skip '/' */
	if (*dbname)
	  dbname++;
    }
    dbname--;  /* remove ending slashed */
    while (*dbname == '/' || (win32 && *dbname == '\\')) {
        *dbname = '\0';
        dbname--;
    }
    return 1;
}


/*  get CGI vars, very complicated routine */
int get_cgi_vars(uchar * query, uchar *subquery)
{
    int i;
    uchar *qs, tmp[BUFSIZE];
    int content_length;

    if ((QueryString = (uchar *)getenv("QUERY_STRING"))) {
        content_length = strlen(QueryString);
        if (content_length > QUERY_STRING_MAX_LENGTH) {
            fputs(MSG_MIME_HEADER, stdout);
            fputx(MSG_QUERY_STRING_TOO_LONG, stdout);
            exit(1);
        }
        if (!(ScriptName = (uchar *)getenv("SCRIPT_NAME"))) {
            return 1;
        }
    } else {
	if ((ContentLength = (uchar *)getenv("CONTENT_LENGTH"))) {
	    content_length = atoi(ContentLength);
            if (content_length > QUERY_STRING_MAX_LENGTH) {
                fputs(MSG_MIME_HEADER, stdout);
                fputx(MSG_QUERY_STRING_TOO_LONG, stdout);
                exit(1);
            }
            QueryString = (uchar *)malloc(content_length * sizeof(uchar) + 1);
	    if (QueryString == NULL)
		die("QueryString( get_cgi_vars )");

	    fread(QueryString, sizeof(char), content_length, stdin);
	    HtmlOutput = 1;
	    HidePageIndex = 1;
	} else {
	    return 1;
	}
    }
    qs = QueryString;
    DbNumber = 0;

    /* note that CERN HTTPD would add empty PATH_INFO */
    if (getenv("PATH_INFO")) {
        uchar *path_info = (uchar *)getenv("PATH_INFO");
        if (strlen(path_info) > 0 && strlen(path_info) < 128) {
            validate_dbname(path_info);
            sprintf(tmp, "%s%s", DEFAULT_DIR, path_info);
            DbNames[DbNumber] = (uchar *) malloc(strlen(tmp) + 1);
            if (DbNames[DbNumber] == NULL) {
                die("cgi: malloc(dbname)");
            }
            strcpy(DbNames[DbNumber], tmp);
            DbNumber++;
        }
    }

    while (*qs) {
        if (*qs == '&') {
            qs++;
            continue;
        } else if (!strncmp(qs, "key=", 4)) {
	    qs += 4;

	    for (i = 0; *qs && *qs != '&'; qs++, i++) {
                *(query + i) = *qs;
	    }
	    *(query + i) = '\0';
            decode_uri(query);
	    if (strlen(query) > QUERY_MAX_LENGTH) {
                fputs(MSG_MIME_HEADER, stdout);
		fputx(MSG_TOO_LONG_KEY, stdout);
		exit(1);
	    }

#ifdef MSIE4MACFIX
#define MSIE4MAC "Mozilla/4.0 (compatible; MSIE 4.01; Mac"

            if (!strncmp(query, "%1B", 3)) {
                char *agent = getenv("HTTP_USER_AGENT");
                if (agent && !strncmp(agent, MSIE4MAC, strlen(MSIE4MAC))) {
                    decode_uri(query);
                }
            }
#endif MSIE4MACFIX

	} else if (!strncmp(qs, "subquery=", 4)) {
	    qs += 9;

	    for (i = 0; *qs && *qs != '&'; qs++, i++) {
                *(subquery + i) = *qs;
	    }
	    *(subquery + i) = '\0';
            decode_uri(subquery);
	    if (strlen(subquery) > QUERY_MAX_LENGTH) {
                fputs(MSG_MIME_HEADER, stdout);
		fputx(MSG_TOO_LONG_KEY, stdout);
		exit(1);
	    }

#ifdef MSIE4MACFIX
#define MSIE4MAC "Mozilla/4.0 (compatible; MSIE 4.01; Mac"

            if (!strncmp(subquery, "%1B", 3)) {
                char *agent = getenv("HTTP_USER_AGENT");
                if (agent && !strncmp(agent, MSIE4MAC, strlen(MSIE4MAC))) {
                    decode_uri(subquery);
                }
            }
#endif MSIE4MACFIX

	} else if (!strncmp(qs, "format=short", 12)) {
	    ShortFormat = 1;
	    qs += 12;
	} else if (!strncmp(qs, "sort=", 5)) {
	    qs += 5;
	    if (!strncmp(qs, "score", 5)) {
		ScoreSort = 1;
		qs += 5;
	    } else if (!strncmp(qs, "later", 5)) {
		LaterOrder = 1;
		ScoreSort = 0;
		qs += 5;
	    } else if (!strncmp(qs, "earlier", 7)) {
		LaterOrder = 0;
		ScoreSort = 0;
		qs += 7;
	    }
	    while (*qs && *qs != '&')
		qs++;
	} else if (!strncmp(qs, "max=", 4)) {
	    qs += 4;
	    sscanf(qs, "%d", &HListMax);
	    if (HListMax < 0)
		HListMax = 0;
	    if (HListMax > HLIST_MAX_MAX)
		HListMax = HLIST_MAX_MAX;
	    while (*qs && *qs != '&')
		qs++;
	} else if (!strncmp(qs, "whence=", 7)) {
	    qs += 7;
	    sscanf(qs, "%d", &HListWhence);
	    if (HListWhence < 0)
		HListWhence = 0;
	    while (*qs && *qs != '&')
		qs++;
	} else if (!strncmp(qs, "lang=", 5)) {
	    qs += 5;
            strncpy(Lang, qs, 2);
            qs += 2;
            init_message();
	    while (*qs && *qs != '&')
		qs++;
	} else if (!strncmp(qs, "reference=off", 13)) {
            NoReference = 1;
            qs += 13;
	} else if (!strncmp(qs, "dbname=", 7)) {
            uchar *pp;

	    qs += 7;
	    for (i = 0; *qs && *qs != '&' && i <= DBNAMELENG_MAX; i++, qs++)
		tmp[i] = *qs;
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
                if (DbNumber >= DB_MAX) { /* ignore too many indices */
                    continue;
                }
                DbNames[DbNumber] = (uchar *)
                    malloc(strlen(DEFAULT_DIR) + 1 + strlen(name) + 1);
                if (DbNames[DbNumber] == NULL) {
                    die("cgi: malloc(dbname)");
                }
                validate_dbname(name);
                strcpy(DbNames[DbNumber], DEFAULT_DIR);
                strcat(DbNames[DbNumber], "/");
                strcat(DbNames[DbNumber], name);
                DbNumber++;
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
	show_usage();	/* if it is NOT CGI, show usage and exit */
	exit(1);
    }
    if (DbNumber == 0) {
        DbNames[DbNumber] = (uchar *) malloc(strlen(DEFAULT_DIR) + 1);
        if (DbNames[DbNumber] == NULL) {
            die("cgi_initialize: malloc(dbname)");
        }
        strcpy(DbNames[DbNumber], DEFAULT_DIR);
        DbNumber++;
    } 
}




