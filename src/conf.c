/*
 * 
 * conf.c -
 * 
 * $Id: conf.c,v 1.10 1999-09-04 08:25:10 satoru Exp $
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
#include "namazu.h"
#include "util.h"
#include "conf.h"
#include "codeconv.h"

static uchar *errmsg = NULL;

/************************************************************
 *
 * Private functions
 *
 ************************************************************/

void set_pathname(char*, char*, char*);
FILE *open_conf_file(char*);
ALIAS *add_alias(ALIAS*, uchar*, uchar*);
void parse_conf(uchar *, int);
int get_conf_args(uchar*, uchar*, uchar*, uchar*);
int get_conf_arg(uchar*, uchar*);
void replace_home(uchar *);
void apply_conf(uchar*, uchar*, uchar*);
int check_argnum(uchar*, int);

/* change filename in full pathname */
void set_pathname(char *to, char *o, char *name)
{
    int i;

    strcpy(to, o);
    for (i = strlen(to) - 1; i > 0; i--) {
	if (to[i] == '/') {
	    i++;
	    break;
	}
    }
    strcpy(to + i, name);
    return;
}

ALIAS *add_alias(ALIAS *ptr, uchar *alias, uchar *real)
{
    ALIAS *tmp;
    
    tmp = malloc(sizeof *tmp);
    if (tmp == NULL) {
	 die("add_alias_malloc");
    }
    tmp->alias = malloc(strlen(alias) + 1);
    if (tmp->alias == NULL) {
	 die("add_alias_malloc");
    }

    tmp->real = malloc(strlen(real) + 1);
    if (tmp->alias == NULL) {
	 die("add_alias_malloc");
    }

    strcpy(tmp->alias, alias);
    strcpy(tmp->real, real);
    tmp->next = ptr;
    return tmp;
}


/*
 1. current_executing_binary_dir/.namazurc
 2. ${HOME}/.namazurc
 3. lib/namazu.conf
 */
FILE *open_conf_file(char *av0)
{
    FILE *fp;
    char fname[BUFSIZE], *home;

    /* be invoked with -f option to specify rcfile */
    if (NAMAZURC[0]) {
        if ((fp = fopen(NAMAZURC, "rb"))) {
            strcpy(NAMAZU_CONF, NAMAZURC);
            return fp;
        }
    }

    /* check the where program is */
    set_pathname(fname, av0, ".namazurc");
    if ((fp = fopen(fname, "rb"))) {
        strcpy(NAMAZU_CONF, fname);
        return fp;
    }

    /* checke a home directory */
    if ((home = getenv("HOME")) != NULL) {
        strcpy(fname, home);
        strcat(fname, "/.namazurc");
        if ((fp = fopen(fname, "rb"))) {
            strcpy(NAMAZU_CONF, fname);
            return fp;
        }
    }

    /* check the defalut */
    if ((fp = fopen(NAMAZU_CONF, "rb"))) {
        return fp;
    }
    return (FILE *) NULL;
}

int get_conf_arg(uchar *line, uchar *arg)
{
    *arg = '\0';
    if (*line != '"') {
	int n = strcspn(line, " \t");
	strncpy(arg, line, n);
	arg[n] = '\0';     /* Hey, don't forget this after strncpy()! */
	return n;
    } else {  /* double quoted argument */
	int n;
	line++;
	n = 1;
	do {
	    int nn = strcspn(line, "\"\\");
	    strncat(arg, line, nn);
	    n += nn;
	    line += nn;
	    arg[n] = '\0';
	    if (*line == '\0') {  /* terminator not matched */
		errmsg = "can't find string terminator";
		return 0;
	    }
	    if  (*line == '"') {  /* ending */
		n++;
		return n;
	    }
	    if (*line == '\\') {  /* escaping character */
		strncat(arg, line + 1, 1);
		n += 2;
		line += 2;
	    }
	} while (1);
	return n;
    }
}

