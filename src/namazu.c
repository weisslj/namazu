/*
 * 
 * namazu.c - search client of Namazu
 *
 * $Id: namazu.c,v 1.6 1999-06-12 14:29:30 satoru Exp $
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
#include <sys/types.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "namazu.h"
#include "getopt.h"


/* stupid function intended for debug use */
void error(const char *msg)
{
    FILE *output;

    if (IsCGI) {
        output = stderr;
    } else {
        output = stdout;
    }
    if (HtmlOutput)
	fputs(MSG_MIME_HEADER, output);
    fprintf(output, "%s: Sorry, something error occurred...\n", msg);
    exit(1);
}


/* output a content of file */
void cat(uchar *fname)
{
    uchar buf[BUFSIZE];
    FILE *fp;

    if ((fp = fopen(fname, "rb"))) {
	while (fgets(buf, BUFSIZE, fp))
	    fputs(buf, stdout);
	fclose(fp);
    }
}


/* display the usage and version info and exit */
void show_usage(void)
{
    uchar buf[2048];
    strcpy(buf, MSG_USAGE);
#if	defined(_WIN32) || defined(__EMX__)
    euctosjis(buf);
#endif
    printf(buf, COPYRIGHT, VERSION);
}


/* redirect stdio to specified file */
void set_redirect_stdout_to_file(uchar * fname)
{
    int fd;

    if (-1 == (fd = open(fname, O_CREAT | O_TRUNC | O_WRONLY, 00600)))
	error("stdio2file(cannot open)");
    close(STDOUT);
    dup(fd);
    close(STDERR);
    dup(fd);
    close(fd);
}

/*
 * Command line options.
 */
static const char *short_options = "acCefFhHlL:n:o:qrRsSUvw0";
static struct option long_options[] = {
    {"all",              no_argument,       NULL, 'a'},
    {"hit-count",        no_argument,       NULL, 'c'},
    {"show-config",      no_argument,       NULL, 'C'},
    {"early",            no_argument,       NULL, 'e'},
    {"config",           no_argument,       NULL, 'f'},
    {"form",             no_argument,       NULL, 'F'},
    {"html",             no_argument,       NULL, 'h'},
    {"page",             no_argument,       NULL, 'H'},
    {"late",             no_argument,       NULL, 'l'},
    {"lang",             required_argument, NULL, 'L'},
    {"max",              required_argument, NULL, 'n'},
    {"output-file",      required_argument, NULL, 'o'},
    {"quiet",            no_argument,       NULL, 'q'},
    {"no-ref",           no_argument,       NULL, 'r'},
    {"no-replace-uri",   no_argument,       NULL, 'R'},
    {"short",            no_argument,       NULL, 's'},
    {"very-short",       no_argument,       NULL, 'S'},
    {"no-encode-uri",    no_argument,       NULL, 'U'},
    {"version",          no_argument,       NULL, 'v'},
    {"whence",           required_argument, NULL, 'w'},
    {"help",             no_argument,       NULL, '0'},
    {NULL, 0, NULL, 0}
};

/* parse command line options */
int parse_options(int argc, char **argv)
{
    for (;;) {
        int ch = getopt_long(argc, argv, short_options, long_options, NULL);
        if (ch == EOF) {
            break;
	}
	switch (ch) {
	case 'n':
	    HListMax = atoi(optarg);
	    break;
	case 'w':
	    HListWhence = atoi(optarg);
	    break;
	case 'd':
	    Debug = 1;
	    break;
	case 's':
	    ShortFormat = 1;
	    break;
	case 'S':
	    MoreShortFormat = 1;
	    break;
	case 'q':
	    Quiet = 1;
	    break;
	case 'c':
	    HitCountOnly = 1;
	    break;
	case 'h':
	    HtmlOutput = 1;
	    break;
	case 'H':
	    HidePageIndex = 0;
	    break;
	case 'F':
	    ForcePrintForm = 1;
	    break;
	case 'a':
	    AllList = 1;
	    break;
	case 'l':
	    LaterOrder = 1;
	    ScoreSort = 0;
	    break;
	case 'e':
	    LaterOrder = 0;
	    ScoreSort = 0;
	    break;
	case 'R':
	    NoReplace = 1;
	    break;
	case 'r':
	    NoReference = 1;
	    break;
	case 'U':
	    DecodeURI = 0;
	    break;
	case 'v':
	    printf("namazu v%s\n", VERSION);
	    exit(0);
	    break;
	case '0':
	    show_usage();
	    exit(0);
	    break;
	case 'C':
	    load_namazu_conf(argv[0]);
	    show_configuration();
	    break;
	case 'f':
	    strcpy(NAMAZURC, optarg);
	    load_namazu_conf(argv[0]);
	    break;
	case 'L':
	    strncpy(Lang, optarg, 2);
	    initialize_message();
	    break;
	case 'o':
	    set_redirect_stdout_to_file(optarg);
	    break;
	}
    } 

    return optind;
}

