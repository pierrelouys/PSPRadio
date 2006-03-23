/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * forms.h -- declares form handling functions in forms.c, and data structures
 * needed
 *
 * (C) 2002, 2003 antrik
 */
#include "layout.h"    /* for layout structure */

#define MIME_BOUNDRY "01iuo040c8""7l4nv20nco0"    /* should be unlikely enough :-) (the "" in the middle is to prevent trouble if someone sends *this* source file...) */

void set_form(const struct String *string, const struct Link *link);    /* write form element value into string */
void update_form(const struct Layout_data *layout, int link_num);    /* update form element rendering */

struct Item *get_form_item(const struct Layout_data *layout, int link_num);    /* get form item containing link */

struct Form_handle {    /* iterator handle for form_start()/form_next() */
   int			filter;    /* only return "successful" form elements */

   const struct Item	*form_item;
   struct Item		*cur_item;
   int			cur_link;
};

struct Form_handle form_start(const struct Item *form, int filter);    /* prepare a handle for form_next() */
struct Link *form_next(struct Form_handle *handle);    /* retrieve form elements (one at a time) */

struct Data_string url_encode(const struct Item *form);    /* store form data in URL-encoded string */
struct Data_string mime_encode(const struct Item *form);    /* store form data in MIME-encoded string */
