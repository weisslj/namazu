#ifndef _FIELD_H
#define _FIELD_H

#include "type.h"

int is_field_safe_character(int);
int isfield(uchar *);
void apply_field_alias(uchar *);
void get_field_name(uchar *, uchar *);

#endif /* _FIELD_H */
