/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * debug.c -- contains functions for printing the messages created in debug
 * mode.
 *
 * (C) 2001 antrik
 *
 * If not compiled with -DDEBUG, this file generates no code.
 */

#include "config.h"

#ifdef DEBUG

#include <stdio.h>
#include <stdarg.h>

#include "debug.h"

/* print a debug message using printf() paramters;
 * presently, this simply makes a printf() to "stderr" */
void debug_printf(char *fmt, ...)
{
   va_list	arg_ptr;

   va_start(arg_ptr, fmt);
   vfprintf(stderr, fmt, arg_ptr);
   va_end(arg_ptr);

   fflush(stderr);    /* make sure message is printed in time */
}

#endif    /* not DEBUG */
