/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * pager.c -- Interactive viewing module.
 *
 * (C) 2001, 2002, 2003 antrik
 *
 * Displays output page and handles user commands.
 */
#include <curses.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "colors.h"
#include "cfg.h"
#include "layout.h"
#include "links.h"
#include "page.h"
#include "pager.h"
#include "render.h"
#include "screen.h"
#include "search.h"
#include "winch.h"

enum Match_type {
   FIND_FIRST,
   FIND_LAST
};

static int active_start(const struct Page *page, int pos);    /* find first active line */
static int active_end(const struct Page *page, int pos);    /* find last active line */

static void set_cursor(struct Page *page, int x, int y);    /* set new cursor position */

static int find_link(const struct Page *page, int start_x, int start_y, int end_x, int end_y, int end_line, enum Match_type match_type);    /* find link matching criteria */

static void activate_link(struct Page *page, int change_link);    /* set active link */
static void scroll_to(struct Page *page, int new_line);    /* adjust visible part of page */
static void activate_anchor(struct Page *page);    /* show active anchor */

static int remap(int key);    /* remap keys */

#define PAGE_INVALID ((unsigned int)-1>>1)    /* biggest possible positive int */
static int	old_line;    /* page position visible up to now */

/* return first line (page coordinates) where links can be activated with the
 * current "pager_pos" plus the relative "pos" */
static int active_start(page, pos)
const struct Page	*page;
int			pos;
{
   const int	screen_start=page->pager_pos+pos;

   if(screen_start > 0)
      return screen_start+cfg.link_margin;
   else
      return 0;
}

/* return pos *after* last line (page coordinates) where links can be
 * activated with the current "pager_pos" plus the relative "pos" */
static int active_end(page, pos)
const struct Page	*page;
int			pos;
{
   const int	page_end=page->layout->item_tree->y_end;
   const int	screen_end=page->pager_pos+pos+LINES;

   if(screen_end < page_end)
      return screen_end-cfg.link_margin;
   else
      return page_end;
}

/*
 * Set cursor to a specified page position. (Not screen position!)
 *
 * The page is scrolled if necessary.
 *
 * If x==y==-1, go home instead. (Go to first line in active screen area, and
 * reset sticky status.)
 *
 * If the cursor moves off a previously active link, the link is deactivated;
 * if it moves onto a new one, it is activated.
 */

static void set_cursor(page, x, y)
struct Page	*page;
int		x, y;
{
   int	home;
   int	new_link;

   if(x==-1 && y==-1) {    /* The cursor is to be homed, not to be set to a specific position */
      home=1;
      x=0;
      y=active_start(page, 0);
   } else {    /* don't home */
      home=0;

      /* scroll if necessary to bring cursor into active screen area */
      if(y < active_start(page, 0))    /* before active screen area */
	 scroll_to(page, page->pager_pos - (active_start(page, 0)-y));    /* -> scroll backward as many lines as cursor is out */
      else if(y >= active_end(page, 0))    /* behind active screen area */
	 scroll_to(page, page->pager_pos + (y - (active_end(page, 0)-1)));    /* -> scroll forward */

      /* don't set outside screen */
      if(x<0)
	 x=0;
      else if(x>=COLS)
	 x=COLS-1;

      if(y >= page->layout->item_tree->y_end)
	 y=page->layout->item_tree->y_end-1;
      if(y<0)    /* undoes previous condition if page contains no lines! */
	 y=0;
   }    /* don't home */

   /* activate link under cursor and/or deactivate old link */
   new_link=find_link(page, -1, active_start(page, 0), x+1, y, active_end(page,0), FIND_LAST);    /* find last link starting before or at cursor (and fitting on screen) */
   if(new_link>=0) {
      const struct Link	*link=get_link(page->layout, new_link);

      if(!(link->y_end > y+1 || (link->y_end > y && link->x_end > x)))    /* link doesn't end behind cursor -> don't activate */
	 new_link=-1;
   }
   if(new_link != page->active_link)    /* active link changed */
      activate_link(page, new_link);

   page->cursor_x=x;
   page->cursor_y=y;
   page->sticky_cursor=!home;
}

