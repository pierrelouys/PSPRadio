/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * sgml.c -- modify syntax tree to cope with SGML
 *
 * (C) 2002 antrik
 *
 * The broken syntax trees created by missing end tags in SGML documents are
 * fixed retrospectively here.
 */
#ifndef XHTML_ONLY

#include "stdio.h"
#include "stdlib.h"

#include <syntax.h>

/* 
 * Fixes a broken syntax tree created by unclosed elements in SGML documents.
 *
 * Every element in the tree is checked against its ancestors to test whether
 * it should actually terminate the ancestor and be at same depth; if so, it's
 * lifted to that depth.
 */
void sgml_rework(tree_top)
struct Element	*tree_top;
{
   struct Element	*cur;    /* currently handled element in tree */
   struct Element	*prev;
   struct Element	*ancestor;    /* currently checked ancestor of "cur" */
   
   for(prev=tree_top, cur=tree_top->list_next; cur!=tree_top; prev=cur, cur=cur->list_next) {    /* all elements in tree (except top) */
      const enum Element_group	cur_group=element_table[cur->name.type].group;    /* element type group of "cur" */

      if(cur_group==GROUP_SINGLE)    /* single tag element -> just ensure it won't keep any children */
	 cur->closed=1;
      else if(cur_group!=GROUP_OBLIGATE) {    /* element with optional end tag -> test if needs to be lifted */
	 for(ancestor=cur->parent; ancestor!=tree_top; ancestor=ancestor->parent) {    /* all ancestors */
	    const enum Element_group	ancestor_group=element_table[ancestor->name.type].group;

	    if(ancestor_group==GROUP_OBLIGATE)    /* can't lift past elements with obligate end tag; stop scanning */
	       break;
	    if(ancestor_group==cur_group) {    /* ancestor should be terminated by current element -> lift current and following elements to it's level */
	       struct Element	*close;

	       /* close all elements in line between current element and the ancestor inclusive, so following elements won't stay inside the ancestor */
	       for(close=cur->parent; close!=ancestor->parent; close=close->parent) {
		  if(!close->closed) {    /* not already closed before */
		     if(cur->content!=NULL) {    /* lifted element has content -> need to create dummy element to save it */
			struct Element	*new;

			new=malloc(sizeof(struct Element));
			if(new==NULL) {
			   fprintf(stderr, "memory allocation error while fixing SGML\n");
			   exit(1);
			}

			new->name.type=EL_NO;
			new->attr_count=0;
			new->attr=NULL;

			/* take content of lifted element */
			new->content=cur->content;
			cur->content=NULL;

			/* insert into list (before "cur", inside closed element) */
			prev->list_next=new;
			new->list_next=cur;
			new->parent=close;
		     }    /* has content */

		     close->closed=1;
		  }    /* not already closed */
	       }    /* for all ancestors up to newly terminated one */

	       break;
	    }    /* lift */
	 }    /* for all ancestors */
      }    /* optional end tag */

      /* lift */
      while(cur->parent->closed)    /* parent is already closed -> lift element */
	 cur->parent=cur->parent->parent;
   }    /* all elements */
}

#endif    /* not XHTML_ONLY */
