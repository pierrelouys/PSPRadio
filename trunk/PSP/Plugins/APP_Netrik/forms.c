/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * forms.c -- helper functions for HTML form handling
 *
 * (C) 2002, 2003 antrik
 *     (2002 Patrice Neff) -- all code obsoleted
 */
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "forms.h"
#include "form-file.h"
#include "layout.h"

static void encode_string(const struct Data_string string, struct Data_string *result, int *alloc_size);     /* URL-encode string */

/* write the value of a given form element into the string */
void set_form(string, link)
const struct String	*string;
const struct Link	*link;
{
   int	data_div;    /* string div showing the form data */
   char	*write_pos;    /* starting position of form value in string */
   
   /* find data_div */
   for(data_div=1; string->div[data_div-1].end<=link->start; ++data_div) {    /* find first div *after* link start (first link div is form marker!) */
#ifdef DEBUG
      if(data_div >= string->div_count) {
	 fprintf(stderr, "internal error: trying to work behind string end (while processing form)\n");
	 exit(100);
      }
#endif
   }

   write_pos=&string->text[string->div[data_div-1].end];

   switch(link->form) {    /* dispatch on form element type */
      case FORM_TEXT:
      case FORM_PASS:
      case FORM_HIDDEN:
      case FORM_FILE:
      case FORM_TEXTAREA: {
	 const int	size=string->div[data_div].end - string->div[data_div-1].end;    /* size of value part displayed in page */

	 char	*src, *dst;

	 for(src=link->value.data, dst=write_pos; dst < write_pos+size; ++src, ++dst) {    /* all characters of value display area */
	    if(src < link->value.data+link->value.size) {    /* still data to store */
	       if(link->form!=FORM_PASS)    /* display value */
		  *dst= isprint(*src) ? *src : ' ';
	       else    /* hide value */
		  *dst='*';
	    } else    /* no more data -> pad */
	       *dst='_';
	 }

	 break;
      }    /* FORM_TEXT */

      case FORM_CHECKBOX:
      case FORM_RADIO:
	 *write_pos=link->enabled?'*':'\xa0';
	 break;

      case FORM_OPTION:
      case FORM_MULTIOPTION:
	 *(write_pos-1)=link->enabled?'+':'-';    /* modify char *before* "data_div" */

      case FORM_SUBMIT:
	 break;    /* nothing to store */

      case FORM_NO:
	 break;    /* shouldn't occur */
   }    /* switch form type */

}

/* update the page (item tree) to the new value of a form element given by its
 * link number */
void update_form(layout, link_num)
const struct Layout_data	*layout;
int				link_num;
{
   const struct Link_ptr	*list_entry=&layout->links->link[link_num];    /* entry in link list pointing to form element */
   const struct Item		*item=list_entry->item;    /* (string) item containing form element */
   const struct String		*string=item->data.string;    /* string containing form element */
   const struct Link		*link=&string->link[list_entry->num];    /* link (form) data */

   set_form(string, link);
}

/* find form item of form to which the given (form) link belongs */
struct Item *get_form_item(layout, link_num)
const struct Layout_data	*layout;
int				link_num;    /* link number of form element */
{
   struct Item	*link_item=layout->links->link[link_num].item;

   struct Item	*form_item;
   for(form_item=link_item; form_item->list_next!=NULL; form_item=form_item->parent)    /* search parent form item (go upward until tree top reached) */
      if(form_item->type==ITEM_FORM)    /* found */
	 return form_item;
   return NULL;    /* nothing found */
}

/* prepares a form handle that can be used then to iterate through all form
 * elements using form_next() */
struct Form_handle form_start(form, filter)
const struct Item	*form;    /* form item of desired form */
int			filter;    /* return only "successfull" forms */
{
   struct Form_handle	handle;

   handle.filter=filter;

   handle.form_item=form;
   for(handle.cur_item=(struct Item *)form; handle.cur_item->first_child!=NULL; handle.cur_item=handle.cur_item->first_child);    /* find very first item inside form */
   handle.cur_link=0;

   return handle;
}

/*
 * Iterates through all links of all sub-items of the form item stored in the
 * handle, and returns the link every time a form data element is found. NULL
 * indicates no more form elements.
 *
 * The handle keeps track of which items/links have already been processed.
 *
 * If "filter" was passed to form_start(), only "successful" elements are
 * returned, i.e. the ones that have to be submitted to the server. (Have name,
 * value, and are enabled.)
 */
struct Link *form_next(handle)
struct Form_handle	*handle;
{
   /* find next form control */
   for(; handle->cur_item!=handle->form_item; handle->cur_item=handle->cur_item->list_next) {    /* until back at form item (scan all sub-items) */
      if(handle->cur_item->type==ITEM_TEXT) {    /* can contain links */
	 for(; handle->cur_link < handle->cur_item->data.string->link_count; ++handle->cur_link) {    /* all remaining links in current item */
	    const struct Link	*link=&handle->cur_item->data.string->link[handle->cur_link];

	    if(link->form) {    /* found a form control */
	       if(handle->filter) {    /* only successful controls to be returned -> need to test */
		  if(link->name==NULL || !*link->name    /* no name */
		     || link->value.data==NULL || !*link->value.data
		     || !link->enabled
		  )    /* not successful -> ignore */
		     continue;
	       }

	       ++handle->cur_link;    /* proceed with next link next time we are called */
	       return (struct Link *)link;
	    }
	 }
	 handle->cur_link=0;    /* start with first link in next item */
      }    /* ITEM_TEXT */
   }    /* for all sub-items */
   return NULL;    /* no more form controls */
}


