%{
#include "namazu.h"
#include <string.h>
	void get_field_data(char *);
	static int Fid;
	static uchar Buf[BUFSIZE * 16];
%}

%%
\$\{[-_a-zA-Z]+\} 	       get_field_data(yytext);
.|\n                           strcat(Buf, yytext);
%%

void make_fullpathname_flist(int n)
{
    uchar *base;

    base = DbNames[n];

    pathcat(base, FLIST);
    pathcat(base, FLISTINDEX);
    pathcat(base, FIELDINFO);
    pathcat(base, RESULTFILE);
}

void encode_entity(uchar *str)
{
    int i;
    uchar tmp[BUFSIZE];

    strcpy(tmp, str);
    strcpy(str, "");
    for (i = 0; tmp[i]; i++) {
	if (tmp[i] == '<') {
	    strcat(str, "&lt;");
	} else if (tmp[i] == '>') {
	    strcat(str, "&gt;");
	} else if (tmp[i] == '&') {
	    strcat(str, "&amp;"); 
	} else if (tmp[i] == '"') {
	    strcat(str, "&quot;");
	} else {
	    strncat(str, tmp + i, 1);
	}
    }
}

void get_field_data(char *key) 
{
    uchar field[BUFSIZE], file_name[BUFSIZE];
    FILE *fp_field, *fp_field_idx;

    /* 6 is length of '&quot;'. It's a consideration for buffer overflow */
    uchar buf[BUFSIZE * 6];  

    strcpy(field, key + 2);
    *(field + strlen(field) - 1) = '\0';
    apply_field_alias(field);

    strcpy(file_name, FIELDINFO); /* make pathname */
    strcat(file_name, field);
    
    fp_field = fopen(file_name, "rb");
    if (fp_field == NULL) {
        fprintf(stderr, "%s: cannot open file.\n", file_name);
    }

    strcat(file_name, ".i");
    fp_field_idx = fopen(file_name, "rb");
    if (fp_field_idx == NULL) {
        fprintf(stderr, "%s: cannot open file.\n", file_name);
    }

    /* You can rely on that length of one field is shorter than 
       BUFSIZE (1024) because its length is restricted at 
       put_field_index() in mknmz.
     */
    fseek(fp_field, get_index_pointer(fp_field_idx, Fid), 0);
    fgets(buf, BUFSIZE, fp_field);

    if (!strcmp(field, "uri")) {
	replace_uri(buf);
    }
    *(buf + strlen(buf) - 1) = '\0';
    encode_entity(buf);
    strcat(Buf, buf);
}

/* display the hlist */
void put_hlist(HLIST hlist)
{
    int i;

    if (hlist.n <= 0 || HListMax == 0)
	return;

    for (i = HListWhence; i < hlist.n; i++) {
	strcpy(Buf, "\t");  /* '\t' has an important role cf. fputx() */
	Fid = hlist.fid[i];
	if (!AllList && (i >= HListWhence + HListMax))
	    break;
	if ((yyin = fopen(RESULTFILE, "r")) == NULL) {
		error("put_hlist");
		exit(-1);
	}
	yylex();
	fputx(Buf, stdout);
	fputx("\n", stdout);
    }
}

