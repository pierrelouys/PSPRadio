/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/

/* page.c -- page/history management
 *
 * (C) 2002, 2003 antrik
 *
 * This file contains all functions and data structures necessary for managing
 * the page to be displayed in the viewer as well as the page history.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "layout.h"
#include "links.h"
#include "page.h"

static void add_page(void);

struct Page_list	page_list={
   0,    /* count */
   -1,    /* pos */
   NULL    /* **page */
};

/* Create a new entry in page list, after the one of the currently active page;
 * set the new one as current. */
static void add_page(void)
{
   struct Page	*new_page;

   /* resize array to hold all pages */
   page_list.page=realloc(page_list.page, (page_list.count=page_list.pos+1)*sizeof(struct Page *));
   if(page_list.page==NULL) {
      fprintf(stderr, "memory allocation error while storing url to history\n");
      exit(1);
   }

   /* create new page descriptor */
   new_page=page_list.page[page_list.pos]=malloc(sizeof(struct Page));
   if(new_page==NULL) {
      fprintf(stderr, "memory allocation error while preparing page load\n");
      exit(1);
   }

   /* init pager */
   new_page->pager_pos=0;    /* beginning of page will be shown in pager after loading */
   new_page->cursor_x=new_page->cursor_y=0;
   new_page->sticky_cursor=0;
   new_page->active_link=-1;    /* no link highlighted */
   new_page->active_anchor=-1;    /* no anchor marked */

   new_page->mark=0;    /* new pages not marked */
}

/* free all memory used by page descriptors and by page list struct itself */
void free_page_list(void)
{
   int	del_page;

   DMSG(("  freeing memory used by page descriptors...\n"));
   for(del_page=0; del_page<page_list.count; ++del_page)
      free_page(page_list.page[del_page]);

   DMSG(("  freeing memory used by list...\n"));
   free(page_list.page);
}

/*
 * load a HTML page so it can be displayed
 *
 * Creates a descriptor for the page, and loads the requested document. The
 * entry number in the page list of the new page descriptor is returned.
 *
 * The URL of the page to be loaded is determined by combining the "url" with
 * fallback values from "base_url" (allowing relative links etc.); if
 * "base_url" is NULL, default values are used as fallback. If "url" is NULL,
 * "base_url" is taken as the final url. (For already prepared URLs from
 * history.)
 *
 * The optional "form" argument points to a form item in the structure tree,
 * describing a form to submit with the HTTP request.
 *
 * When called with the a "reference" page (meaning the new page is identical
 * to that one, and only a link to a local anchor was followed), the document
 * isn't reloaded, and the layouting passes aren't performed; only a new
 * descriptor is created, and the data structures are copied from the
 * reference.
 *
 * If no "url" is given, meaning that a page from the page list (history) is to
 * be reloaded, no new descriptor is created; the old one from the list is
 * reused. Only the document is reloaded/layouted. (Unless a "reference" is
 * given.)
 */

int load_page(base_url, url, form, reference, page_width, syntax_err)
const struct Url	*base_url;    /* fallback values */
const char		*url;    /* main URL */
const struct Item	*form;    /* form item of form to upload */
const struct Page	*reference;    /* old page; if not NULL, take layout data from it instead of reloading */
int			page_width;
enum Syntax_error	*syntax_err;    /* return: parse_syntax() found HTML syntax errors in page */
{
   struct Page		*page;    /* descriptor for new page */

