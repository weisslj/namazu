#ifndef _PARSER_H
#define _PARSER_H

/* definitions of operator */
#define AND_STRING "&"
#define OR_STRING  "|"
#define NOT_STRING "!"
#define LP_STRING  "("
#define RP_STRING  ")"

/* also acceptable as word since v1.1.1 */
#define AND_STRING_ALT "and"
#define OR_STRING_ALT  "or"
#define NOT_STRING_ALT "not"

#define AND_OP 1
#define NOT_OP 2

void init_parser(void);
HLIST expr(void);

#endif /* _PARSER_H */
