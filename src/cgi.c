/*
 * 
 * cgi.c -
 * 
 * $Id: cgi.c,v 1.21 1999-11-14 13:55:05 satoru Exp $
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
#include "idxname.h"

/************************************************************
 *
 * Private functions
 *
 ************************************************************/

static int validate_idxname(uchar* );

static CGIVAR *add_cgivar(CGIVAR*, uchar*, uchar*);
static CGIVAR *get_cgi_vars(uchar*);
static uchar *get_query_string(void);
static int process_cgi_vars(CGIARG*);
static int apply_cgifunc(CGIVAR*, CGIARG*);
static void free_cgi_vars(CGIVAR*);

static void decode_uri(uchar*);
static uchar decode_uri_sub(uchar, uchar);

static void process_cgi_var_query(uchar*, CGIARG*);
static void process_cgi_var_subquery(uchar*, CGIARG*);
static void process_cgi_var_format(uchar*, CGIARG*);
static void process_cgi_var_sort(uchar*, CGIARG*);
static void process_cgi_var_max(uchar*, CGIARG*);
static void process_cgi_var_whence(uchar*, CGIARG*);
static void process_cgi_var_lang(uchar*, CGIARG*);
static void process_cgi_var_result(uchar*, CGIARG*);
static void process_cgi_var_reference(uchar*, CGIARG*);
static void process_cgi_var_idxname(uchar*, CGIARG*);
static void process_cgi_var_submit(uchar*, CGIARG*);

/* Table for cgi vars and corresponding functions. */
static CGIVAR_FUNC CgiFuncTab[] = {
    { "query",     (void *)process_cgi_var_query },
    { "key",       (void *)process_cgi_var_query },  /* backward comat. */
    { "subquery",  (void *)process_cgi_var_subquery },
    { "format",    (void *)process_cgi_var_format }, /* backward comat. */
    { "sort",      (void *)process_cgi_var_sort },
    { "max",       (void *)process_cgi_var_max },
    { "whence",    (void *)process_cgi_var_whence },
    { "lang",      (void *)process_cgi_var_lang },
    { "result",    (void *)process_cgi_var_result },
    { "reference", (void *)process_cgi_var_reference },
    { "idxname",   (void *)process_cgi_var_idxname },
    { "dbname",    (void *)process_cgi_var_idxname },  /* backward comat. */
    { "submit",    (void *)process_cgi_var_submit },
    { NULL,        (void *)NULL }   /* sentry */
};


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

static CGIVAR *add_cgivar(CGIVAR *ptr, uchar *name, uchar *value)
{
    CGIVAR *tmp;

    tmp = malloc(sizeof(CGIVAR));
    if (tmp == NULL) {
	 printf("add_cgivar_malloc");
	 return NULL;
    }
    tmp->name = malloc(strlen(name) + 1);
    if (tmp->name == NULL) {
	 printf("add_cgivar_malloc");
	 return NULL;
    }

    tmp->value = malloc(strlen(value) + 1);
    if (tmp->value == NULL) {
	 printf("add_cgivar_malloc");
	 return NULL;
    }

    strcpy(tmp->name, name);
    strcpy(tmp->value, value);
    tmp->next = ptr;
    return tmp;
}

static void free_cgi_vars(CGIVAR *cv)
{
    if (cv == NULL)
	return;
    free(cv->name);
    free(cv->value);
    free_cgi_vars(cv->next);
    free(cv);
}

static uchar *get_query_string(void) 
{
    int contlen;
    uchar *script_name    = (uchar *)"";
    uchar *query_string   = (uchar *)"";
    uchar *content_length = (uchar *)"";


    if ((query_string = (uchar *)getenv("QUERY_STRING"))) {
        /* 
	 * get QUERY_STRING from environmental variables.
	 */
        contlen = strlen(query_string);
        if (contlen > CGI_QUERY_MAX) {
            printf(MSG_MIME_HEADER);
            printf(_("Too long QUERY_STRING"));
            exit(1);
        }
	script_name = (uchar *)getenv("SCRIPT_NAME");
        if (script_name == NULL) {
            return NULL;
        }
    } else {   
        /* 
	 * get QUERY_STRING from stdin 
	 */
	if ((content_length = (uchar *)getenv("CONTENT_LENGTH"))) {
	    contlen = atoi(content_length);
            if (contlen > CGI_QUERY_MAX) {
                printf(MSG_MIME_HEADER);
                printf(_("Too long QUERY_STRING"));
                exit(1);
            }
            query_string = (uchar *)malloc(contlen * sizeof(uchar) + 1);
	    if (query_string == NULL) {
                printf(MSG_MIME_HEADER);
		printf("query_string(get_cgivar)");
		exit(1);
	    }
	    fread(query_string, sizeof(char), contlen, stdin);
	} else {
	    return NULL;
	}
    }
    return query_string;
}