/* store a string, escaping characters according to URL encoding */
static void encode_string(string, result, alloc_size)
const struct Data_string	string;    /* string to encode */
struct Data_string		*result;    /* where to store */
int				*alloc_size;    /* allocated size of "data" */
{
   char	*src;    /* currently processed character position */

   for(src=(char *)string.data; src < string.data+string.size; ++src) {    /* all chars in input string */
      const unsigned char	chr=*src;    /* character to store */

      if(result->size > *alloc_size-5) {    /* not enough space left (5 chars may follow before next realloc in worst case: "%HH=&") */
	 result->data=realloc(result->data, *alloc_size+=16);
	 if(result->data==NULL) {
	    fprintf(stderr, "memory allocation error while preparing form data (in function encode_string())\n");
	    exit(1);
	 }
      }

      if(isascii(chr) && isgraph(chr) && !strchr("+=&%#", chr))    /* normal char */
	 result->data[result->size++]=chr;
      else if(chr==' ')
	 result->data[result->size++]='+';
      else {    /* needs to be escaped */
	 result->data[result->size++]='%';
	 result->data[result->size++]=(chr>>4) + ((chr>>4) > 9 ? 'A'-10 : '0');    /* upper hex digit */
	 result->data[result->size++]=(chr&0xf) + ((chr&0xf) > 9 ? 'A'-10 : '0');    /* lower hex digit */
      }
   }    /* for all chars in input string */
}

/*
 * store form data in URL-encoded string
 *
 * scans a form (from the item tree) and appends the name/value pair of each
 * successful form control to a URL-encoded string
 */
struct Data_string url_encode(form)
const struct Item	*form;    /* points to the desired form's describing item in structure tree */
{
   struct Form_handle	handle=form_start(form, 1);    /* state var for iterating through form elements (only succesful ones) */
   struct Link		*link;    /* link struct of current form element */

   struct Data_string	result;    /* URL-encoded form data */
   int			alloc_size;    /* current allocated size of "data" */

   result.data=NULL; result.size=alloc_size=0;
   while((link=form_next(&handle)) != NULL) {    /* for all successful form elements */
      struct Data_string	name={link->name, strlen(link->name)};    /* form elment's name as Data_string */
      struct Data_string	value= link->form!=FORM_FILE ? link->value : form_read_file(link->value.data);

      if(value.size<0) {    /* fail to obtain form value (in form_read_file()) -> bail out */
	 free(result.data);
	 return value;    /* pass on error string */
      }
	 
      encode_string(name, &result, &alloc_size);
      result.data[result.size++]='=';
      encode_string(value, &result, &alloc_size);
      result.data[result.size++]='&';

      if(link->form==FORM_FILE)
	 free(value.data);
   }
   
   if(result.size) {    /* anything stored -> discard last '&' */
      --result.size;
      result.data[result.size]='\0';    /* store C string end instead, for callers not using the "size" component */
   }

   return result;
}

/*
 * store form data in MIME-encoded string
 *
 * scans a form (from the item tree) and stores the name/value pair of each
 * successful form control as part of a multipart MIME encoded HTTP request body
 */
struct Data_string mime_encode(form)
const struct Item	*form;    /* points to the desired form's describing item in structure tree */
{
   struct Form_handle	handle=form_start(form, 1);    /* state var for iterating through form elements (only succesful ones) */
   struct Link		*link;    /* link struct of current form element */

   struct Data_string	result;    /* MIME-encoded form data */

   /* store data */
   result.data=NULL; result.size=0;
   while((link=form_next(&handle)) != NULL) {    /* all successful form elements */
      static const char	line_feed[]="\r\n";
      char		*format;

      char			*file_name;
      struct Data_string	value;

      int			part_len;    /* number of chars this form item will append */
	 
      if(link->form!=FORM_FILE) {    /* controls where data is stored in "value" directly */
	 value=link->value;
	 file_name="";
	 format="--"MIME_BOUNDRY"\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n""%s";    /* second "%s" for unused file_name parameter */
      } else {    /* file upload */
	 file_name=link->value.data;

	 value=form_read_file(file_name);
	 if(value.size<0) {    /* fail to obtain form value (in form_read_file()) -> bail out */
	    free(result.data);
	    return value;    /* pass on error string */
	 }

	 format="--"MIME_BOUNDRY"\r\nContent-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n\r\n";
      }

      part_len= strlen(format) + strlen(link->name) + strlen(file_name) + value.size + sizeof(line_feed);    /* a few bytes too much, but who cares... */
	       
      result.data=realloc(result.data, result.size+part_len);
      if(result.data==NULL) {
	 fprintf(stderr, "memory allocation error while preparing form data (in function mime_encode())\n");
	 exit(1);
      }

      result.size+=snprintf(&result.data[result.size], part_len, format, link->name, file_name);    /* store header */

      memcpy(&result.data[result.size], value.data, value.size);    /* store data */
      result.size+=value.size;

      if(link->form==FORM_FILE)
	 free(value.data);

      memcpy(&result.data[result.size], line_feed, sizeof(line_feed));    /* store linefeed */
      result.size+=strlen(line_feed);
   }

   /* store footer */
   {
      static const char	footer[]="--"MIME_BOUNDRY"--\r\n";

      result.data=realloc(result.data, result.size+sizeof(footer));
      if(result.data==NULL) {
	 fprintf(stderr, "memory allocation error while preparing form data (in function mime_encode())\n");
	 exit(1);
      }
      memcpy(&result.data[result.size], footer, sizeof(footer));
      result.size+=strlen(footer);
   }

   return result;
}
