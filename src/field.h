#ifndef _FIELD_H
#define _FIELD_H

int is_field_safe_character(int);
int isfield(uchar *);
void apply_field_alias(uchar *);
void get_field_name(uchar *, uchar *);
void get_field_data(int, int, uchar *, uchar *);

#endif /* _FIELD_H */
