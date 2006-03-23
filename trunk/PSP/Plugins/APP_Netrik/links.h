/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * links.h -- declares the link-handling functions in links.c
 *
 * (C) 2002 antrik
 *
 */
#include "items.h"    /* for item tree structure */
#include "layout.h"    /* for page structure */

struct Link *get_link(const struct Layout_data *layout, int link_num);    /* extract link from page struct */
void highlight_link(const struct Layout_data *layout, int change_link, int off);    /* highlight a link in the item tree */
