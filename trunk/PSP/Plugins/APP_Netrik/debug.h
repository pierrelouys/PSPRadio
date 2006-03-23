/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * debug.h -- contains macros generating messages in debug mode and definitions
 * for debug.c
 *
 * (C) 2001 antrik
 *
 * If not compiled with -DDEBUG, the DMSG macro is defined to do nothing.
 */
#include <stdio.h>
#include <stdarg.h>

#include "config.h"

#include "cfg.h"

#ifdef DEBUG

   /*
    * if in debug mode, print message; otherwise do nothing.
    *
    * Note: paramter list for DMSG must be enclosed in double parenthesis!
    * (outer parenthesis are for the macro, inner ones for debug_printf() )
    */
   #define DMSG(args) if(cfg.debug) debug_printf args

void debug_printf(char *fmt, ...);    /* print a message */

#else    /* not DEBUG */

   #define DMSG(args)    /* don't compile anything for DMSG */

#endif
