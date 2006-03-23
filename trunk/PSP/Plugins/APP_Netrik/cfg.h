/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * cfg.h -- declare structure for storing configuration
 *
 * (C) 2001, 2002 Patrice Neff
 *     2001, 2002, 2003 antrik
 */

#ifndef __cfg_h
#define __cfg_h

enum Parser_mode {
   FUSSY_HTML,    /* immediately quit on any errors or warnings */
   CLEAN_HTML,    /* pause with warning message after parsing completes if any warnings or errors occured */
   VALID_HTML,    /* pause only if errors occured (not on warnings) */
   BROKEN_HTML,    /* pause only if severe errors occured for which we have no good workaround */
   IGNORE_BROKEN    /* never pause */
};

/* struct to store config */
/* also change cfg.c if you add a property */
struct Config {
   int force_colors;    /* don't use terminals default colors for normal text */
   int term_width;    /* use screen width from terminfo as output width */
   enum Parser_mode parser;    /* behaviour on HTML errors */
   int debug;    /* print debug messages */
   int warn_unknown;    /* issue warnings when encountering unknown elements or attributes */
   int dump;    /* dump whole page and exit */
   int wget;    /* use wget to retrieve files via HTTP instead of builtin loader */
   int link_margin;    /* number of lines at bottom&top of screen which can't have highlighted links */
   int anchor_offset;    /* when jumping to anchors, scroll so that the anchor will appear at 1/anchor_offset below the screen top (if 0, show anchor at link_margin) */
   int cursor_keys;    /* use arrow keys for cursor movement instead of navigation */
   char *anchor_mark;    /* marker displayed at end of line(s) when activating anchor */
   int inverse_bold;    /* terminal can't display bright background directly, only over reverse video */
   int bright_background;    /* use color definitions for bright background */
   int color;    /* try using colors */
   int proxy;    /* use "http_proxy" or "HTTP_PROXY" environment variables */
};
extern struct Config cfg;

#endif
