/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/

/*
 * render.c -- generates screen output from the item tree and page map.
 *
 * (C) 2001, 2002 antrik
 *
 * There are functions to dump the whole page to stdout line by line and to
 * render a specified region of the page to a specified (curses) screen region.
 *
 * There is also a debug function which dumps the item tree including the text
 * in the text items, including text attributes, but without the correct layout
 * like dump().
 */

#include <curses.h>
#include <stdlib.h>
#include <term.h>
#include <string.h>

#include "cfg.h"
#include "colors.h"
#include "items.h"
#include "layout.h"
#include "render.h"
#include "screen.h"

#define MIN(a,b) ((a)<(b)?(a):(b))

#ifdef DEBUG
/* dump structure tree;
 * dumps all items in item tree and their strings with correct attributes;
 * also prints positions of items on page */
void dump_items(item_tree)
struct Item	*item_tree;    /* top of item tree to dump */
{
   struct Item	*cur_item;    /* currently dumped item */
   struct Item	*new_item;    /* current item for depth calculations */

   char	*item_name[]={
      "TEXT",
      "BOX",
      "FORM",
      "BLANK",
      "BLOCK_ANCHOR",
      "INLINE_ANCHOR"
   };
   
   /* for all items (starting from top) */
   cur_item=item_tree;
   do {    /* until back at top */

      /* draw tree */
      for(new_item=cur_item; new_item!=item_tree; new_item=new_item->parent)    /* ascend till top */
	 printf("|");
      printf("-");

      /* print item type */
      printf("%s", item_name[cur_item->type]);
      if(cur_item->type==ITEM_BLOCK_ANCHOR) {    /* virtual block -> list children */
	 struct Item	*item;

	 printf(" { ");
	 for(item=cur_item->data.block_anchor->virtual_child; item!=cur_item; item=item->list_next)
	    printf("%s ", item_name[item->type]);
	 printf("}");
      }

      if(cur_item->center)
	 printf(" (center)");

      printf(" (%d,%d - %d,%d)\n", cur_item->x_start, cur_item->y_start, cur_item->x_end, cur_item->y_end);    /* print page position */

      if(cur_item->type==ITEM_TEXT) {    /* text item -> output string */
	 int	cur_div;    /* div currently printed */

	 int	div_start;    /* starting position in string of currend div */
	 int	div_end;    /* starting position in string of next div */

	 for(cur_div=0, div_start=0; cur_div<cur_item->data.string->div_count; ++cur_div) {    /* complete string (all divisions) */
	    div_end=cur_item->data.string->div[cur_div].end;

	    set_color_raw(cur_item->data.string->div[cur_div].color);

	    /* output current div's text */
	    printf("%.*s", div_end-div_start, &cur_item->data.string->text[div_start]);

	    reset_color_raw();

	    div_start=div_end;
	 }    /* for all div's */
	 printf("\n");

	 if(cur_item->data.string->link_count) {
	    int	link;

	    set_color_raw(((COLOR_BLACK|8)<<4)+color_map[TM_LINK]); printf("Links:");
	    reset_color_raw(); printf("\n");

	    for(link=0; link<cur_item->data.string->link_count; ++link)
	       printf("   %d,%d-%d,%d\n", cur_item->data.string->link[link].x_start, cur_item->data.string->link[link].y_start, cur_item->data.string->link[link].x_end, cur_item->data.string->link[link].y_end);    /* print page position */
	 }

      }    /* if text item */

      fflush(stdout);    /* pagers behave funny on segfault... */

      /* next item */
      if(cur_item->first_child==NULL) {    /* no children -> go to next item */
	 while(cur_item->next==NULL)    /* if last item at this depth: ascend before going to next item */
	    cur_item=cur_item->parent;
	 cur_item=cur_item->next;
      } else    /* has children -> descend */
	 cur_item=cur_item->first_child;
   } while(cur_item!=item_tree);    /* for all items (until back at top) */

   printf("\n");
}
#endif

/* dump whole page to screen (stdout) with correct layout */
void dump(layout)
struct Layout_data	*layout;
{
   const struct Item		*item_tree=layout->item_tree;    /* top of item tree of page */
   const struct Item_list	*page_map=layout->page_map;    /* space usage map */

   int		line;    /* line number in page */