void free_dbnames(void)
{
    int i;
    for (i = 0; i < DbNumber; i++) {
        free(DbNames[i]);
    }
}

void pathcat(uchar *base, uchar name[])
{
    uchar work[BUFSIZE];
    int i;

    for (i = strlen(name) - 1; i >= 0; i--) {
        if (name[i] == '/') {
            strcpy(name, name + i + 1);
            break;
        }
    }
    strcpy(work, base);
    strcat(work, "/");
    strcat(work, name);
    strcpy(name, work);
}


void make_fullpathname_msg(void)
{
    uchar *base;
    
    if (DbNumber == 1) {
        base = DbNames[0];
    } else {
        base = DEFAULT_DIR;
    }
    
    pathcat(base, HEADERFILE);
    pathcat(base, FOOTERFILE);
    pathcat(base, LOCKFILE);
    pathcat(base, BODYMSGFILE);
}

/* namazu core routine */
void namazu_core(uchar * query, uchar *subquery, uchar *av0)
{
    uchar query_with_subquery[BUFSIZE * 2];

    /* make full-pathname of NMZ.{head,foot,msg,body,slog}.?? */
    make_fullpathname_msg();
    codeconv_query(query); /* code conversion */
    codeconv_query(subquery); /* code conversion */
    strcpy(query_with_subquery, query);
    strcat(query_with_subquery, " ");
    strcat(query_with_subquery, subquery);

    /* if query is null, show NMZ.head,body,foot and exit with error */
    if (*query == '\0') {
        if (HtmlOutput) {
            fputs(MSG_MIME_HEADER, stdout);
            cat_head_or_foot(HEADERFILE, query, subquery);
            cat(BODYMSGFILE);
            cat_head_or_foot(FOOTERFILE, query, subquery);
        }
	exit(1);
    }

    if (Debug) {
	fprintf(stderr, " -n: %d\n", HListMax);
	fprintf(stderr, " -w: %d\n", HListWhence);
	fprintf(stderr, "key: [%s]\n", query);
    }

    if (HtmlOutput && IsCGI)
	fputs(MSG_MIME_HEADER, stdout);
    if (HtmlOutput)
	cat_head_or_foot(HEADERFILE, query, subquery);

    search_main(query_with_subquery);

    if (HtmlOutput)
	cat_head_or_foot(FOOTERFILE, query, subquery);
    free_dbnames();
}


/* get an environmental variable of NAMAZUCONFPATH
 * original by Shimizu-san [1998-02-27]
 */
void getenv_namazuconf(void)
{
    uchar *env_namazu_conf;

    env_namazu_conf = (uchar*)getenv("NAMAZUCONFPATH");
    if (env_namazu_conf == NULL)
        env_namazu_conf = (uchar*)getenv("NAMAZUCONF");

    if (env_namazu_conf != NULL)
        strcpy(NAMAZURC, env_namazu_conf);
}

