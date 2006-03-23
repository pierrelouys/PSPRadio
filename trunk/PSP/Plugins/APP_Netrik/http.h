/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * http.h -- declarations for http.c
 *
 * (C) 2001 Patrice Neff
 *     2002 antrik
 */

#ifndef __HTTP_H
#define __HTTP_H

#include "load.h"    /* need "struct Resource" */

void http_init_load(struct Resource *res, const struct Item *form);    /* prepare http connection for reading data */

#endif
