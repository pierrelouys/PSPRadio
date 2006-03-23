/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * parse-elements.c -- this one looks up the textual tags and parameters in the
 * syntax tree and replaces them by enum numbers for further processing.
 *
 * (C) 2001 antrik
 *     2002 Patrice
 *
 * It also contains a debug function which dumps all tag and parameter types to
 * see if parse_tags recognized them correctly.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cfg.h"
#include "syntax.h"

/* look up the element and attriubte name strings to find out what kind they are */
void parse_elements(syntax_tree)
struct Element	*syntax_tree;    /* top of syntax parse tree */
{
   struct Element	*cur_el;    /* element currently processed */
   int			i;
   int			cur_attr;    /* attribute currently processed */

#ifdef DEBUG
   if(strcmp(element_table[EL_NO].name, "?")) {    /* number of entries in "Element_type" enum doesn't match number of entries in "element_table[]" */
      fprintf(stderr, "internal error: \"element_table[]\" broken\n");
      exit(100);
   } else if(strcmp(attr_table[ATTR_NO].name, "?")) {
      fprintf(stderr, "internal error: \"attr_table[]\" broken\n");
      exit(100);
   }
#endif

   syntax_tree->name.type=EL_GLOBAL;    /* first element is always global element */

   for(cur_el=syntax_tree->list_next; cur_el->parent!=NULL; cur_el=cur_el->list_next) {    /* all elements in syntax parse tree */
      if(cur_el->name.str!=NULL) {    /* element has a name string */
	 /* search this element's name in table */
	 for(i=0; i<EL_NO; ++i)
	    if(!strcmp(cur_el->name.str, element_table[i].name))    /* found -> don't search further */
	       break;

	 if(cfg.warn_unknown) {
	    if(i==EL_NO)    /* unknown element */
	       fprintf(stderr, "warning: ignoring unknown element: %s\n", cur_el->name.str);
	 }

	 free(cur_el->name.str);
	 cur_el->name.type=i;    /* set type (==EL_NO if not found in table) */

	 for(cur_attr=0; cur_attr<cur_el->attr_count; ++cur_attr) {    /* all attributes */
	    /* search this attributes's name in table */
	    for(i=0; i<ATTR_NO; ++i)
	       if(!strcmp(cur_el->attr[cur_attr].name.str, attr_table[i].name))    /* found -> don't search further */
		  break;

	    if(cfg.warn_unknown) {
	       if(i==ATTR_NO)    /* unknown attribute */
		  fprintf(stderr, "warning: ignoring unknown attribute: %s\n", cur_el->attr[cur_attr].name.str);
	    }

	    free(cur_el->attr[cur_attr].name.str);
	    cur_el->attr[cur_attr].name.type=i;    /* set type (==ATTR_NO if not found in table) */

	    /* avoid NULL values */
	    if(cur_el->attr[cur_attr].value.str==NULL) {
	       cur_el->attr[cur_attr].value.str=strdup("");
	       if(cur_el->attr[cur_attr].value.str==NULL) {
		  fprintf(stderr, "memory allocation error while parsing elements (in function parse_elements)\n");
		  exit(1);
	       }
	    }

/* Patrice --> */
            /* convert to a number if there is a numeric value expected */
            if(attr_table[i].numeric) {
               char	*tmpval=strdup(cur_el->attr[cur_attr].value.str);
               free(cur_el->attr[cur_attr].value.str);
               cur_el->attr[cur_attr].value.num=atoi(tmpval);
               free(tmpval);
            }
	 }    /* for all attributes */

         /* check if all mandatory attributes are set */
	 if(cur_el->name.type<EL_NO) {    /* normal element (not EL_NO or EL_GLOBAL) -> check (avoids addign all non-mandatory attributes to dummy elements!) */
	    for(i=0; i<ATTR_NO; ++i) {
	       if(attr_table[i].def_val!=0 && attr_table[i].el==cur_el->name.type) { 
		  for(cur_attr=0; cur_attr<cur_el->attr_count; ++cur_attr)
		     if((int)cur_el->attr[cur_attr].name.type==i)    /* found -> don't search further */
			break;
		  if(cur_attr==cur_el->attr_count) {	/* need to add attribute */
		     cur_el->attr=realloc(cur_el->attr, (++cur_el->attr_count)*sizeof(struct Attr));    /* resize attribute array to hold new attribute */
		     if(cur_el->attr==NULL) {
			fprintf(stderr, "memory allocation error while parsing elements (in function parse_elements)\n");
		     }
		     cur_el->attr[cur_el->attr_count-1].name.type=i;
		     if(attr_table[i].numeric)
			cur_el->attr[cur_el->attr_count-1].value.num=atoi(attr_table[i].def_val);
		     else
			cur_el->attr[cur_el->attr_count-1].value.str=strdup(attr_table[i].def_val);
		  }
	       }
	    }
	 }    /* normal element */
/* <-- Patrice */

      } else    /* no name string => dummy element */
	 cur_el->name.type=EL_NO;
   }    /* for all elements */
}