void replace_home(uchar *str)
{
    uchar tmp[BUFSIZE];

    strcpy(tmp, str);
    if (strprefixcmp(tmp, "~/") == 0) {
	char *home;
	/* checke a home directory */
	if ((home = getenv("HOME")) != NULL) {
	    strcpy(tmp, home);
	    strcat(tmp, "/");
	    strcat(tmp, str + strlen("~/"));
	    strcpy(str, tmp);
	    return;
	}
    }

    if (strprefixcmp(tmp, "$HOME") == 0) {
	char *home;
	/* checke a home directory */
	if ((home = getenv("HOME")) != NULL) {
	    strcpy(tmp, home);
	    strcat(tmp, str + strlen("$HOME"));
	    strcpy(str, tmp);
	    return;
	}
    }
    return;
}


int get_conf_args(uchar *line, uchar *directive, uchar *arg1, uchar *arg2)
{
    int n;

    /* Skip white spaces in the beginning of this line */
    n = strspn(line, " \t");    /* skip white spaces */
    line += n;

    /* Determine whether or not this line is only a blank */
    if (*line == '\0') {
	strcpy(directive, "BLANK");
	return 0;
    }

    /* Determine whether or not this line is only a comment */
    if (*line == '#') {
	strcpy(directive, "COMMENT");
	return 0;
    }

    /* get a directive name */
    n = strspn(line, DIRECTIVE_CHARS);
    if (n == 0) {
	errmsg = "invalid directive name";
	return 0;
    }
    strncpy(directive, line, n);
    directive[n] = '\0';        /* Hey, don't forget this after strncpy()! */
    line += n;

    /* skip a delimiter after a directive */
    n = strspn(line, " \t");    /* skip white spaces */
    line += n;
    if (n == 0) {
	errmsg = "can't find a delimiter after directive";
	return 0;
    }

    /* get arg1 */
    n = get_conf_arg(line, arg1);
    line += n;

    if (n == 0) { /* cannot get arg1 */
	return 0;
    }

    /* replace $HOME or ~/ */
    replace_home(arg1);

    /* skip a delimiter after an arg1 */
    n = strspn(line, " \t");    /* skip white spaces */
    line += n;

    if (*line == '\0' || *line == '#') {  /* allow comment at the ending */
        /* this line has only one argument (arg1) */
	return 1;
    }

    /* get arg2 */
    n = get_conf_arg(line, arg2);
    line += n;

    if (n == 0) { /* cannot get arg2 */
	return 1;
    }

    /* replace $HOME or ~/ */
    replace_home(arg2);

    /* skip trailing white spaces after an arg2 */
    n = strspn(line, " \t");    /* skip white spaces */
    line += n;

    if (*line != '\0' || *line != '#') {  /* allow comment at the ending */
	return 2;
    } else {
	errmsg = "trailing garbages exist";
	return 0;
    }
}

void parse_conf(uchar *line, int lineno) {
    uchar directive[BUFSIZE] = "";
    uchar arg1[BUFSIZE] = "";
    uchar arg2[BUFSIZE] = "";
    int argnum = 0;

    argnum = get_conf_args(line, directive, arg1, arg2);
    if (errmsg != NULL) { 
	return; /* error */
    }

    if (Debug && 
	(strcasecmp(directive, "COMMENT") != 0) &&
	(strcasecmp(directive, "BLANK") != 0))
    {
	printf("%d: DIRECTIVE: [%s]\n", lineno, directive);
	if (argnum >= 1) {
	    printf("    ARG1: [%s]\n", arg1);
	}
	if (argnum == 2) {
	    printf("    ARG2: [%s]\n", arg2);
	}
	printf("\n");
    }

    if (check_argnum(directive, argnum) != 0) {
	return; /* error */
    }

    apply_conf(directive, arg1, arg2);
    return;
}