/* 
 * Find first or last link inside some specified range (part of page).
 *
 * The link has to *start* inside the range ("start_x","start_y") -
 * ("end_x","end_y"). (The end coordinates describing the first invalid
 * position.) Additionally, it has to *end* before "end_line".
 *
 * Setting any of those paramteres to -1 means there is no bound for that.
 * (Note that if "end_x" is -1, only links will be accepted that end *before*
 * "end_y" -- which is logical, though it doesn't exactly comply to the
 * previous rule.)
 *
 * If "match_type" is FIND_FIRST, the first link inside the desided range
 * (fulfillig all mentioned conditions) is returned; otherwise, the last one.
 */

static int find_link(page, start_x, start_y, end_x, end_y, end_line, match_type)
const struct Page	*page;    /* page to search */
int			start_x, start_y;    /* beginning of range for link start */
int			end_x, end_y;    /* end of range for link start */
int			end_line;    /* end of range for link end */
enum Match_type		match_type;    /* take first match or last match (list direction) */
{

#define INSIDE_START() (link->y_start > start_y || (link->y_start==start_y && link->x_start >= start_x))    /* link starts after start of range */
#define INSIDE_END() ( \
   ((unsigned)link->y_start < (unsigned)end_y || (link->y_start==end_y && link->x_start < end_x))    /* starts before end of link start range */ \
   && (unsigned)link->y_end <= (unsigned)end_line    /* ends before end of link end range */ \
)

   const int	link_count=page->layout->links->count;

   int		first, last, mid;

   int		candidate;
   struct Link	*link;

   /* binary search for first/last link inside major bound (first link in range when FIND_FIRST, last when FIND_LAST) */
   for(first=0, last=link_count-1; last>=first; ) {
      mid=(first+last)/2;
      link=get_link(page->layout, mid);
      if(match_type==FIND_FIRST ? INSIDE_START() : !INSIDE_END())
	 last=mid-1;
      else
	 first=mid+1;
   }
   candidate= match_type==FIND_FIRST ? first : last;

   /* check whether we have a link fulfilling all conditions */
   if((unsigned)candidate < (unsigned)link_count) {    /* link is valid (i.e. anything found in previous search) */
      link=get_link(page->layout, candidate);
      if(match_type==FIND_FIRST ? INSIDE_END() : INSIDE_START())    /* also inside other bound => have a result */
	 return candidate;
   }
   return -1;    /* no valid link in range */
}

/*
 * set active link
 *
 * Also highlights the new link and redraws the affected screen area.
 *
 * If some link was alredy active, it is first deactivated before activating
 * the new one.
 *
 * If change_link is -1, deactivate link (no link is active afterwards).
 */

