#include <ctype.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "libnamazu.h"
#include "field.h"
#include "result.h"
#include "util.h"
#include "re.h"
#include "var.h"
#include "replace.h"
#include "idxname.h"

/*
 *
 * Private functions
 *
 */

static void apply_field_alias ( char *field );
static int is_field_safe_char ( int c );
static void make_fullpathname_field ( int n );

static void 
apply_field_alias(char *field)
{
    if (strcmp(field, "title") == 0) {
        strcpy(field, "subject");
    } else if (strcmp(field, "author") == 0) {
        strcpy(field, "from");
    } else if (strcmp(field, "path") == 0) {
        strcpy(field, "uri");
    } 
}

static int 
is_field_safe_char(int c)
{
    if ((strchr(FIELD_SAFE_CHARS, c) != NULL)) {
        return 1;
    } else {
        return 0;
    }

}

static void 
make_fullpathname_field(int n)
{
    char *base;

    base = get_idxname(n);
    nmz_pathcat(base, NMZ.field);
}

/*
 *
 * Public functions
 *
 */


/*
 * Check the key whether field or not.
 */
int 
isfield(const char *key)
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

/*
 * This function returns a string storing a field name in
 * the fieldpat. The string can only be used until the next
 * call to the function. 
 */
char *
get_field_name(const char *fieldpat)
{
    static char field_name[BUFSIZE]; /* storing field name */
    char *tmp = field_name;

    fieldpat++;  /* ignore beggining '+' mark */
    while (*fieldpat) {
        if (! is_field_safe_char(*fieldpat)) {
            break;
        }
        *tmp = *fieldpat;
        tmp++;
        fieldpat++;
    }
    *tmp = '\0';

    apply_field_alias(field_name);
    return field_name;
}

void 
get_field_data(int idxid, int docid, const char *field, char *data) 
{
    char fname[BUFSIZE];
    char tmpfield[BUFSIZE];
    int i;
    static int cache_idx = 0, cache_num = 0;
    FILE *fp_field, *fp_field_idx;
    struct field_cache {
	int idxid;
	int docid;
	char field[BUFSIZE];
	char data[BUFSIZE];
    };
    static struct field_cache fc[FIELD_CACHE_SIZE];

    strcpy(tmpfield, field);
    apply_field_alias(tmpfield);  /* This would overwrite `tmpfield' */

    /* 
     * Consult caches.
     * Caching is intended to reduce rereading same data from a disk drive.
     * It works well with this kind of format: <a href="${uri}">${uri}</a>.
     */
    for (i = 0; i < cache_num; i++) {
	if (idxid == fc[i].idxid && docid == fc[i].docid &&
	    strcmp(tmpfield, fc[i].field) == 0)
	{  /* cache hit! */
	    nmz_debug_printf("field cache [%s] hit!\n", tmpfield);
	    strcpy(data, fc[i].data);
	    return;
	}
    }

    /* Make a pathname */
    make_fullpathname_field(idxid);
    strcpy(fname, NMZ.field);
    strcat(fname, tmpfield);
    
    fp_field = fopen(fname, "rb");
    if (fp_field == NULL) {
        nmz_debug_printf("%s: cannot open file.\n", fname);
    }

    strcat(fname, ".i");
    fp_field_idx = fopen(fname, "rb");
    if (fp_field_idx == NULL) {
        nmz_debug_printf("%s: cannot open file.\n", fname);
    }

    /* 
     * You can rely on that length of a field is shorter than 
     * BUFSIZE [1024] because its length is restricted in 
     * conf.pl: $conf::MAX_FIELD_LENGTH = 200;
     */
    fseek(fp_field, nmz_getidxptr(fp_field_idx, docid), 0);
    fgets(data, BUFSIZE, fp_field);
    nmz_chomp(data);

    fclose(fp_field);
    fclose(fp_field_idx);

    /* Cache */
    fc[cache_idx].idxid = idxid;
    fc[cache_idx].docid = docid;
    strcpy(fc[cache_idx].field, tmpfield);
    strcpy(fc[cache_idx].data, data);
    cache_idx = (cache_idx + 1) % FIELD_CACHE_SIZE;
    if (cache_num < FIELD_CACHE_SIZE) {
	cache_num++;
    }
}


