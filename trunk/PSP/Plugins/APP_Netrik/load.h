/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * load.h -- data structures and extern function declarations for load.c
 *
 * (C) 2001, 2002 antrik
 *     2001, 2002 Patrice Neff
 */

#ifndef __LOAD_H
#define __LOAD_H

#include <stdio.h>

#include "items.h"
#include "url.h"

struct Http_headers {
   int			count;
   struct Header {
      char	*name;
      char	*value;
   }			*header;
};

/* Patrice --> */
struct Http_handle {
   int	socket;
   struct Http_headers	headers;
};
/* <-- Patrice */

enum Res_type {
   RES_FAIL,    /* failed to open resource */
   RES_STDIN,
   RES_FILE,
   RES_HTTP,
   RES_PIPE
};

/* all data for an input resource (file etc.) */
struct Resource {
   struct Url		*url;    /* effective URL */
   enum Res_type	type;
   union {    /* specific for every ressource type */
      FILE	*stream;
      struct Http_handle
		*http;
   } 			handle;

   char			*buf;    /* input buffer */
   char			*buf_end;    /* end of data in input buffer */
   char			*buf_ptr;    /* current read position in buffer */

   int			user_break;    /* SIGINT recieved while loading resource */
};

struct Resource *init_load(const struct Url *base, const char *url, const struct Item *form_data);    /* build new URL, and prepare for reading */
void load(struct Resource *res);    /* read data block into buffer */
void uninit_load(struct Resource *res);    /* tidy up after reading a file */

#endif
