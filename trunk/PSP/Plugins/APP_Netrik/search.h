/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * search.h -- declare struct for search state
 *
 * (C) 2003 antrik
 */

enum Search_type {
   SEARCH_NO=0,
   SEARCH_FORWARD
};

struct Search {
   enum Search_type	type;
   char			*string;
};

extern struct Search	search;