static CGIVAR *get_cgi_vars(uchar *qs)
{
    CGIVAR *cv = NULL; /* SHOULD BE NULL for sentinel! */

    while (1) {
	int len;
	uchar *tmp;
	uchar name[BUFSIZE];
	uchar value[BUFSIZE];

	tmp = strchr(qs, '=');
	if (tmp == NULL) {
	    break;
	}
	len = tmp - qs;
	strncpy(name, qs, len);
	*(name + len) = '\0';
	qs += len;

	qs++;
	tmp = strchr(qs, '&');
	if (tmp == NULL) {
	    tmp = qs + strlen(qs);  /* last point: '\0' */
	}
	len = tmp - qs;
	strncpy(value, qs, len);
	*(value + len) = '\0';
	decode_uri(value);
	qs += len;

	cv = add_cgivar(cv, name, value);

	if (cv == NULL) {
	    fprintf(stderr, "an error occured at add_cgivar.\n");
	    exit(1);
	}

	if (*qs == '\0') {
	    break;
	}
	qs++;
    }
    return cv;
}

static int process_cgi_vars(CGIARG *ca)
{
    uchar *qs;
    CGIVAR *cv;

    qs = get_query_string();
    if (qs == NULL) {
	return 1; /* NOT CGI */
    }
    cv = get_cgi_vars(qs);

    for (; cv != NULL; cv = cv->next) {
	if (!apply_cgifunc(cv, ca)) {
	    /* message for httpd's error_log */
	    wprintf("unkown cgi var: %s=%s\n", cv->name, cv->value);
	}
    }
    free_cgi_vars(cv);
    return 0;
}

static int apply_cgifunc(CGIVAR *cv, CGIARG *ca) 
{
    CGIVAR_FUNC *cf = CgiFuncTab;
    int i;

    for (i = 0; cf->name != NULL; cf++) {
	if (strcmp(cv->name, cf->name) == 0) {
	    (*(cf->func))(cv->value, ca);
	    return 1;  /* applied */
	}
    }
    return 0; /* not applied */
}

static void process_cgi_var_query(uchar *value, CGIARG *ca)
{
    if (strlen(value) > QUERY_MAX) {
	print(MSG_MIME_HEADER);
	html_print(_(MSG_TOO_LONG_QUERY));
	exit(1);
    }
#ifdef MSIE4MACFIX
#define MSIE4MAC "Mozilla/4.0 (compatible; MSIE 4.01; Mac"

    if (strprefixcasecmp(value, "%1B") == 0) {
	char *agent = getenv("HTTP_USER_AGENT");
	if (agent && strprefixcasecmp(agent, MSIE4MAC) == 0) {
	    decode_uri(value);
	}
    }
#endif MSIE4MACFIX

    strcpy(ca->query, value);
}

static void process_cgi_var_subquery(uchar *value, CGIARG *ca)
{
    if (strlen(value) > QUERY_MAX) {
	print(MSG_MIME_HEADER);
	html_print(_(MSG_TOO_LONG_QUERY));
	exit(1);
    }
#ifdef MSIE4MACFIX
#define MSIE4MAC "Mozilla/4.0 (compatible; MSIE 4.01; Mac"

    if (strprefixcasecmp(value, "%1B") == 0) {
	char *agent = getenv("HTTP_USER_AGENT");
	if (agent && strprefixcasecmp(agent, MSIE4MAC) == 0) {
	    decode_uri(value);
	}
    }
#endif MSIE4MACFIX

    strcpy(ca->subquery, value);
}

/* this function is for backward compatibility with 1.3.0.x */
static void process_cgi_var_format(uchar *value, CGIARG *ca)
{
    if (strcmp(value, "short") == 0) {
	strcpy(Template, "short");
    } else if (strcmp(value, "long") == 0) {
	strcpy(Template, "normal");
    }
}

