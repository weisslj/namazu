#ifndef _NAMAZU_H
#define _NAMAZU_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "type.h"
#include "enum.h"
#include "message.h"
#include "variable.h"

#define is_lang_ja(a) (strcmp((a),"ja") == 0)
#define EMPHASIZING_START_ELEMENT "<strong class=\"keyword\">"
#define EMPHASIZING_END_ELEMENT   "</strong>"
 
#endif /* _NAMAZU_H */
