/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/

/*
 * items.c -- helper functions for handling the page structure tree
 *
 * (C) 2003 antrik
 *
 * These functions help in retrieving some common data from the item tree.
 */

#include "items.h"

/* Get the starting position of a specific line inside a text block.
 * The line number is relative to the page, not the text item. */
int line_start(item, line)
const struct Item	*item;
int			line;
{
   const struct String	*string=item->data.string;    /* current string */
   const int		string_line=line-item->y_start;    /* current line number relatively to beginning of text block */

   int	start;

   if(string_line)    /* not first line of text block */
      start=string->line_table[string_line-1];    /* start of desired line */
   else    /* first line */
      start=0;

   return start;
}

/* Get the ending position of a specific line inside a text block.
 * The line number is relative to the page, not the text item. */
int line_end(item, line)
const struct Item	*item;
int			line;
{
   const struct String	*string=item->data.string;    /* current string */
   const int		string_line=line-item->y_start;    /* current line number relatively to beginning of text block */

   int	end;

   if(line<item->y_end-1)    /* not last line of text block */
      end=string->line_table[string_line];    /* start of next(!) line is end of the desired one */
   else    /* last line */
      end=string->div[string->div_count-1].end;    /* string end */

   if((unsigned)string->text[end-1]<=' ') --end;    /* discard blanks/newlines at line end */

   return end;
}

/* Get starting x position (column) of a specific (possibly centered) line from
 * a text block. (Returned in page coordinates.) The line number is relative to
 * the page, not the text item. */

int line_pos(item, line)
const struct Item	*item;
int			line;
{
   int	pos;

   if(item->center==0)    /* normal */
      pos=item->x_start;    /* just return item position (no offset) */
   else    /* centered */
      pos=item->x_start + ((item->x_end-item->x_start) - (line_end(item, line)-line_start(item,line))) / 2;    /* offset=(block width-line width)/2 */

   return pos;
}
