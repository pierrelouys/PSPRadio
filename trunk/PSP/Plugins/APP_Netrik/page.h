/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/

/*
 * page.h -- declarations for page.c
 *
 * (C) 2002, 2003 antrik
 *
 * Declares the Page and Page_list structures, and the loading and history
 * handling functions using these.
 */

#include "layout.h"

struct Page {    /* all data necessary for handling a page */
   struct Layout_data	*layout;    /* the document data necessary to display the page */

   struct Url		*url;    /* effective URL of page */

   int			pager_pos;    /* number of first line currently visible in pager */
   int			active_anchor;    /* nuber of anchor to go to when entering pager; reset after first keypress */
   int			active_link;    /* number of currently highlighted link */
   int			url_fingerprint, text_fingerprint;    /* hashes of active link URL and text (contents); necessary to recognize link if a revisited page changes */
   int			cursor_x, cursor_y;    /* cursor position on page (not screen!) */
   int			sticky_cursor;    /* flag: keep position when scrolling */

   int			mark;    /* page has return mark */
};

struct Page_list {
   int		count;    /* number of entries in history */
   int		pos;    /* position in history of current visible page */
   struct Page	**page;    /* array of pointers to Page structures */
};

extern struct Page_list	page_list;

int load_page(const struct Url *base_url, const char *url, const struct Item *form, const struct Page *reference, int page_width, enum Syntax_error *syntax_err);    /* load a new page to be displayed in the pager */
void free_page(struct Page *page);    /* unallocate page descriptor */
void free_page_list(void);    /* unallocate page history */