static void activate_link(page, change_link)
struct Page	*page;
int		change_link;
{
   const int			link_num=change_link>=0?change_link:page->active_link;    /* link to activate or deactivate (normally, change (activate) given link; if -1, change presently active one (deactivate)) */

   if(link_num>=0) {    /* actually have work to do (not trying to deactivate when there was no link active anyways) */
      const struct Link_ptr	*list_entry=&page->layout->links->link[link_num];    /* entry in link list pointing to link */
      const struct Item		*item=list_entry->item;    /* item containing link */
      const struct String	*string=item->data.string;    /* string containing link */
      const struct Link		*link=&string->link[list_entry->num];    /* link data */

      int	x_start, y_start, x_end, y_end;    /* screen area that needs to be redrawn (affected by link) */
      int	page_x, page_y;    /* starting position of redrawn area from page */

      /* scroll page so link will be on screen */
      if(change_link>=0) {    /* activating link, not deactivating (deactivating a link not on screen is OK) */
	 const int	first_line=active_start(page, 0);    /* first line in which links may be activated */
	 const int	last_line=active_end(page, 0);    /* (after) last line where links may be activated */

	 int	scroll_lines=0;

	 if(link->y_end > last_line)    /* link ends behind active screen area */
	    scroll_lines=link->y_end-last_line;    /* -> scroll as many lines as necessary so it fits */
	 if(link->y_start < first_line)    /* starts before active area */
	    scroll_lines=link->y_start-first_line;    /* -> scroll backwards (negative scroll_lines) NOTE: this may undo previous condition, if link spans more lines than active area... */

	 if(scroll_lines) {    /* have to scroll */
	    if(abs(scroll_lines)<LINES)    /* need to scroll less then screenful */
	       scroll_to(page, page->pager_pos+scroll_lines);    /* -> scroll just as far as necessary so link fits */
	    else    /* scroll very far */
	       scroll_to(page, link->y_start-cfg.link_margin);    /* -> scroll so that link will appear just at the beginning of active screen area (this is identical to scrolling just as far as necessary when scrolling back, but different when scrolling forward) */
	 }
      }    /* scroll */

      if(change_link>=0)    /* activating link, not deactivating */
	 if(page->active_link>=0)    /* some link is already active */
	    activate_link(page, -1);    /* -> first deactivate the old one */

      highlight_link(page->layout, link_num, change_link>=0?0:1);    /* highlight the link, or unhighlight if deactivating */

      /* coordinates of updated area */
      if(link->y_end-link->y_start==1) {    /* link is on one line -> only redraw the affected line part */
	 page_x=link->x_start;
	 x_start=link->x_start;    /* no x-scrolling yet... */
	 x_end=link->x_end;
      } else {    /* link spans multiple lines -> completely redraw all lines that contain some part of the link */
	 /* full width of item (whole lines) */
	 page_x=item->x_start;
	 x_start=item->x_start;
	 x_end=item->x_end;
      }
      page_y=link->y_start;
      y_start=link->y_start-page->pager_pos;    /* screen coordinates of link depend on pager position */
      y_end=link->y_end-page->pager_pos;

      if(x_start<COLS && x_end>0 && y_start<LINES && y_end>0) {    /* some part visible */
	 /* truncate to visible area */
	 if(x_start<0) {
	    page_x-=x_start;
	    x_start=0;
	 }
	 if(y_start<0) {
	    page_y-=y_start;
	    y_start=0;
	 }
	 if(x_end>COLS)
	    x_end=COLS;
	 if(y_end>LINES)
	    y_end=LINES;

	 render(page->layout, page_x, page_y, x_start, y_start, x_end-x_start, y_end-y_start, 0);    /* update area affected by link */
      }

      if(change_link>=0) {    /* activate link */
	 page->active_link=change_link;
	 set_cursor(page, link->x_start, link->y_start);
	 /* also store hashes */
	 page->url_fingerprint=link->url_fingerprint;
	 page->text_fingerprint=link->text_fingerprint;
      } else    /* deactivate link */
	 page->active_link=change_link;    /* store -1 */
   }    /* anything to do */
}

/* 
 * move visible page area so "new_line" will be first line visible on screen
 *
 * If this isn't possible because of page boundaries, scroll as far as
 * possible.
 */

