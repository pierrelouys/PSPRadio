/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/

/*
 * layout.c -- contains the framework for the layouting module.
 *
 * (C) 2001, 2002 antrik
 *
 * The document is loaded and all layouting passes are applied here except for
 * the final rendering, which is done in render() upon need.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cfg.h"
#include "debug.h"
#include "items.h"
#include "layout.h"
#include "links.h"
#include "load.h"
#include "render.h"
#include "syntax.h"
#include "url.h"

/*
 * load and layout a HTML document (prepare for rendering);
 *
 * Creates a descriptor for the layout data, calls layouting passes (syntax
 * parsing, element parsing, structure parsing, pre-rendering), prints debug
 * messages in between, and returns a structure with pointers to all layouting
 * data. (Loading the file is done by the syntax parser.)
 *
 * The URL of the page to be loaded is determined by combining the "url" with
 * fallback values from "base_url" (allowing relative links etc.); if
 * "base_url" is NULL, default values are used as fallback. If "url" is NULL,
 * "base_url" is taken as the final url. (For already prepared URLs from
 * history.)
 *
 * The optional "form" argument points to a form item in the structure tree,
 * describing a form to submit with the HTTP request.
 */

struct Layout_data *layout(base_url, url, form, page_width, syntax_err)
const struct Url	*base_url;    /* fallback values */
const char		*url;    /* main URL */
const struct Item	*form;    /* form item of form to upload */
int			page_width;
enum Syntax_error	*syntax_err;    /* return: parse_syntax() found HTML syntax errors in document */
{
   struct Layout_data	*layout;    /* descriptor of processed document */

   layout=malloc(sizeof(struct Layout_data));
   if(layout==NULL) {
      fprintf(stderr, "memory allocation error while loading document (in function layout())\n");
      exit(1);
   }

   layout->input=init_load(base_url, url, form);    /* open file/HTTP connection */

   DMSG(("parsing syntax...\n"));
   layout->syntax_tree=parse_syntax(layout->input, syntax_err);
   layout->url=layout->input->url;    /* save effective URL */
   uninit_load(layout->input);    /* close file, free memory; also detects errors in wget loading */

#ifdef DEBUG
   if(cfg.debug) {
      debug_printf("syntax tree:\n");
      dump_tree(layout->syntax_tree, 1, 0);    /* dump (including content, elements not parsed) */
   }
#endif

   DMSG(("parsing elements...\n"));
   parse_elements(layout->syntax_tree);

#ifndef XHTML_ONLY
   DMSG(("fixing SGML (missing end tags)...\n"));
   sgml_rework(layout->syntax_tree);
   #ifdef DEBUG
   if(cfg.debug) {
      debug_printf("fixed syntax tree:\n");
      dump_tree(layout->syntax_tree, 1, 1);    /* dump (including content, elements parsed) */
   }
   #endif
#endif

   DMSG(("parsing structure...\n"));
   layout->item_tree=parse_struct(layout->syntax_tree);
   DMSG(("freeing syntax tree...\n"));
   free_syntax(layout->syntax_tree, 1);    /* don't need syntax tree any longer */
   DMSG(("creating link list...\n"));
   layout->links=make_link_list(layout->item_tree);
   DMSG(("creating anchor list...\n"));
   layout->anchors=make_anchor_list(layout->item_tree);

   DMSG(("pre-rendering...\n"));
   layout->page_map=pre_render(layout->item_tree, page_width);
#ifdef DEBUG
   if(cfg.debug) {
      debug_printf("item tree:\n");
      dump_items(layout->item_tree);
   }
#endif

   return layout;
}

void resize(layout, page_width)
struct Layout_data	*layout;
int			page_width;
{
   DMSG(("freeing old page map...\n"));
   free_map(layout->item_tree, layout->page_map);

   DMSG(("pre-rendering...\n"));
   layout->page_map=pre_render(layout->item_tree, page_width);
}

/* 
 * unallocate a document's layouting data
 *
 * Frees item tree, page map, anchor and link lists, and the layout descriptor
 * itself. (The input resource descriptor and the syntax tree should already be
 * cleared when calling this.)
 */

void free_layout(layout)
struct Layout_data	*layout;
{
   DMSG(("   freeing page map...\n"));
   free_map(layout->item_tree, layout->page_map);
   DMSG(("   freeing item tree...\n"));
   free_items(layout->item_tree);
   DMSG(("   freeing link list...\n"));
   free_links(layout->links);
   DMSG(("   freeing anchor list...\n"));
   free_anchors(layout->anchors);

   DMSG(("   freeing descriptor...\n"));
   free(layout);
}
