/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * render.h -- contains extern function prototypes for render.c.
 *
 * (C) 2001, 2002 antrik
 */

#include "config.h"

#include "items.h"

#ifdef DEBUG
void dump_items(struct Item *item_tree);    /* dump items+text */
#endif
void dump(struct Layout_data *layout);    /* dump whole page to screen, with correct layout */
void render(struct Layout_data *layout, int page_x, int page_y, int screen_x, int screen_y, int width, int height, int overpaint);    /* render specified region of page */