static void scroll_to(page, new_line)
struct Page	*page;
int		new_line;    /* page position to move to */
{
   int		scroll_lines;    /* number of lines to scroll */

   /* page boundaries */
   if(new_line>page->layout->item_tree->y_end-LINES)    /* page ends before end of screen -> adjust */
      new_line=page->layout->item_tree->y_end-LINES;
   if(new_line<0)    /* page begins after start of screen -> adjust (may undo previous condition, if page length < screen length!) */
      new_line=0;

   scroll_lines=new_line-old_line;

   if(abs(scroll_lines)<=LINES) {    /* move not more than a screenfull -> scroll */
      move(0,0);
      if(insdelln(-scroll_lines)==ERR) {
	 endwin();
	 fprintf(stderr, "curses error: can't scroll (in function scroll_to)\n");
	 exit(2);
      }
      
      if(scroll_lines<0)    /* scroll back */
	 render(page->layout, 0, new_line, 0, 0, COLS, -scroll_lines, 0);    /* render new first line(s) */
      else    /* scroll forward */
	 render(page->layout, 0, new_line+LINES-scroll_lines, 0, LINES-scroll_lines, COLS, scroll_lines, 0);    /* render new last line(s) */

   } else {    /* move more than screenful (or PAGE_INVALID) -> repaint */
      erase();
      render(page->layout, 0, new_line, 0, 0, COLS, LINES, 0);    /* fill whole screen */
   }
   
   old_line=new_line;    /* save new position */
   page->pager_pos=new_line;

   if(page->active_link>=0) {    /* some link is active -> may have to deactivate */
      const struct Link	*link=get_link(page->layout, page->active_link);    /* current link's data */

      if(link->y_start < active_start(page, 0) || link->y_end > active_end(page, 0))    /* moved out of legal area for active links */
	 activate_link(page, -1);    /* -> deactivate */
   }

   if(
      !page->sticky_cursor
      || page->cursor_y < active_start(page, 0) || page->cursor_y >= active_end(page, 0)    /* cursor is no longer in active screen area */
   )    /* don't keep cursor position */
      set_cursor(page, -1, -1);    /* -> home */
}

/*
 * Show active anchor.
 *
 * Scroll so that the anchor will be in the desired screen line (determined by
 * "cfg.anchor_offset"); draw marks at all screen lines spanned by the anchor.
 * When called while no "active_anchor" set, clear anchor marks.
 *
 * First the anchor position is retrieved and the page scrolled to the optimal
 * position. Afterwards, the real anchor position on the screen is calculated,
 * and the marks set directly to the screen. (Not added to "item_tree".)
 * Clearing is done by rerendering the affected area.
 */

static void activate_anchor(page)
struct Page	*page;
{
   const int	size=strlen(cfg.anchor_mark);

   static int	anchor;    /* affected anchor */
   int		start_line;    /* screen line containing anchor start */
   int		end_line;    /* screen lines after anchor end */

   const struct Item	*anchor_item;

   if(page->active_anchor>=0)    /* there is an anchor to activate -> take it (otherwise use the one stored before) */
      anchor=page->active_anchor;

   anchor_item=page->layout->anchors->anchor_item[anchor];

   /* get screen position of anchor */
   {
      if(page->active_anchor>=0)    /* activating anchor -> first need to scroll */
	 scroll_to(page, anchor_item->y_start-(cfg.anchor_offset?LINES/cfg.anchor_offset:cfg.link_margin));    /* scroll so anchor will appear "1/anchor_offset" below screen top, or "link_margin" below screen top if no "anchor_offset" */

      start_line=anchor_item->y_start - page->pager_pos;
      end_line=anchor_item->y_end - page->pager_pos;

      if(end_line==start_line)    /* empty anchor -> mark one line */
	 ++end_line;

      if(start_line<0)    /* starts before screen top -> truncate */
	 start_line=0;

      if(end_line>LINES)
	 end_line=LINES;
   }

   if(page->active_anchor>=0) {    /* activate -> draw marks */
      int	line;

#ifdef DEBUG
      if(start_line>=LINES || end_line<=0) {
	 endwin();
	 fprintf(stderr, "internal error: trying to activate anchor not on screen\n");
	 exit(100);
      }
#endif

      for(line=start_line; line<end_line; ++line) {
	 set_color(color_map[TM_SYS]);
	 mvprintw(line, COLS-size, "%s", cfg.anchor_mark);
      }

      set_cursor(page, anchor_item->x_start, anchor_item->y_start);
   } else    /* deactivate -> rerender area containing marks */
      if(end_line>start_line)    /* anything to render (not scrolled out of screen) */
	 render(page->layout, COLS-size, start_line+page->pager_pos, COLS-size, start_line, size, end_line-start_line, 1);    /* overpaint */
}

