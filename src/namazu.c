
/*
 * 
 * namazu.c - search client of Namazu
 *
 * $Id: namazu.c,v 1.3 1999-05-14 04:38:50 satoru Exp $
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
    uchar buf[1024];
    strcpy(buf, MSG_USAGE);
#if	defined(_WIN32) || defined(__EMX__)
    euctosjis(buf);
#endif
    printf(buf, COPYRIGHT, VERSION);
    exit(0);
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

/* get command line options */
int get_commandline_opt(int ac, uchar **av)
{
    int i, j;

    for (i = 1; i < ac; i++) {
	if (av[i][0] == '-') {
            for (j = 1;  av[i][j] != '\0' ; j++) {
                switch (av[i][j]) {
                case 'n':
                    if (av[i][j + 1] == '\0' && i < ac - 1)
                        HListMax = atoi(av[++i]);
                    goto LOOP1;
                case 'w':
                    if (av[i][j + 1] == '\0' && i < ac - 1)
                        HListWhence = atoi(av[++i]);
                    goto LOOP1;
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
                    DecodeURL = 0;
                    break;
                case 'v':
                    show_usage();
                    break;
                case 'C':
                    load_namazu_conf(av[0]);
                    show_configuration();
                    break;
                case 'f':
                    if (av[i][j + 1] == '\0' && i < ac - 1)
                        strcpy(NAMAZURC, av[++i]);
                    load_namazu_conf(av[0]);
                    goto LOOP1;
                case 'L':
                    if (av[i][j + 1] == '\0' && i < ac - 1) {
                        strncpy(Lang, av[i + 1], 2);
                        i++;
                        initialize_message();
                    }
                    goto LOOP1;
                case 'o':
                    if (av[i][j + 1] == '\0' && i < ac - 1)
                        set_redirect_stdout_to_file(av[++i]);
                    goto LOOP1;
                case 't':
                    if (av[i][j + 1] == '\0' && i < ac - 1) {
                        if (!strcmp(av[i + 1], "SIMPLE"))
                            TfIdf = 0;
                        if (!strcmp(av[i + 1], "TFIDF"))
                            TfIdf = 1;
                        i++;
                    }
                    goto LOOP1;
		case '-':
		    i++;
		    goto out;
                }
            } 
        } else {
            break;
        }
    LOOP1: ;
    }
out:
    return i;
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
    int legacy_dos_fs = 0;
#if  defined(_WIN32) || defined(__EMX__)
    legacy_dos_fs = 1;   /* miserable!! */
#endif

    for (i = 0; i < DbNumber; i++) {
 	if (isalnum(*DbNames[i]) && !(legacy_dos_fs && *(DbNames[i] + 1) == ':')) {
	    uchar *tmp;
	    tmp = (uchar *)malloc(strlen(DEFAULT_DIR) + 1 + strlen(DbNames[i]) + 1);
	    if (tmp == NULL) {
		error("complete_dbnames: malloc()");
	    }
	    strcpy(tmp, DEFAULT_DIR);
	    strcat(tmp, "/");
	    strcat(tmp, DbNames[i]);
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
	DecodeURL = 1;		/* in default mode of command line, 
				 * decode a URL */
	HidePageIndex = 1;	/* in default mode of command line, 
				 * do not diplay page index
				 */
	load_namazu_conf(argv[0]);
        /* do commad line processing */
	i = get_commandline_opt(argc, (uchar **)argv); 
	if (i == argc)
	    show_usage();
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
