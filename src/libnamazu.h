/*
 * 
 * libnamazu.h - Namazu library api
 *
 * $Id: libnamazu.h,v 1.1 1999-10-12 07:28:11 knok Exp $
 * 
 */

#ifdef _LIBNAMAZU_H
#define _LIBNAMAZU_H

int set_redirect_stdout_to_file(uchar * fname);
void free_idxnames(void);
void free_aliases(void);
void free_replaces(void);
void make_fullpathname_msg(void);
void codeconv_query(uchar *query);
void getenv_namazuconf(void);
void uniq_idxnames(void);
int expand_idxname_aliases(void);
int complete_idxnames(void);

#endif _LIBNAMAZU_H