/* remap arrow keys to cursor movement keys if cfg.cursor_keys */
static int remap(key)
int	key;
{
   static const struct Map_entry {
      int	from;    /* key to be remapped */
      int	to;    /* replace with */
   } map[]={
      {KEY_LEFT, 'H'-'@'},
      {KEY_DOWN, 'J'-'@'},
      {KEY_UP, 'K'-'@'},
      {KEY_RIGHT, 'L'-'@'}
   };

   unsigned int	entry;

   if(cfg.cursor_keys)    /* want remapping */
      for(entry=0; entry < sizeof(map)/sizeof(struct Map_entry); ++entry)    /* all entries in "map[]" */
	 if(map[entry].from==key)    /* found a mapping for "key" */
	    return map[entry].to;    /* -> return mapped key */
   return key;    /* no mapping -> just return original key */
}

/*
 * Interactive pager.
 *
 * Until some command requirering returning from pager (following link,
 * pressing 'q' etc.) issued: Display part of output page on curses screen;
 * read input key; move visible part according to pressed key function.
 *
 * When some command was issued that requires quitting the pager and returning
 * to the main program, return a flag indicating what was the cause, so main()
 * knows what to do.
 */

enum Pager_ret display(page)
struct Page	*page;
{
   enum Pager_ret	quit=RET_NO;

   /* create initial pager screen */

   hold_winch();    /* in fullscreen mode, allow curses resizing only during keywait (to prevent faults due to unexpected changes in screen size) */

   /* handle anchor/link/page position/cursor position */
   {
      int	active_link=page->active_link;    /* need to backup, as may be overwritten by scroll_to() or set_cursor() */
      int	cursor_x=page->cursor_x;
      int	cursor_y=page->cursor_y;

      old_line=PAGE_INVALID;    /* force repaint */

      if(page->active_anchor>=0)    /* anchor requested -> jump to it */
	 activate_anchor(page);
      else {    /* no anchor -> just go to current page position */
	 scroll_to(page, page->pager_pos);    /* display visible part of page */

	 set_cursor(page, cursor_x, cursor_y);

	 if(search.type==SEARCH_NO) {    /* prevent forced reactivation after text search (cursor position has to be kept in this case) */
	    search.type=SEARCH_NO;    /* only jump to search position once */
	    if(active_link>=0)    /* some link active -> force highlighting (necessary after reloading from history) */
	       activate_link(page, active_link);
	 }
      }
   }