   if(reference==NULL) {    /* new document (not already in memory) -> load */
      if(url!=NULL) {    /* new page (not from history) */
	 int	del_page;
	 int	last;

	 for(last=page_list.pos; last>=0 && page_list.page[last]->url->proto.type==PT_INTERNAL; --last);    /* find last non-internal page in history */
	 page_list.pos=last+1;    /* add new page after last */

	 /* free descriptors of all pages lost from history by loading some new page when not at end (after going back, or when overwriting internal pages) */
	 DMSG(("freeing memory used by page descriptors lost from history\n"));
	 for(del_page=page_list.pos; del_page<page_list.count; ++del_page)
	    free_page(page_list.page[del_page]);

	 add_page();
      } else {    /* from history */
	 int	del_page;
	 int	last;

	 for(last=page_list.count-1; last>=0 && page_list.page[last]->url->proto.type==PT_INTERNAL; --last);    /* find last non-internal page in history */

	 DMSG(("freeing memory used by page descriptors of internal page...\n"));
	 for(del_page=last+1; del_page<page_list.count; ++del_page)
	    free_page(page_list.page[del_page]);
	 page_list.count=last+1;
      }
      page=page_list.page[page_list.pos];

      page->layout=layout(base_url, url, form, page_width, syntax_err);
      if(url!=NULL)    /* new page -> need to store effective URL */
	 page->url=page->layout->url;

      if(url==NULL && page->active_link >= 0) {    /* page from history with some link previously active */
	 const int	links_count=page->layout->links->count;

	 struct Link	*link;

	 if(page->active_link >= links_count    /* no longer valid */
	    || (link=get_link(page->layout, page->active_link), page->cursor_x != link->x_start)
	    || page->cursor_y != link->y_start
	    || page->url_fingerprint != link->url_fingerprint
	    || page->text_fingerprint != link->text_fingerprint
	   ) {    /* link doesn't match the old one exactly -> look for best match */
	    int		candidate;
	    int		best;
	    float	best_deviation;

	    best=-1;
	    best_deviation=1.0;    /* accept anything better than (or equal to) this */
	    for(candidate=0; candidate < links_count; ++candidate) {    /* all links in page */
	       const struct Link	*link=get_link(page->layout, candidate);

	       int	url_dev=(page->url_fingerprint != link->url_fingerprint);
	       int	text_dev=(page->text_fingerprint != link->text_fingerprint);
	       int	x_dev=(page->cursor_x != link->x_start);    /* only test whether x mateches exactly */
	       float	y_dev=(float)abs(page->cursor_y - link->y_start)/page->layout->item_tree->y_end;    /* deviation relative to page length */
	       float	num_dev=(float)abs(page->active_link - candidate)/links_count;    /* deviation relative to link count */

	       float	deviation=0.5*url_dev+0.5*text_dev+1.5*num_dev+1.5*y_dev+0.1*x_dev;

	       if(deviation<=best_deviation) {    /* best one so far -> save */
		  best=candidate;
		  best_deviation=deviation;
	       }
	    }    /* for all links */
	    page->active_link=best;
	 }    /* search best matching link */
      }    /* from history, with some link active */
   } else {    /* document already in memory -> copy layouting data */
      if(reference->active_link>=0)    /* some link is active in old page */
	 highlight_link(reference->layout, reference->active_link, 1);    /* unhighlight (not active in new one) */
      
      page=page_list.page[page_list.pos];    /* assume page from history */
      if(url!=NULL) {    /* new page (not from history) -> create */
	 struct Url	*new_url;

	 new_url=merge_urls(base_url, url, NULL);
	 if(new_url->proto.type==PT_INTERNAL)    /* couldn't merge URLs */
	    return page_list.pos;    /* -> don't load anything */

	 ++page_list.pos;    /* add new after current page */
	 add_page();
	 page=page_list.page[page_list.pos];
	 page->url=new_url;
      }

      page->layout=reference->layout;
   }    /* reuse document */

   if(*page->url->frag && url!=NULL) {    /* fragment identifier specified (and not from history) -> go to anchor */
      int	anchor;

      /* find desired anchor */
      for(anchor=0; anchor < page->layout->anchors->count; ++anchor) {    /* all anchors in list */
	 const struct Item	*item=page->layout->anchors->anchor_item[anchor];    /* item of currently tested anchor */
	 const char		*id=item->type==ITEM_BLOCK_ANCHOR?item->data.block_anchor->id:item->data.inline_anchor->id;    /* name of current anchor */

	 if(strcasecmp(id, page->url->frag)==0)    /* found right one -> don't search further */
	    break;
      }
      if(anchor < page->layout->anchors->count)    /* found the anchor -> store */
	 page->active_anchor=anchor;
      else {
	 fprintf(stderr, "\nAnchor \"%s\" not found in page.\n", page->url->frag);
	 *syntax_err=1;
      }
   }    /* fragment identifier */

   return page_list.pos;
}

/*
 * unallocate page data
 *
 * Frees the split URL, and the page struct itself. (All other data should be
 * already cleared when calling this.)
 */
void free_page(page)
struct Page	*page;
{
   if(page->url!=NULL)
      free_url(page->url);

   DMSG(("   freeing page struct...\n"));
   free(page);
}
