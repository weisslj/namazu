/*
 * 
 * idxname.h - header files for Idx handling.
 *
 * $Id: idxname.h,v 1.2 1999-11-18 02:46:00 satoru Exp $
 * 
 * 
 */

#ifndef _IDXNAME_H
#define _IDXNAME_H

enum {
  ERR_INDEX_MAX = -1,
  ERR_MALLOC = -2
};

int add_index(char *idxname);
int getidxnum();

#endif /* _IDXNAME_H */