   for(line=0; line<item_tree->y_end; ++line) {    /* all lines in page */
      int	item_num;    /* item in current line */

      if(cfg.force_colors) {
         set_color_raw(7);    /* force white on black */
         printf("\n");    /* begin new line after setting background (otherwise the new background isn't applied to the whole line!) */
      }

      for(item_num=0; item_num<page_map[line].count; ++item_num) {    /* all items in line (NOTE: this function presently won't really work if there is actually more than one item in a line!) */
	 const struct Item	*item=page_map[line].item[item_num];    /* current text item */
	 const struct String	*string=item->data.string;    /* current string */

	 const int		text_start=line_start(item, line);    /* starting position of line inside string */
	 const int		text_end=line_end(item, line);    /* ending position of line inside string */
	 const int		x_start=line_pos(item, line);    /* starting column of line on page */

	 int	cur_div;    /* div currently printed */

	 int	div_start;    /* starting position in string of currend div */
	 int	div_end;    /* starting position in string of next div */

	 int	cursor_col;

	 /* set cursor col */
	 for(cursor_col=0; cursor_col<x_start; ++cursor_col)
	    putchar(' ');

	 /* seek div containing beginning of line */
	 for(cur_div=0; (div_end=string->div[cur_div].end)<=text_start; ++cur_div);

	 /* print line */
	 for(div_start=text_start; div_start<text_end; ++cur_div) {    /* all divs containing parts of line */
	    div_end=MIN(string->div[cur_div].end, text_end);    /* div containing end of line is truncated */

	    set_color_raw(string->div[cur_div].color);

	    /* output current div's text */
	    {
	       char	output_string[div_end-div_start+1];
	       char	*src, *dst;

	       /* copy printed string part, replacing &nbsp; chars */
	       for(src=&string->text[div_start], dst=output_string; src < &string->text[div_end]; ++src, ++dst) {
		  if(*src != '\xa0')
		     *dst=*src;
		  else
		     *dst=' ';
	       }
	       *dst=0;

	       printf("%s", output_string);
	    }

	    reset_color_raw();

	    div_start=div_end;    /* next div starts where current one ends */
	 }    /* next div */

	 if((unsigned)string->text[text_end]>' ') {    /* printable character follows end of line => word broken -> display continuation mark */
	    set_color_raw(color_map[TM_SYS]);
	    printf("\\");
	    reset_color_raw();
	 }

      }    /* next item in line */

      if(!cfg.force_colors)    /* end line (if force_color, a linewrap is done *before* printing the line content!) */
         printf("\n");

#ifdef DEBUG
      fflush(stdout);    /* pagers behave funny on segfault... */
#endif
   }    /* next line */

   if(cfg.force_colors)    /* need to end last line (no line breaks are noramlly issued *after* printing a line if force_colors!) */
      printf("\n");

}

/* render specified region of page and output to (curses) screen at specified position */
void render(layout, page_x, page_y, screen_x, screen_y, width, height, overpaint)
struct Layout_data	*layout;
int			page_x, page_y;    /* starting position of rendered region inside page */
int			screen_x, screen_y;    /* starting position on screen */
int			width, height;    /* size of region */
int			overpaint;    /* clean the screen area before rendering */
{
   const struct Item		*item_tree=layout->item_tree;    /* top of item tree of page */
   const struct Item_list	*page_map=layout->page_map;    /* space usage map */

   int		line;    /* line number in page */

   if(page_y+height>item_tree->y_end)    /* try to render lines after the page end -> truncate area */
      height=item_tree->y_end-page_y;

   /* clear area */
   if(overpaint) {
      int	clr_line;
      char	clr_string[width+1];

      memset(clr_string, ' ', width);
      set_color(color_map[TM_NORMAL]);
      for(clr_line=screen_y; clr_line<screen_y+height; ++clr_line)    /* all lines in area */
	 mvprintw(clr_line, screen_x, "%.*s", width, clr_string);    /* overprint with empty string */
   }
	 
   for(line=page_y; line<page_y+height; ++line) {    /* all lines in area */
      int	item_num;    /* item in current line */


      for(item_num=0; item_num<page_map[line].count; ++item_num) {    /* all items in line */
	 const struct Item	*item=page_map[line].item[item_num];    /* current text item */
	 const struct String	*string=item->data.string;    /* current string */

	 int	text_start;    /* position inside string of first char of rendered line part */
	 int	text_end;    /* position after last char */

	 int	x_start;    /* starting position of current line relative to rendered area */
	 int	x_end;    /* position after line end */

	 int	div_start;    /* starting position in string of currend div */
	 int	div_end;    /* starting position in string of next div */
	 int	cur_div;    /* div currently printed */

	 /* first assume whole line will be rendered */
	 text_start=line_start(item, line);
	 text_end=line_end(item, line);

	 /* calc starting column of line relative to start of rendered area */
	 x_start=line_pos(item, line) - page_x;
	 if(x_start<0) {    /* line starts before beginning of rendered area -> truncate */
	    text_start-=x_start;
	    x_start=0;
	 }

	 /* calc ending column */
	 x_end=x_start + (text_end-text_start);
	 if(x_end>width) {    /* line ends after end of rendered area */
	    text_end-=x_end-width;
	    x_end=width;
	 }

	 /* print line */
	 move(screen_y+(line-page_y), screen_x+x_start);    /* set cursor to beginning of line */

	 if(text_end>text_start) {    /* anything to print (some part of line is inside area) */
	    /* seek div containing beginning of line */
	    for(cur_div=0; (div_end=string->div[cur_div].end)<=text_start; ++cur_div);

	    /* output divs */
	    for(div_start=text_start; div_start<text_end; ++cur_div) {    /* all divs containing parts of this line */
	       div_end=string->div[cur_div].end;

	       if(div_end>text_end)    /* div exceeds line end -> truncate */
		  div_end=text_end;

	       set_color(string->div[cur_div].color);

	       /* output current div's text */
	       {
		  char	output_string[div_end-div_start+1];
		  char	*src, *dst;

		  /* copy printed string part, replacing &nbsp; chars */
		  for(src=&string->text[div_start], dst=output_string; src < &string->text[div_end]; ++src, ++dst) {
		     if(*src != '\xa0')
			*dst=*src;
		     else
			*dst=' ';
		  }
		  *dst=0;

		  printw("%s", output_string);
	       }

	       div_start=div_end;    /* next div starts where current one ends */
	    }    /* next div */
	 }

	 if(x_end>=0 && x_end<width && (unsigned)string->text[text_end]>' ') {    /* end of line inside area, and normal character follows => word broken -> display continuation mark */
	    set_color(color_map[TM_SYS]);
	    addch('\\');
	 }

      }    /* next item in line */

   }    /* next line */

}
