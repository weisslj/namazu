/*
 * 
 * namazu.c - search client of Namazu
 *
 * $Id: namazu.c,v 1.22 1999-09-02 00:14:49 satoru Exp $
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
#include <signal.h>

#include "getopt.h"
#include "util.h"
#include "codeconv.h"
#include "form.h"
#include "usage.h"
#include "conf.h"
#include "output.h"
#include "search.h"
#include "cgi.h"
#include "namazu.h"

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


/* redirect stdio to specified file */
void set_redirect_stdout_to_file(uchar * fname)
{
    int fd;

    if (-1 == (fd = open(fname, O_CREAT | O_TRUNC | O_WRONLY, 00600)))
	die("stdio2file(cannot open)");
    close(STDOUT);
    dup(fd);
    close(STDERR);
    dup(fd);
    close(fd);
}

/*
 * Command line options.
 */
static const char *short_options = "01acCedfFhHlL:n:o:qrRsSUvw";
static struct option long_options[] = {
    {"help",             no_argument,       NULL, '0'},
    {"result",           required_argument, NULL, '1'},
    {"late",             no_argument,       NULL, '2'},
    {"early",            no_argument,       NULL, '3'},
    {"all",              no_argument,       NULL, 'a'},
    {"count",            no_argument,       NULL, 'c'},
    {"show-config",      no_argument,       NULL, 'C'},
    {"debug",            no_argument,       NULL, 'd'},
    {"config",           no_argument,       NULL, 'f'},
    {"form",             no_argument,       NULL, 'F'},
    {"html",             no_argument,       NULL, 'h'},
    {"page",             no_argument,       NULL, 'H'},
    {"lang",             required_argument, NULL, 'L'},
    {"max",              required_argument, NULL, 'n'},
    {"output",           required_argument, NULL, 'o'},
    {"quiet",            no_argument,       NULL, 'q'},
    {"no-references",    no_argument,       NULL, 'r'},
    {"no-replace-uri",   no_argument,       NULL, 'R'},
    {"short",            no_argument,       NULL, 's'},
    {"very-short",       no_argument,       NULL, 'S'},
    {"no-encode-uri",    no_argument,       NULL, 'U'},
    {"version",          no_argument,       NULL, 'v'},
    {"whence",           required_argument, NULL, 'w'},
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
	    strcpy(Template, "short");
	    break;
	case 'l':
	case 'S':  /* 'S' for backward compatibility */
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
	case '2':
	    LaterOrder = 1;
	    ScoreSort = 0;
	    break;
	case '3':
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
	    show_long_usage();
	    exit(0);
	    break;
	case '1':
	    strcpy(Template, optarg);
	    break;
	case 'C':
	    show_conf();
	    break;
	case 'f':
	    strcpy(NAMAZURC, optarg);
	    load_conf(argv[0]);
	    break;
	case 'L':
	    strncpy(Lang, optarg, 2);
	    init_message();
	    break;
	case 'o':
	    set_redirect_stdout_to_file(optarg);
	    break;
	}
    } 

    return optind;
}

void free_idxnames(void)
{
    int i;
    for (i = 0; i < Idx.num; i++) {
        free(Idx.names[i]);
    }
}

void make_fullpathname_msg(void)
{
    uchar *base;
    
    if (Idx.num == 1) {
        base = Idx.names[0];
    } else {
        base = DEFAULT_INDEX;
    }
    
    pathcat(base, NMZ.head);
    pathcat(base, NMZ.foot);
    pathcat(base, NMZ.body);
    pathcat(base, NMZ.lock);
}

void codeconv_query(uchar *query)
{
    if (is_lang_ja(Lang)) {
        if (codeconv(query)) {
            zen2han(query);
        }
    }
}