   /* keyboard loop */
   do {    /* until quit */
      int	cursor_x, cursor_y;    /* screen position of cursor */

      cursor_y=page->cursor_y - page->pager_pos;
      cursor_x=page->cursor_x;    /* no horizontal scrolling yet, so screen position is equal page position */

#ifdef DEBUG
      if((unsigned)cursor_x>=(unsigned)COLS || (unsigned)cursor_y>=(unsigned)LINES) {
	 endwin();
	 fprintf(stderr, "internal error: trying to set cursor outside screen (x: %d; y: %d)\n", cursor_x, cursor_y);
	 exit(100);
      }
#endif

      /* get key */
      {
	 int	key;

	 enable_winch();    /* allow SIGWINCH during keywait */
	 key=remap(mvgetch(cursor_y, cursor_x));
	 hold_winch();

	 switch(key) {

	    /* cursor left */
	    case 'H'-'@':    /* ^H */
	       set_cursor(page, page->cursor_x-1, page->cursor_y);
	       break;

	    /* cursor down */
	    case 'J'-'@':    /* ^J */
	       set_cursor(page, page->cursor_x, page->cursor_y+1);
	       break;

	    /* cursor up */
	    case 'K'-'@':    /* ^K */
	       set_cursor(page, page->cursor_x, page->cursor_y-1);
	       break;

	    /* cursor right */
	    case 'L'-'@':    /* ^L */
	       set_cursor(page, page->cursor_x+1, page->cursor_y);
	       break;

	    /* one line down */
	    case 'j':
	       scroll_to(page, page->pager_pos+1);
	       break;

	    /* one line up */
	    case 'k':
	       scroll_to(page, page->pager_pos-1);
	       break;

	    /* two lines down */
	    case KEY_DC:    /* DEL */
	       scroll_to(page, page->pager_pos+2);
	       break;

	    /* two lines up */
	    case KEY_IC:    /* INS */
	       scroll_to(page, page->pager_pos-2);
	       break;

	    /* screen forward */
	    case 'F'-'@':    /* ^f */
	    case ' ':
	       scroll_to(page, page->pager_pos+LINES);
	       break;

	    /* screen backward */
	    case 'B'-'@':    /* ^b */
	       scroll_to(page, page->pager_pos-LINES);
	       break;

	    /* half screen forward */
	    case 'D'-'@':    /* ^d */
	    case KEY_NPAGE:
	       scroll_to(page, page->pager_pos+LINES/2);
	       break;

	    /* half screen backward */
	    case 'U'-'@':    /* ^u */
	    case KEY_PPAGE:
	       scroll_to(page, page->pager_pos-LINES/2);
	       break;

	    /* page top */
	    case 'g':
	    case KEY_HOME:
	       scroll_to(page, 0);
	       break;

	    /* page bottom */
	    case 'G':
	    case KEY_END:
	       scroll_to(page, page->layout->item_tree->y_end-LINES);
	       break;

	    /* down/next link */
	    case 'J':
	    case KEY_DOWN: {
	       int	new_link;    /* link that will be activated */

	       new_link=find_link(page, page->cursor_x+(page->active_link >= 0), page->cursor_y, -1, -1, active_end(page, 1), FIND_FIRST);    /* first link after cursor (or at, if no link active yet), ending at most in the lines(s) becoming active when scrolling one line forward */

	       if(new_link>=0)    /* some new link found */
		  activate_link(page, new_link);    /* -> activate it */
	       else    /* nothing found */
		  scroll_to(page, page->pager_pos+1);    /* -> just scroll a line forward */

	       break;
	    }

	    /* up/next link */
	    case 'K':
	    case KEY_UP: {
	       int	new_link;    /* link that will be activated */

	       new_link=find_link(page, -1, active_start(page, -1), page->cursor_x, page->cursor_y, -1, FIND_LAST);    /* last link before cursor, starting at most in the lines(s) becoming active when scrolling one line back */

	       if(new_link>=0)    /* some new link found */
		  activate_link(page, new_link);    /* -> activate it */
	       else    /* nothing found */
		  scroll_to(page, page->pager_pos-1);    /* -> just scroll a line back */

	       break;
	    }

	    /* activate first link on screen */
	    case 'H':
	       activate_link(page, find_link(page, -1, active_start(page, 0), -1, -1, active_end(page, 0), FIND_FIRST));    /* activate first link in active screen area */
	       break;

	    /* activate last link on screen */
	    case 'L':
	       activate_link(page, find_link(page, -1, active_start(page, 0), -1, -1, active_end(page, 0), FIND_LAST));    /* activate last link in active screen area */
	       break;

	    /* activate first link in second screen half */
	    case 'M': {
	       activate_link(page, find_link(page, -1, page->pager_pos+LINES/2, -1, -1, active_end(page, 0), FIND_FIRST));    /* activate first link in second screen half */
	       break;
	    }

	    /* activate first link in next line having links (or scroll one line) */
	    case '+':
	    case '=': {
	       int	new_link;

	       new_link=find_link(page, -1, page->cursor_y+1, -1, -1, active_end(page, 1), FIND_FIRST);    /* -> find first link that starts in next line or behind (up to the last line becoming active when scrolling one line) */

	       if(new_link>=0)    /* something found */
		  activate_link(page, new_link);    /* -> activate */
	       else    /* no new link found */
		  scroll_to(page, page->pager_pos+1);    /* -> just scroll one line */

	       break;
	    }

	    /* activate first link in previous line having links (or scroll one line) */
	    case '-': {
	       int	new_link;

	       /* first, get the last link starting before the current line */
	       new_link=find_link(page, -1, active_start(page, -1), -1, page->cursor_y, -1, FIND_LAST);    /* last link starting before current line, up to first line inside area active now or after scrolling one line */

	       /* now, activate the first link on the same line */
	       if(new_link>=0) {    /* something found */
		  activate_link(page, find_link(page, -1, get_link(page->layout, new_link)->y_start, -1, -1, -1, FIND_FIRST));
	       } else    /* no new link found */
		  scroll_to(page, page->pager_pos-1);    /* -> just scroll one line */

	       break;
	    }

	    /* activate first link starting on line */
	    case '^':
	    case 'A'-'@':    /* ^A */
	       activate_link(page, find_link(page, -1, page->cursor_y, -1, page->cursor_y+1, -1, FIND_FIRST));
	       break;

	    /* activate first link in line */
	    case '0': {
	       int	new_link;

	       /* first, find last link ending before current line */
	       new_link=find_link(page, -1, -1, -1, -1, page->cursor_y, FIND_LAST);

	       /* now, activate next link */
	       if(new_link>=0)    /* there is some link ending before current line */
		  activate_link(page, find_link(page, get_link(page->layout, new_link)->x_start+1, get_link(page->layout, new_link)->y_start, -1, page->cursor_y+1, -1, FIND_FIRST));    /* -> activate next link (if starts before or on current line) */
	       else    /* no link ends before current line */
		  activate_link(page, find_link(page, -1, -1, -1, page->cursor_y+1, -1, FIND_FIRST));    /* activate first link on page (if starts before or on current line) */
	       break;
	    }

	    /* activate last link starting on line */
	    case '$':
	    case 'E'-'@':    /* ^E */
	       activate_link(page, find_link(page, -1, page->cursor_y, -1, page->cursor_y+1, -1, FIND_LAST));
	       break;

	    /* activate first link on page */
	    case KEY_BACKSPACE:
	       if(page->layout->links->count)    /* any links on page */
		  activate_link(page, 0);    /* -> activate first */
	       break;

	    /* activate next link */
	    case 'I'-'@': {    /* Tab (^I) */
	       int	new_link;

	       new_link=find_link(page, page->cursor_x+(page->active_link >= 0), page->cursor_y, -1, -1, -1, FIND_FIRST);    /* first link after cursor (or at cursor if no link active yet) */
	       if(new_link>=0)
		  activate_link(page, new_link);
	       break;
	    }

	    /* activate previous link */
	    case 'p': {
	       int	new_link;

	       new_link=find_link(page, -1, -1, page->cursor_x, page->cursor_y, -1, FIND_LAST);    /* first link before cursor */
	       if(new_link>=0)
		  activate_link(page, new_link);
	       break;
	    }

	    /* follow active link */
	    case '\r':    /* <return> */
	       if(page->active_link>=0)    /* some link active -> follow */
		  quit=RET_LINK;
	       else
		  flash();
	       break;

	    /* show link URL */
	    case 'u':
	       if(page->active_link>=0)    /* some link active */
		  quit=RET_LINK_URL;
	       else
		  flash();
	       break;

	    /* show absolute link target URL */
	    case 'U':
	       if(page->active_link>=0)    /* some link active */
		  quit=RET_ABSOLUTE_URL;
	       else
		  flash();
	       break;

	    /* show current page URL */
	    case 'c':
	       quit=RET_URL;
	       break;

	    /* reload page */
	    case 'R'-'@':    /* ^r */
	       if(page->url->proto.type!=PT_INTERNAL)    /* normal page */
		  quit=RET_HISTORY;    /* reload (same) page */
	       else    /* stdin -> can't reload */
		  flash();
	       break;

	    /* back to previous URL */
	    case 'b':
	    case KEY_LEFT:
	       if(page_list.pos > 0) {    /* any URL(s) to go back */
		  --page_list.pos;
		  quit=RET_HISTORY;
	       } else
		  flash();
	       break;

	    /* forward to next URL (after 'b' or other commands going back in history) */
	    case 'f':
	    case KEY_RIGHT:
	       if(page_list.pos < page_list.count-1) {    /* can go forward (not at last entry) */
		  ++page_list.pos;
		  quit=RET_HISTORY;
	       } else
		  flash();
	       break;

	    /* back to URL before last absolute link/load */
	    case 'B':
	       if(page_list.pos > 0) {    /* any URL(s) to go back */
		  int	pos;

		  for(pos=page_list.pos-1; pos>0; --pos)    /* traverse history backwards (stop at first entry) */
		     if(page_list.page[pos+1]->url->absolute)    /* found matching entry (a page before one with absolute URL) */
			break;
		  page_list.pos=pos;    /* take found entry (or first entry, if nothing found) */

		  quit=RET_HISTORY;
	       } else    /* already at first entry */
		  flash();
	       break;

	    /* forward to URL before next absolute link/load */
	    case 'F':
	       if(page_list.pos < page_list.count-1) {    /* can go forward (not at last entry) */
		  int	pos;

		  for(pos=page_list.pos+1; pos<page_list.count-1; ++pos)    /* traverse history forward (stop at last entry) */
		     if(page_list.page[pos+1]->url->absolute)    /* found matching entry (a page before one with absolute URL) */
			break;
		  page_list.pos=pos;    /* take found entry (or last entry, if nothing found) */

		  quit=RET_HISTORY;
	       } else    /* already at last entry */
		  flash();
	       break;

	    /* return to previous mark */
	    case 'r':
	       if(page_list.pos > 0) {    /* any URL(s) to go back */
		  int	pos;

		  for(pos=page_list.pos-1; pos>0; --pos)    /* traverse history backwards (stop at first entry) */
		     if(page_list.page[pos]->mark)    /* found a marked page */
			break;
		  page_list.pos=pos;    /* take found entry (or first entry, if nothing found) */

		  quit=RET_HISTORY;
	       } else    /* already at first entry */
		  flash();
	       break;

	    /* forward to next mark */
	    case 'R':
	       if(page_list.pos < page_list.count-1) {    /* can go forward (not at last entry) */
		  int	pos;

		  for(pos=page_list.pos+1; pos<page_list.count-1; ++pos)    /* traverse history forward (stop at last entry) */
		     if(page_list.page[pos]->mark)    /* found a marked page */
			break;
		  page_list.pos=pos;    /* take found entry (or first entry, if nothing found) */

		  quit=RET_HISTORY;
	       } else    /* already at last entry */
		  flash();
	       break;

	    /* set page mark */
	    case 's':
	       if(page->url->proto.type!=PT_INTERNAL)    /* normal page */
		  page->mark=1;
	       else    /* internal -> can't mark */
		  flash();
	       break;

	    /* unset page mark */
	    case 'S':
	       if(page->url->proto.type!=PT_INTERNAL)    /* normal page */
		  page->mark=0;
	       else    /* internal -> can't (un)mark */
		  flash();
	       break;

	    /* text search */
	    case '/':
	       search.type=SEARCH_FORWARD;
	       quit=RET_SEARCH;
	       break;

	    /* enter one command in ex mode */
	    case ':':
	       quit=RET_COMMAND;
	       break;

	    /* quit */
	    case 'q':
	       quit=RET_QUIT;
	       break;

	    /* SIGWINCH received */
	    case KEY_RESIZE:
	       /* discard ERR caused by signal during getch() */
	       if(nodelay(stdscr, TRUE)!=ERR) {    /* prevent waiting if nothing pending (don't do anything if nodelay() fails!) */
		  int key=getch();
		  if(key!=ERR)    /* some normal key -> save it */
		     ungetch(key);
	       }
	       nodelay(stdscr, FALSE);

	       quit=RET_WINCH;
	       break;

#ifdef DEBUG
	    case ERR:
	       endwin();
	       fprintf(stderr, "internal error: getch() failed (can't read keyboard input)\n");
	       exit(100);
	       break;
#endif
	    
	    /* wrong key */
	    default:
	       flash();
	 }
      }    /* get key */

      if(page->active_anchor>=0) {
	 page->active_anchor=-1;
	 activate_anchor(page);
      }
   } while(quit==RET_NO);

   end_fullscreen();
   return quit;    /* main program needs to know whether to quit or to enter command prompt */
}
