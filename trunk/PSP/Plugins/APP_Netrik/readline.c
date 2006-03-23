/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * readline.c -- readline() wrapper function/replacement
 *
 * (C) 2002, 2003 antrik
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "readline.h"

/* 
 * readline() wrapper: Normally, this function just calls the normal readline()
 * from the GNU Readline library, and cleans up the mess readline() does to the
 * "COLUMNS" and "LINES" environment variables. (Necessary for SIGWINCH
 * handling to work.)
 *
 * If readline() isn't available however, it provides a cheap replacement using
 * fgets().
 */

char *read_line(prompt)
const char	*prompt;
{
   char		*input;    /* dynamic string to return */
#ifdef READLINE
   const int	use_cols=(getenv("COLUMNS")!=NULL);
   const int	use_lines=(getenv("LINES")!=NULL);

   input=readline(prompt);

   if(!use_cols)
      putenv("COLUMNS=");
   if(!use_lines)
      putenv("LINES=");
#else    /* don't have READLINE */
   char		buf[1024];    /* user input */

   printf("%s",prompt); fflush(stdout);

   fgets(buf, sizeof(buf), stdin);
   buf[strlen(buf)-1]=0;    /* discard trailing newline returned by fgets() */

   input=strdup(buf);
   if(input==NULL) {
      fprintf(stderr, "memory allocation error while storing input\n");
      exit(1);
   }
#endif    /* don't have READLINE */
   return input;
}
