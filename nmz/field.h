#ifndef _FIELD_H
#define _FIELD_H

#define FIELD_SAFE_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"

enum {
    FIELD_CACHE_SIZE = 8        /* Size of field caches */
};

extern int is_field_safe_character(int);
extern int isfield(char *);
extern void apply_field_alias(char *);
extern void get_field_name(char *, char *);
extern void get_field_data(int, int, char *, char *);

#endif /* _FIELD_H */
