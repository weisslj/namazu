#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "namazu.h"
#include "field.h"
#include "result.h"
#include "util.h"
#include "re.h"

/************************************************************
 *
 * Private functions
 *
 ************************************************************/

int is_field_safe_char(int);
void make_fullpathname_field(int);

int is_field_safe_char(int c)
{
    if ((strchr(FIELD_SAFE_CHARS, c) != NULL)) {
        return 1;
    } else {
        return 0;
    }

}

void make_fullpathname_field(int n)
{
    uchar *base;

    base = Idx.names[n];
    pathcat(base, NMZ.field);
}

/************************************************************
 *
 * Public functions
 *
 ************************************************************/


/* check a key if field or not */
int isfield(uchar *key)
{
    if (*key == '+') {
        key++;
    } else {
        return 0;
    }
    while (*key) {
        if (! is_field_safe_char(*key)) {
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
        if (! is_field_safe_char(*str)) {
            break;
        }
        *tmp = *str;
        tmp++;
        str++;
    }
    *tmp = '\0';

    apply_field_alias(field);
}

void get_field_data(int did, int fid, uchar *orig_field, uchar *data) 
{
    uchar fname[BUFSIZE];
    uchar *field = orig_field;
    int i;
    static int cache_idx = 0, cache_num = 0;
    FILE *fp_field, *fp_field_idx;
    struct field_cache {
	int did;
	int fid;
	uchar field[BUFSIZE];
	uchar data[BUFSIZE];
    };
    static struct field_cache fc[FIELD_CACHE_SIZE];

    apply_field_alias(field);  /* This would overwrite `field' */

    /* consult caches */
    for (i = 0; i < cache_num; i++) {
	if (Debug) {
	    printf("@@ field cache [%s] hit!\n", field);
	}
	if (did == fc[i].did && fid == fc[i].fid &&
	    strcmp(field, fc[i].field) == 0)
	{  /* cache hit! */
	    strcpy(data, fc[i].data);
	    return;
	}
    }

    /* make a pathname */
    make_fullpathname_field(did);
    strcpy(fname, NMZ.field);
    strcat(fname, field);
    
    fp_field = fopen(fname, "rb");
    if (fp_field == NULL) {
        fprintf(stderr, "%s: cannot open file.\n", fname);
    }

    strcat(fname, ".i");
    fp_field_idx = fopen(fname, "rb");
    if (fp_field_idx == NULL) {
        fprintf(stderr, "%s: cannot open file.\n", fname);
    }

    /* You can rely on that length of one field is shorter than 
       BUFSIZE (1024) because its length is restricted at 
       put_field_index() in mknmz.
     */
    fseek(fp_field, getidxptr(fp_field_idx, fid), 0);
    fgets(data, BUFSIZE, fp_field);
    chomp(data);

    if (strcmp(field, "uri") == 0) {
	replace_uri(data);
    } 

    fclose(fp_field);
    fclose(fp_field_idx);

    /* cache */
    fc[cache_idx].did = did;
    fc[cache_idx].fid = fid;
    strcpy(fc[cache_idx].field, field);
    strcpy(fc[cache_idx].data, data);
    cache_idx = (cache_idx + 1) % FIELD_CACHE_SIZE;
    if (cache_num < FIELD_CACHE_SIZE) {
	cache_num++;
    }
}

