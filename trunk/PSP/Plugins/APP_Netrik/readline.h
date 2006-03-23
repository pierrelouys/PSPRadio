/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/

/*
 * readline.h -- handle Readline library
 *
 * (C) 2002, 2003 antrik
 *
 * Declares the read_line() wrapper/replacement function for readline().
 *
 * Also includes original headers if the Readline library is available.
 * Otherwise, it maps add_history() to nothing, as we have no replacement for
 * it.
 */

#include "config.h"
#ifdef READLINE
   #include <readline/history.h>
   #include <readline/readline.h>
#else
   #define add_history(string)
#endif

char *read_line(const char *prompt);