void uniq_dbnames(void)
{
    int i, j, k;

    for (i = 0; i < DbNumber - 1; i++) {
        for (j = i + 1; j < DbNumber; j++) {
            if (!strcmp(DbNames[i], DbNames[j])) {
                free(DbNames[j]);
                for (k = j + 1; k < DbNumber; k++) {
                    DbNames[k - 1] = DbNames[k];
                }
                DbNumber--;
                j--;
            }
        }
    }
}

void expand_dbname_aliases(void)
{
    int i;

    for (i = 0; i < DbNumber; i++) {
	ALIAS list = Alias;
	while (list.alias) {
	    if (!strcmp(DbNames[i], list.alias->str)) {
		free(DbNames[i]);
		DbNames[i] = (uchar *) malloc(strlen(list.real->str) + 1);
		if (DbNames[i] == NULL) {
		    error("expand_dbname_aliases: malloc()");
		}
		strcpy(DbNames[i], list.real->str);
            }
	    list.alias = list.alias->next;
	    list.real  = list.real->next;
	}
    }
}

void complete_dbnames(void)
{
    int i;

    for (i = 0; i < DbNumber; i++) {
 	if (*DbNames[i] == '+' && isalnum(*(DbNames[i] + 1))) {
	    uchar *tmp;
	    tmp = (uchar *)malloc(strlen(DEFAULT_DIR) 
				  + 1 + strlen(DbNames[i]) + 1);
	    if (tmp == NULL) {
		error("complete_dbnames: malloc()");
	    }
	    strcpy(tmp, DEFAULT_DIR);
	    strcat(tmp, "/");
	    strcat(tmp, DbNames[i] + 1);  /* +1 means '+' */
	    free(DbNames[i]);
	    DbNames[i] = tmp;
	}
    }
}


/* main function */
int main(int argc, char **argv)
{
    int i = 0;
    uchar query[BUFSIZE] = "", subquery[BUFSIZE] = "";

    getenv_namazuconf();
    initialize_message();
    if (argc == 1) {
	load_namazu_conf(argv[0]);
	IsCGI = 1;		/* if no argument, assume as CGI */
	HtmlOutput = 1;
    } else {
	HtmlOutput = 0;		/* in default mode of command line, 
				 * do not display result by HTML format */
	DecodeURI = 1;		/* in default mode of command line, 
				 * decode a URI */
	HidePageIndex = 1;	/* in default mode of command line, 
				 * do not diplay page index
				 */
	load_namazu_conf(argv[0]);
        /* parse commad line options */
	i = parse_options(argc, argv); 
	if (i == argc) {
	    show_usage();
	    exit(0);
	}
	if (strlen(argv[i]) > QUERY_MAX_LENGTH) {
	    fputx(MSG_TOO_LONG_KEY, stdout);
	    return 1;
	}
        strcpy(query, argv[i++]);
        if (i < argc) {
            for (DbNumber = 0; i < argc && DbNumber < DB_MAX; i++) {
                if (strlen(argv[i]) <= DBNAMELENG_MAX) {
                    DbNames[DbNumber] = 
                        (uchar *) malloc(strlen(argv[i]) + 1);
                    if (DbNames[DbNumber] == NULL) {
                        error("main: malloc(dbname)");
                    }
                    strcpy(DbNames[DbNumber], argv[i]);
                    DbNumber++;
                }
            }
        } 
        if (DbNumber == 0) {
            DbNumber = 0;
            DbNames[DbNumber] = 
                (uchar *) malloc(strlen(DEFAULT_DIR) + 1);
            if (DbNames[DbNumber] == NULL) {
                error("main: malloc(dbname)");
            }
            strcpy(DbNames[DbNumber], DEFAULT_DIR);
            DbNumber = 1;
	}
    }
    if (IsCGI)
	cgi_initialize(query, subquery);

    uniq_dbnames();
    expand_dbname_aliases();
    complete_dbnames();

    if (Debug) {
        for (i = 0; i < DbNumber; i++) {
            fprintf(stderr, "DbNames[%d]: %s\n", i, DbNames[i]);
        }
    }

    namazu_core(query, subquery, argv[0]);
    return 0;
}
