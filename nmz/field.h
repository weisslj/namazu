#ifndef _FIELD_H
#define _FIELD_H

#define FIELD_SAFE_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"

enum {
    FIELD_CACHE_SIZE = 8        /* Size of field caches */
};

int is_field_safe_character(int);
int isfield(char *);
void apply_field_alias(char *);
void get_field_name(char *, char *);
void get_field_data(int, int, char *, char *);

#endif /* _FIELD_H */
