/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * screen.h -- prototypes/definitions of screen handling functions & global vars.
 *
 * (C) 2001, 2002 antrik
 */

void start_curses(void);    /* init curses full screen mode */
void set_color(int color);    /* set text attributes */
void end_fullscreen(void);    /* leave curses full screen mode */

int init_curses(void);    /* init curses (not full screen) */
void set_color_raw(int color);    /* set text attributes in raw (not fullscreen) mode */
void reset_color_raw(void);    /* reset text attributes to default */
