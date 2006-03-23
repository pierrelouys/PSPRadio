/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * url.h -- declarations for url.c
 *
 * (C) 2002 antrik
 *
 * Declares the Url structure used to store parsed URLs, and the functions for
 * parsing the URL and other URL handling.
 */

#ifndef __url_h
#define __url_h

#include "items.h"

/* parsed URL data */
struct Url {
   char			*full_url;    /* complete URL string (without fragment identifier) */
   char 		*path;    /* position of begining of path specification inside "full_url" */

   struct {
      char	*str;
      enum Protocol {
	 PT_UNKNOWN,
	 PT_INTERNAL,
	 PT_FILE,
	 PT_HTTP,
	 PT_FTP
      }		type;
   }			proto;
   char			*host;
   int			port;

   char			*dir;    /* path (without file name) */
   char			*name;    /* file name */
   char			*params;    /* CGI parameters */
   char			*frag;    /* fragment identifier */

   int			absolute;    /* the (merged) URL resulted from an absolute URL string */
   int			local;    /* same page as previous; only fragment identifier differs */
};


struct Url *merge_urls(const struct Url *base, const char *main_url, const char *form);    /* merges two URLs (plus form data) to create a new one */
struct Url *split_url(const char *url);    /* parse URL */
void free_url(struct Url *url);    /* free parsed URL struct */
char *url_unescape(const char *url);    /* replace %xx codes */

#endif
