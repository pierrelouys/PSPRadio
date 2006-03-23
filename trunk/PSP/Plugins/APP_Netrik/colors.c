/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/

/*
 * colors.c -- define and initialize the color mapping table
 *
 * (C) 2003 antrik
 */

#include "colors.h"
#include "cfg.h"

int	*color_map;

void load_color_map(void)
{
   if(cfg.bright_background)
      color_map=(int *)color_map_bright;
   else
      color_map=(int *)color_map_dark;
}
