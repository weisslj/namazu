/*
 * 
 * idxname.h - header files for Idx handling.
 *
 * $Id: idxname.h,v 1.3 1999-11-18 09:42:11 satoru Exp $
 * 
 * 
 */

#ifndef _IDXNAME_H
#define _IDXNAME_H

enum {
  ERR_INDEX_MAX = -1,
  ERR_MALLOC = -2
};

extern int add_index(char *idxname);
extern int getidxnum();

#endif /* _IDXNAME_H */
