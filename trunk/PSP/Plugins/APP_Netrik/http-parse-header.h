/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * http-parse-header.h -- declarations for http-parse-header.o
 *
 * (C) 2002 Patrice Neff
 */

#ifndef __HTTP_PARSE_HEADER_H
#define __HTTP_PARSE_HEADER_H

#include "load.h"
void parse_header(struct Resource *res);

#endif
