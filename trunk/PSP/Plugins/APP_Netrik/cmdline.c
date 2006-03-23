/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * cfg.c -- read configuration (command line / config file)
 *
 * (C) 2001 Patrice Neff
 *     2001, 2002, 2003 antrik
 */

#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "cmdline.h"

/* read command line and write values into the struct */
int config_cmdln(argc, argv)
int	argc;
char	*argv[];
{
   /* available command line options */
   const struct option		longopts[]={
      { "force-colors", 0, &cfg.force_colors, 1 },
      { "no-force-colors", 0, &cfg.force_colors, 0 },
      { "term-width", 0, &cfg.term_width, 1 },
      { "no-term-width", 0, &cfg.term_width, 0 },
      { "fussy-html", 0, (int *)&cfg.parser, FUSSY_HTML },
      { "clean-html", 0, (int *)&cfg.parser, CLEAN_HTML },
      { "valid-html", 0, (int *)&cfg.parser, VALID_HTML },
      { "broken-html", 0, (int *)&cfg.parser, BROKEN_HTML },
      { "ignore-broken", 0, (int *)&cfg.parser, IGNORE_BROKEN },
      { "debug", 0, &cfg.debug, 1 },
      { "no-debug", 0, &cfg.debug, 0 },
      { "warn-unknown", 0, &cfg.warn_unknown, 1 },
      { "no-warn-unknown", 0, &cfg.warn_unknown, 0 },
      { "dump", 0, &cfg.dump, 1 },
      { "no-dump", 0, &cfg.dump, 0 },
      { "builtin-http", 0, &cfg.wget, 0 },
      { "no-builtin-http", 0, &cfg.wget, 1 },
      { "proxy", 0, &cfg.proxy, 1 },
      { "no-proxy", 0, &cfg.proxy, 0 },
      { "anchor-offset", 0, &cfg.anchor_offset, 5 },
      { "no-anchor-offset", 0, &cfg.anchor_offset, 0 },
      { "cursor-keys", 0, &cfg.cursor_keys, 1 },
      { "no-cursor-keys", 0, &cfg.cursor_keys, 0 },
      { "xterm", 0, &cfg.inverse_bold, 1 },
      { "console", 0, &cfg.inverse_bold, 0 },
      { "bright-background", 0, &cfg.bright_background, 1 },
      { "dark-background", 0, &cfg.bright_background, 0 },
      { "bw", 0, &cfg.color, 0 },
      { "color", 0, &cfg.color, 1 },
      /* this element ends array, don't remove */
      { 0, 0, 0, 0 }
   };

   int c=0; /* return value */
   int option_index=0; /* set option */

   while(1) {
      c=getopt_long(argc, argv, "", longopts, &option_index);
      if(c==-1) /* end of options */
         break;
   }

   return(optind);
}
