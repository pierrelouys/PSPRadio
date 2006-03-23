/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * pager.h -- declarations/prototypes for pager.c.
 *
 * (C) 2001, 2002 antrik
 */

#include "layout.h"

/* return value of display() */
enum Pager_ret {
   RET_NO,    /* don't return (internal use) */
   RET_QUIT,
   RET_COMMAND,    /* enter command prompt */
   RET_SEARCH,
   RET_LINK,    /* follow a link; page->active_link tells which one */
   RET_LINK_URL,    /* display active link URL */
   RET_URL,    /* display current page URL */
   RET_ABSOLUTE_URL,    /* display absolute (merged) target URL of active link */
   RET_HISTORY,    /* reload a page from history; url_hist.pos tells which one */
   RET_WINCH    /* resize layout after SIGWINCH */
};
   
enum Pager_ret display(struct Page *page);    /* interactive pager */
