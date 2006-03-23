/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * colors.c -- this file defines colors for all the text types
 *
 * (C) 2001, 2002, 2003 antrik
 *
 * Alternative color definitions; looks better on dark background.
 *
 * The color is generated as follows:
 *
 * +-------+-------+-------+-------+-------+-------+-------+-------+
 * |bright :                       |bright :                       |
 * |back-  :   background color    |fore-  :   foreground color    |
 * |ground :                       |ground :                       |
 * +-------+-------+-------+-------+-------+-------+-------+-------+
 * (e.g. 0x80|COLOR_GREEN<<4|0x08|COLOR_BLUE is bright blue on bright green)
 *
 * Note: Setting any background attributes for the text types or setting any
 * foreground attributes for TM_ACTIVE may cause strange effects, as link
 * activation is done by bit-masking with TM_ACTIVE color. Certain combinations
 * may work, but you have been warned :-)
 */
#include <curses.h>

#include "colors.h"
#include "items.h"

const int	color_map_dark[]={
   COLOR_WHITE,    /* TM_NORMAL */
   COLOR_BLACK|8,    /* TM_DIM */
   COLOR_MAGENTA,    /* TM_ITALIC */
   COLOR_CYAN,    /* TM_LINK */
   COLOR_RED,    /* TM_FORM */
   COLOR_GREEN,    /* TM_IMG */
   COLOR_YELLOW|8,    /* TM_SYS (internally generated symbols) */
   COLOR_BLACK<<4|0x80    /* TM_ACTIVE (activated link) */
};
