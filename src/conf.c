/*
 * 
 * conf.c -
 * 
 * $Id: conf.c,v 1.8 1999-09-01 07:17:42 satoru Exp $
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

/************************************************************
 *
 * Private functions
 *
 ************************************************************/

void set_pathname(char*, char*, char*);
FILE *open_conf_file(char*);
ALIAS *add_alias(ALIAS*, uchar*, uchar*);

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
    if ((home = getenv("HOME"))) {
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
    int n, i;
    
#ifdef NOCONF
    return;
#endif

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
	chomp(buf);
	if (!strncmp(buf, "INDEX\t", 6)) {
	    for (n = 6; buf[n] == '\t'; n++);
	    strcpy(DEFAULT_DIR, buf + n);
	} else if (!strncmp(buf, "BASE\t", 5)) {
	    for (n = 5; buf[n] == '\t'; n++);
	    strcpy(BASE_URI, buf + n);
	} else if (!strncmp(buf, "REPLACE\t", 8)) {
	    uchar tmp[BUFSIZE];
	    for (n = 8; buf[n] == '\t'; n++);
	    for (i = 0; buf[n] != '\t' && buf[n]; n++, i++) {
		tmp[i] = buf[n];
	    }
	    tmp[i] = '\0';
	    Replace.src = add_list(Replace.src, tmp);
	    for (; buf[n] == '\t'; n++);
	    Replace.dst = add_list(Replace.dst, buf + n);
	} else if (!strncmp(buf, "ALIAS\t", 6)) {
	    uchar tmp[BUFSIZE];
	    for (n = 6; buf[n] == '\t'; n++);
	    for (i = 0; buf[n] != '\t' && buf[n]; n++, i++) {
		tmp[i] = buf[n];
	    }
	    tmp[i] = '\0';
	    for (; buf[n] == '\t'; n++);
	    Alias = add_alias(Alias, tmp, buf + n);
	} else if (!strncmp(buf, "LOGGING\t", 8)) {
	    for (n = 8; buf[n] == '\t'; n++);
	    if (!strncmp(&buf[n], "OFF", 3))
		Logging = 0;
	} else if (!strncmp(buf, "SCORING\t", 8)) {
	    for (n = 8; buf[n] == '\t'; n++);
	    if (!strncmp(&buf[n], "TFIDF", 5))
		TfIdf = 1;
	    if (!strncmp(&buf[n], "SIMPLE", 6))
		TfIdf = 0;
	} else if (!strncmp(buf, "LANG\t", 5)) {
	    for (n = 5; buf[n] == '\t'; n++);
	    strncpy(Lang, &buf[n], 2);
            init_message();
	}
    }
    fclose(fp);
    loaded = 1;
}

void show_conf(void)
{
    if (ConfLoaded)
	printf("Config:  %s\n", NAMAZU_CONF);

    printf("\
Default: %s\n\
BASE:    %s\n\
Logging: %s\n\
Lang:    %s\n\
Scoring: %s\n\
", DEFAULT_DIR, BASE_URI, Logging ? "on" : "off",
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


