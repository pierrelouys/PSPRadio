/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * layout.h -- declarations for layout.c
 *
 * (C) 2001, 2002 antrik
 */
#ifndef __LAYOUT_H
#define __LAYOUT_H

#include "items.h"
#include "load.h"
#include "syntax.h"
#include "url.h"

struct Layout_data {    /* all data necessary for loading, layouting and rendering a HTML page */
   struct Resource	*input;    /* input resource descriptor */
   struct Url		*url;

   struct Element	*syntax_tree;    /* top of syntax parsing tree */
   struct Item		*item_tree;    /* top of structure (item) tree */
   struct Item_list	*page_map;    /* page usage map (one list of items for every line) */
   struct Link_list	*links;    /* list of all links */
   struct Anchor_list	*anchors;    /* list of all anchors */
};

struct Layout_data *layout(const struct Url *base_url, const char *url, const struct Item *form, int page_width, enum Syntax_error *syntax_err);    /* load and layout HTML page */
void resize(struct Layout_data *layout, int page_width);    /* adjust layout to new page width */
void free_layout(struct Layout_data *layout);    /* unallocate layouting data */

#endif
