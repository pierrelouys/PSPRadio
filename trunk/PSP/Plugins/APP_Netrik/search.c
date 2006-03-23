/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * search.c -- define search state struct
 *
 * (C) 2003 antrik
 */
#include <stdio.h>

#include "search.h"

struct Search	search={
   SEARCH_NO,    /* type */
   NULL    /* string */
};
