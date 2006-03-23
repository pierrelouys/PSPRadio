/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * cfg.c -- default configuration
 *
 * (C) 2001, 2002 Patrice Neff
 *     2002, 2003 antrik
 */

#include <stdio.h>
#include <string.h>

#include "cfg.h"

struct Config cfg={
   -1, /* force_colors (no default; determined at run time if not explicitely set) */
   1, /* term_width */
#ifndef XHTML_ONLY
   BROKEN_HTML, /* parser */
#else
   0, /* parser */
#endif
   0, /* debug */
   0, /* warn_unknown */
   0, /* dump */
   0, /* wget */
   1, /* link_margin */
   5, /* anchor_offset */
   0, /* cursor_keys */
   "<<", /* anchor_mark */
   -1,    /* inverse_bold (no default; determined at run time if not explicitely set) */
   0,    /* bright_background */
   1,    /* color */
   1  /* proxy */
};
