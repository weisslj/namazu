#include <ctype.h>
#include "namazu.h"
#include "field.h"

int is_field_safe_character(int c)
{
    if ((isalnum(c) || c == (int)'-')) {
        return 1;
    } else {
        return 0;
    }

}

/* check a key if field or not */
int isfield(uchar *key)
{
    if (*key == '+') {
        key++;
    } else {
        return 0;
    }
    while (*key) {
        if (! is_field_safe_character(*key)) {
            break;
        }
        key++;
    }
    if (isalpha(*(key - 1)) && *key == ':' ) {
        return 1;
    }
    return 0;
}

void apply_field_alias(uchar *field)
{
    if (strcmp(field, "title") == 0) {
        strcpy(field, "subject");
    } else if (strcmp(field, "author") == 0) {
        strcpy(field, "from");
    } else if (strcmp(field, "path") == 0) {
        strcpy(field, "uri");
    } 
}

void get_field_name(uchar *field, uchar *str)
{
    uchar *tmp = field;

    str++;  /* ignore beggining '+' mark */
    while (*str) {
        if (! is_field_safe_character(*str)) {
            break;
        }
        *tmp = *str;
        tmp++;
        str++;
    }
    *tmp = '\0';

    apply_field_alias(field);
}


