/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * colors.h -- declares functions and variables for color map handling
 *
 * (C) 2003 antrik
 */

#ifndef _colors_h
#define _colors_h

extern int	*color_map;    /* colors actually associated to text modes */

/* candidates for "color_map" (from colors-dark.c/colors-bright.c) */
extern const int	color_map_dark[];
extern const int	color_map_bright[];

void load_color_map(void);

#endif
