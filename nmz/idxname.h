/*
 * 
 * idxname.h - header files for Idx handling.
 *
 * $Id: idxname.h,v 1.1 1999-11-08 05:06:05 knok Exp $
 * 
 * 
 */

#ifndef _IDXNAME_H
#define _IDXNAME_H

enum {
  ERR_INDEX_MAX = -1,
  ERR_MALLOC = -2
};

int add_index(uchar *idxname);
int getidxnum();

#endif /* _IDXNAME_H */