/* namazu core routine */
void namazu_core(uchar * query, uchar *subquery, uchar *av0)
{
    uchar query_with_subquery[BUFSIZE * 2];

    /* make full-pathname of NMZ.{head,foot,msg,body,slog}.?? */
    make_fullpathname_msg();

    codeconv_query(query);
    codeconv_query(subquery);
    strcpy(query_with_subquery, query);
    strcat(query_with_subquery, " ");
    strcat(query_with_subquery, subquery);

    /* if query is null, show NMZ.head,body,foot and exit with error */
    if (*query == '\0') {
        if (HtmlOutput) {
            fputs(MSG_MIME_HEADER, stdout);
            print_headfoot(NMZ.head, query, subquery);
            cat(NMZ.body);
            print_headfoot(NMZ.foot, query, subquery);
        }
	free_idxnames();
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
	print_headfoot(NMZ.head, query, subquery);

    search_main(query_with_subquery);

    if (HtmlOutput)
	print_headfoot(NMZ.foot, query, subquery);
    free_idxnames();
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

void uniq_idxnames(void)
{
    int i, j, k;

    for (i = 0; i < Idx.num - 1; i++) {
        for (j = i + 1; j < Idx.num; j++) {
            if (!strcmp(Idx.names[i], Idx.names[j])) {
                free(Idx.names[j]);
                for (k = j + 1; k < Idx.num; k++) {
                    Idx.names[k - 1] = Idx.names[k];
                }
                Idx.num--;
                j--;
            }
        }
    }
}

void expand_idxname_aliases(void)
{
    int i;

    for (i = 0; i < Idx.num; i++) {
	ALIAS *list = Alias;
	while (list) {
	    if (strcmp(Idx.names[i], list->alias) == 0) {
		free(Idx.names[i]);
		Idx.names[i] = (uchar *) malloc(strlen(list->real) + 1);
		if (Idx.names[i] == NULL) {
		    die("expand_idxname_aliases: malloc()");
		}
		strcpy(Idx.names[i], list->real);
            }
	    list = list->next;
	}
    }
}

void complete_idxnames(void)
{
    int i;

    for (i = 0; i < Idx.num; i++) {
 	if (*Idx.names[i] == '+' && isalnum(*(Idx.names[i] + 1))) {
	    uchar *tmp;
	    tmp = (uchar *)malloc(strlen(DEFAULT_INDEX) 
				  + 1 + strlen(Idx.names[i]) + 1);
	    if (tmp == NULL) {
		die("complete_idxnames: malloc()");
	    }
	    strcpy(tmp, DEFAULT_INDEX);
	    strcat(tmp, "/");
	    strcat(tmp, Idx.names[i] + 1);  /* +1 means '+' */
	    free(Idx.names[i]);
	    Idx.names[i] = tmp;
	}
    }
}

void suicide ()
{
    die("processing time exceeds a limit: %d", SUICIDE_TIME);
}

int main(int argc, char **argv)
{
    int i = 0;
    uchar query[BUFSIZE] = "", subquery[BUFSIZE] = "";

    setprogname(argv[0]);

    Idx.num = 0;

    getenv_namazuconf();
    init_message();
    if (argc == 1) {
	load_conf(argv[0]);
	IsCGI = 1;	/* if no argument, assume this session as CGI */
	HtmlOutput = 1;
    } else {
	HtmlOutput = 0;		 /* do not display result by HTML format */
	DecodeURI = 1;		 /* decode a URI */
	HidePageIndex = 1;	 /* do not diplay page index */

	load_conf(argv[0]);

	i = parse_options(argc, argv); 
	if (i == argc) {
	    show_mini_usage();
	    exit(1);
	}
	if (strlen(argv[i]) > QUERY_MAX) {
	    fputx(MSG_TOO_LONG_KEY, stdout);
	    return 1;
	}
        strcpy(query, argv[i++]);
        if (i < argc) {
            for (Idx.num = 0; i < argc && Idx.num < INDEX_MAX; i++) {
		Idx.names[Idx.num] = 
		    (uchar *) malloc(strlen(argv[i]) + 1);
		if (Idx.names[Idx.num] == NULL) {
		    die("main: malloc(idxname)");
		}
		strcpy(Idx.names[Idx.num], argv[i]);
		Idx.num++;
            }
        } 
        if (Idx.num == 0) {
            Idx.num = 0;
            Idx.names[Idx.num] = 
                (uchar *) malloc(strlen(DEFAULT_INDEX) + 1);
            if (Idx.names[Idx.num] == NULL) {
                die("main: malloc(idxname)");
            }
            strcpy(Idx.names[Idx.num], DEFAULT_INDEX);
            Idx.num = 1;
	}
    }
    if (IsCGI) {
	init_cgi(query, subquery);
    }

    uniq_idxnames();
    expand_idxname_aliases();
    complete_idxnames();

    if (Debug) {
        for (i = 0; i < Idx.num; i++) {
            fprintf(stderr, "Idx.names[%d]: %s\n", i, Idx.names[i]);
        }
    }

    if (IsCGI) {
	/* set a suicide timer */
	signal(SIGALRM, suicide);
	alarm(SUICIDE_TIME);
    }

    namazu_core(query, subquery, argv[0]);
    return 0;
}
