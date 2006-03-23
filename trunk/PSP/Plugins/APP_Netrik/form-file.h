/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/

/*
 * textarea.h -- declare edit_textarea() from textarea.c
 *
 * (C) 2003 antrik
 */

#include "items.h"    /* for "struct Data_string" */

char *edit_textarea(struct Data_string *value, char *name);    /* edit a "textarea" with external editor */
struct Data_string form_read_file(const char *file_name);    /* read a file into memory */
