/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * links.c -- some functions for handling hyperlink-related stuff
 *
 * (C) 2002 antrik
 *
 */
#include <curses.h>
#include <stdlib.h>

#include "colors.h"
#include "items.h"    /* for item tree structure */
#include "layout.h"    /* for layout structure */

#include "links.h"

/* extract the link_num'th link's Link structure from the item
 * tree and return a pointer to it */
struct Link *get_link(layout, link_num)
const struct Layout_data	*layout;
int				link_num;    /* which link */
{
   const struct Link_ptr	*list_entry=&layout->links->link[link_num];    /* entry in link list pointing to link */
   return &list_entry->item->data.string->link[list_entry->num];    /* link's data */
}

/* highlight the active link in the item tree */
void highlight_link(layout, change_link, off)
const struct Layout_data	*layout;
int				change_link;    /* link to highlight */
int				off;    /* unhighlight */
{
   const struct Link_ptr	*list_entry=&layout->links->link[change_link];    /* entry in link list pointing to link */
   const struct String		*string=list_entry->item->data.string;    /* string containing link */
   const struct Link		*link=&string->link[list_entry->num];    /* link data */

   int	cur_div;

   /* find div containing beginning of link */
   for(cur_div=0; string->div[cur_div].end<=link->start; ++cur_div);
#ifdef DEBUG
   if(cur_div>=string->div_count) {
      endwin();
      fprintf(stderr, "internal error: trying to work behind string end (in function activate_link())\n");
      exit(100);
   }
#endif

   for(; cur_div<string->div_count && string->div[cur_div].end<=link->end; ++cur_div) {    /* all divs containing parts of link */
      if(!off)    /* activate */
	 string->div[cur_div].color|=color_map[TM_ACTIVE];    /* set active link color */
      else    /* deactivate */
	 string->div[cur_div].color&=~color_map[TM_ACTIVE];    /* clear active link color */
   }
}