int check_argnum(uchar *directive, int argnum)
{
    struct conf_directive {
	uchar *name;
	int argnum;
    };
    struct conf_directive dtab[] = {
	{"BLANK", 0},
	{"COMMENT", 0},
	{"INDEX", 1},
	{"BASE", 1},
	{"REPLACE", 2},
	{"ALIAS", 2},
	{"LOGGING", 1},
	{"SCORING", 1},
	{"LANG", 1},
	{NULL, 0}
    };
    int i;
    for (i = 0; dtab[i].name != NULL; i++) {
	if (strcasecmp(dtab[i].name, directive) == 0) {
	    if (argnum == dtab[i].argnum) {
		return 0;  /* OK */
	    } else if (argnum < dtab[i].argnum) {
		errmsg = "too few arguments";
		return 1;  /* error */
	    } else if (argnum > dtab[i].argnum) {
		errmsg = "too many arguments";
		return 1;  /* error */
	    } else {
		die("check_argnum[1]: It MUST not be happend! ");
	    }
	}
    }
    die("check_argnum[2]: It MUST not be happend! ");
    return 1;
}

void apply_conf(uchar *directive, uchar *arg1, uchar *arg2)
{
    if (strcasecmp(directive, "COMMENT") == 0) {
	;  /* only a comment */
    } else if (strcasecmp(directive, "INDEX") == 0) {
	strcpy(DEFAULT_INDEX, arg1);
    } else if (strcasecmp(directive, "BASE") == 0) {
	strcpy(BASE_URI, arg1);
    } else if (strcasecmp(directive, "REPLACE") == 0) {
	Replace.src = add_list(Replace.src, arg1);
	Replace.dst = add_list(Replace.dst, arg2);
    } else if (strcasecmp(directive, "ALIAS") == 0) {
	Alias = add_alias(Alias, arg1, arg2);
    } else if (strcasecmp(directive, "LOGGING") == 0) {
	Logging = 0;
    } else if (strcasecmp(directive, "SCORING") == 0) {
	if (strcasecmp(arg1, "TFIDF") == 0) {
	    TfIdf = 1;
	} else if (strcasecmp(arg1, "SIMPLE") == 0) {
	    TfIdf = 0;
	}
    } else if (strcasecmp(directive, "LANG") == 0) {
	strcpy(Lang, arg1);
	init_message();
    }
}

/************************************************************
 *
 * Public functions
 *
 ************************************************************/

/* loading configuration file of Namazu */
void load_conf(char *av0)
{
    static int loaded = 0;
    FILE *fp;
    uchar buf[BUFSIZE];
    int lineno = 0;
    
    if (loaded) {
	/* free Replace and Alias */
    }

    /* I don't know why but GNU-Win32 require this */
    BASE_URI[0] = '\0';

    fp = open_conf_file(av0);
    if (fp == NULL)
	return;
    ConfLoaded = 1;
    while (fgets(buf, BUFSIZE, fp)) {
	lineno++;
	chomp(buf);
	codeconv(buf);  /* for Shift_JIS encoding */
	parse_conf(buf, lineno);
	if (errmsg != NULL) { /* error occurred */
	    die("%s:%d: syntax error: %s.\n",  
		NAMAZU_CONF, lineno, errmsg);
	}
    }
    fclose(fp);
    loaded = 1;
}

void show_conf(void)
{
    if (ConfLoaded) {
	printf("Config:  %s\n", NAMAZU_CONF);
    }

    printf("\
Default: %s\n\
BASE:    %s\n\
Logging: %s\n\
Lang:    %s\n\
Scoring: %s\n\
", DEFAULT_INDEX, BASE_URI, Logging ? "on" : "off",
           Lang, TfIdf ? "tfidf" : "simple");

    {
	ALIAS *list = Alias;

	while (list) {
	    printf("Alias:   \"%s\" -> \"%s\"\n", 
		   list->alias, list->real);
	    list = list->next;
	}
    }
    {
	REPLACE list = Replace;

	while (list.src) {
	    printf("Replace: \"%s\" -> \"%s\"\n", 
		   list.src->str, list.dst->str);
	    list.src = list.src->next;
	    list.dst = list.dst->next;
	}
    }

    exit(0);
}