static void process_cgi_var_sort(uchar *value, CGIARG *ca)
{
    if (strprefixcasecmp(value, "score") == 0) {
	SortMethod  = SORT_BY_SCORE;
	SortOrder   = DESCENDING;
    } if (strprefixcasecmp(value, "later") == 0) {  /* backward compat. */
	SortMethod  = SORT_BY_DATE;
	SortOrder   = DESCENDING;
    } if (strprefixcasecmp(value, "earlier") == 0) { /* backward compat. */
	SortMethod  = SORT_BY_DATE;
	SortOrder   = ASCENDING;
    } else if (strprefixcasecmp(value, "date:late") == 0) {
	SortMethod  = SORT_BY_DATE;
	SortOrder   = DESCENDING;
    } else if (strprefixcasecmp(value, "date:early") == 0) {
	SortMethod  = SORT_BY_DATE;
	SortOrder   = ASCENDING;
    } else if (strprefixcasecmp(value, "field:") == 0) {
	int n;
	uchar field[BUFSIZE];

	value += strlen("field:");
	n = strspn(value, FIELD_SAFE_CHARS);
	strncpy(field, value, n);
	field[n] = '\0';  /* Hey, don't forget this after strncpy()! */
	set_sort_field(field);
	value += n;
	SortMethod    = SORT_BY_FIELD;
	if (strprefixcasecmp(value, ":ascending") == 0) {
	    SortOrder = ASCENDING;
	    value += strlen(":ascending");
	} else if (strprefixcasecmp(value, ":descending") == 0) {
	    SortOrder = DESCENDING;
	    value += strlen(":descending");
	}
    } 
}

static void process_cgi_var_max(uchar *value, CGIARG *ca)
{
    sscanf(value, "%d", &HListMax);
    if (HListMax < 0)
	HListMax = 0;
    if (HListMax > RESULT_MAX)
	HListMax = RESULT_MAX;
}

static void process_cgi_var_whence(uchar *value, CGIARG *ca)
{
    sscanf(value, "%d", &HListWhence);
    if (HListWhence < 0) {
	HListWhence = 0;
    }
}

static void process_cgi_var_lang(uchar *value, CGIARG *ca)
{
    set_lang(value);
}

static void process_cgi_var_result(uchar *value, CGIARG *ca)
{
    strcpy(Template, value);
}

static void process_cgi_var_reference(uchar *value, CGIARG *ca)
{
    if (strcmp(value, "off") == 0) {
	NoReference = 1;
    }
    
}

static void process_cgi_var_submit(uchar *value, CGIARG *ca)
{
    /* do nothing; */
}

static void process_cgi_var_idxname(uchar *value, CGIARG *ca)
{
    uchar *pp;

    for (pp = value; *pp ;) {
	uchar name[BUFSIZE], tmp[BUFSIZE], *x;

	if ((x = (uchar *)strchr(pp, (int)','))) {
	    *x = '\0';
	    strcpy(name, pp);
	    pp = x + 1;
	} else {
	    strcpy(name, pp);
	    pp += strlen(pp);
	}
	validate_idxname(name);
	strcpy(tmp, DEFAULT_INDEX);
	strcat(tmp, "/");
	strcat(tmp, name);
	add_index(tmp);
    }
}

/* decoding URI encoded strings */
static void decode_uri(uchar * s)
{
    int i, j;
    for (i = j = 0; s[i]; i++, j++) {
	if (s[i] == '%') {
	    s[j] = decode_uri_sub(s[i + 1], s[i + 2]);
	    i += 2;
	} else if (s[i] == '+') {
	    s[j] = ' ';
	} else {
	    s[j] = s[i];
	}
    }
    s[j] = '\0';
}

static uchar decode_uri_sub(uchar c1, uchar c2)
{
    uchar c;

    c =  ((c1 >= 'A' ? (toupper(c1) - 'A') + 10 : (c1 - '0'))) * 16;
    c += ( c2 >= 'A' ? (toupper(c2) - 'A') + 10 : (c2 - '0'));
    return c;
}


/************************************************************
 *
 * Public functions
 *
 ************************************************************/

/* initialize CGI mode. actually, to be invoked from commandline
 * with no arguments also trhough this function */
void init_cgi(uchar *query, uchar *subquery)
{
    CGIARG ca;  /* passed for process_cgi_var_*() functions 
		   for modifying important variables. */

    ca.query    = query;
    ca.subquery = subquery;

    if (process_cgi_vars(&ca)) {
	show_mini_usage();   /* if it is NOT CGI, show usage and exit */
	exit(1);
    }

    if (Idx.num == 0) {
	add_index(DEFAULT_INDEX);
    } 
}


