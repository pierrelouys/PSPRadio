/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * url.c -- URL handling functions
 *
 * (C) 2001, 2002 antrik
 *     2002 Patrice Neff
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "url.h"

static void dump_url(const struct Url *url);    /* print components of a split URL */
static void store_component(const char *start, const char *end, char **component);    /* store a URL field extracted by split_url() */
static void str_append(char **str, const char *append);    /* append string to end of other string, reallocating memory */

/* print components of a split URL */
static void dump_url(url)
const struct Url *url;
{
   debug_printf("   full url: %s\n", url->full_url);
   debug_printf("   full path: %s\n", url->path);
   debug_printf("   protocol: %s\n", url->proto.str);
   debug_printf("   host: %s\n", url->host);
   debug_printf("   port: %d\n", url->port);
   debug_printf("   directory: %s\n", url->dir);
   debug_printf("   file name: %s\n", url->name);
   debug_printf("   parameters: %s\n", url->params);
   debug_printf("   fragment identifier: %s\n", url->frag);
}

/* store a section representing one URL field extracted by split_url()
 * to the respective string */
void store_component(start, end, component)
const char	*start;
const char	*end;
char		**component;    /* where to store */
{
   const int	len=end-start;

   if(len>0) {
      *component=malloc(len+1);
      if(component==NULL) {
	 fprintf(stderr, "memory allocation error while parsing URL (in function store_component)\n");
	 exit(1);
      }

      strncpy(*component, start, len);
      (*component)[len]='\0';
   } else
      *component=NULL;
}
   
/* parse URL (split URL string into fields) */
struct Url *split_url(url)
const char	*url;
{
   struct Url	*components;

   char		*url_char;    /* current position in URL string */

   char		*word_start;    /* starting position of currently parsed url field */
   char		*name_start;    /* start of file name (component of path after last '/') */

   /* mode to parse next char in (<<8 to allow simple combining with "dispatch_char") */
   enum {
      PM_START=0x000,
      PM_SLASH=0x100,    /* '/' at beginning of URL */
      PM_PROTO=0x200,    /* inside protocoll specification */
      PM_HOST_START1=0x300,    /* first '/' of "//" introcucing host */
      PM_HOST_START2=0x400,
      PM_HOST=0x500,    /* inside host name */
      PM_PORT=0x600,    /* inside port number */
      PM_PATH=0x700,    /* inside directory/name string */
      PM_PARA=0x800,    /* inside CGI parameter list */
      PM_FRAG=0x900,    /* inside fragment identifier */
      PM_END=0xa00    /* '\0' ending URL string */
   } parse_mode=PM_START;

   char		escaped_url[3*strlen(url)+1];    /* enough for worst case of all characters needing escaping */


   DMSG(("parsing URL...\n"));

   components=malloc(sizeof(struct Url));
   if(components==NULL) {
      fprintf(stderr, "memory allocation error while parsing URL\n");
      exit(1);
   }
   /* set default values */
   {
      const struct Url	def={NULL, NULL, {NULL, PT_UNKNOWN}, NULL, 0, NULL, NULL, NULL, NULL, 0, 0};

      memcpy(components, &def, sizeof(def));
      components->proto.str=strdup("");    /* must be present when setting PT_INTERNAL due to error */
      if(components->proto.str==NULL) {
	 fprintf(stderr, "memory allocation error while splitting URL\n");
	 exit(1);
      }
   }

   /* Escape invalid chars in URL. */
   {
      char	*src_pos;
      char	*write_pos;

      for(src_pos=(char *)url, write_pos=escaped_url; *src_pos; ++src_pos) {    /* all chars in original URL */
	 const unsigned char	chr=*src_pos;

	 if(isascii(chr) && isgraph(chr) && chr!='\'')    /* normal char (see hacking-load.* for explanation!) */
	    write_pos+=snprintf(write_pos, sizeof(escaped_url) - (write_pos-escaped_url), "%c", chr);    /* just store char */
	 else    /* needs to be escaped */
	    write_pos+=snprintf(write_pos, sizeof(escaped_url) - (write_pos-escaped_url), "%%%.2x", (int)chr);    /* store %hh */
      }
      *write_pos=0;
   }

   for(name_start=word_start=url_char=escaped_url; parse_mode!=PM_END; ++url_char) {    /* all chars in URL string */
      int	dispatch_char;    /* input character after modification for dispatch */
      int	dispatch;    /* combined state variable used to determine action in main dispatcher */
      int	recycle=0;    /* need additional dispatch pass (sometimes necessary after switching parsing mode */

      DMSG(("%c", *url_char));

      if(strchr(":/?#", *url_char)==NULL)    /* normal character */
	 dispatch_char='$';    /* indicated by '$' (shared dispatch case) */
      else    /* characters which may have special function */
	 dispatch_char=*url_char;    /* handled each seperately in dispatcher */

      do {    /* while recycle (additional pass necessary) */
	 recycle=0;
	 dispatch=dispatch_char|parse_mode;    /* dispatch on new char and current state */
	 switch(dispatch) {

	    /* URL starts with normal char -> assume it is a protocol specification */
	    case '$'|PM_START:
	       parse_mode=PM_PROTO;

	    /* normal protocol string char -> go ahead */
	    case '$'|PM_PROTO:
	       break;

	    /* URL starts with '/' */
	    case '/'|PM_START:
	       name_start=url_char+1;    /* might be path start... (see '/'|PM_PATH below) */
	       parse_mode=PM_SLASH;
	       break;

	    /* URL start with host ("//") */
	    case '/'|PM_SLASH:
	       parse_mode=PM_HOST_START2;    /* host will follow */
	       recycle=1;
	       break;

	    /* URL starts with path (with single '/') -> skip to path parsing */
	    case '$'|PM_SLASH:
	    case '?'|PM_SLASH:
	    case '#'|PM_SLASH:
	    case '\0'|PM_SLASH:

	    /* URL starts neither with '/' nor with word -> also skip to path parsing (which will skip to next component) */
	    case '?'|PM_START:
	    case '#'|PM_START:
	    case '\0'|PM_START:

	    /* URL starts with word, but first word doesn't end with ':' => it was not the protocol specification -> also skip to path parsing */
	    case '/'|PM_PROTO:
	    case '?'|PM_PROTO:
	    case '#'|PM_PROTO:
	    case '\0'|PM_PROTO:
	       parse_mode=PM_PATH;
	       recycle=1;
	       break;

	    /* protocol specification end */
	    case ':'|PM_PROTO:
	       free(components->proto.str);
	       store_component(word_start, url_char, &components->proto.str);

	       /* test for known protocols */
	       if(components->proto.str!=NULL) {
		  if(strcasecmp(components->proto.str, "http")==0)
		     components->proto.type=PT_HTTP;
		  else if(strcasecmp(components->proto.str, "ftp")==0)
		     components->proto.type=PT_FTP;
		  else if(strcasecmp(components->proto.str, "file")==0)
		     components->proto.type=PT_FILE;
		  else {
		     fprintf(stderr, "unknown protocol %s\n", components->proto.str);
		     components->proto.type=PT_INTERNAL;
		     return components;
		  }
	       } else
		  components->proto.type=PT_UNKNOWN;

	       parse_mode=PM_HOST_START1;
	       break;

	    /* first '/' of "//" introducing host specification */
	    case '/'|PM_HOST_START1:
	       parse_mode=PM_HOST_START2;
	       break;

	    /* second '/' */
	    case '/'|PM_HOST_START2:
	       parse_mode=PM_HOST;
	       word_start=url_char+1;    /* host name starts after the "//" */
	       break;

	    /* host name char */
	    case '$'|PM_HOST:
	       break;

	    /* host name end */
	    case ':'|PM_HOST:
	    case '/'|PM_HOST:
	    case '?'|PM_HOST:
	    case '#'|PM_HOST:
	    case '\0'|PM_HOST:
	       store_component(word_start, url_char, &components->host);

	       parse_mode=PM_PORT;
	       if(*url_char==':')    /* port number follows host name */
		  word_start=url_char+1;
	       else {    /* something else than port number follows -> needs to take another look at the current char */
		  word_start=url_char;
		  recycle=1;
	       }
	       break;

	    /* port number char */
	    case '$'|PM_PORT:
	       break;

	    /* port number end */
	    case '/'|PM_PORT:
	    case '?'|PM_PORT:
	    case '#'|PM_PORT:
	    case '\0'|PM_PORT:
	       components->port=atoi(word_start);
	       if(components->port==0)    /* no port specified -> use default port */
		  components->port=80;

	       name_start=word_start=url_char;
	       parse_mode=PM_PATH;
	       recycle=1;    /* path start always needs additional pass (for setting "name_start" properly) */
	       break;

	    /* normal path char */
	    case '$'|PM_PATH:
	    case ':'|PM_PATH:
	       break;

	    /* '/' in path -> previous word wasn't file name, but the following one might be */
	    case '/'|PM_PATH:
	       name_start=url_char+1;    /* file name starts after last '/' */
	       break;

	    /* path end */
	    case '?'|PM_PATH:
	    case '#'|PM_PATH:
	    case '\0'|PM_PATH:
	       if((url_char-name_start==1 && !strncmp(name_start, ".", 1)) || (url_char-name_start==2 && !strncmp(name_start, "..", 2))) {    /* path ends in "." or ".." => this is *not* the file name, but a dir component! */
		  char	save_char=*url_char;
		  *url_char='/';    /* let it end with a slash like a dir should... */
		  store_component(word_start, url_char+1, &components->dir);    /* whole path, and the appended '/', are directory */
		  *url_char=save_char;    /* need to restore the original char at this position, as it is important for further processing */
	       } else {
		  store_component(word_start, name_start, &components->dir);    /* path ends at last '/' */
		  store_component(name_start, url_char, &components->name);    /* file name is path component after last '/' */
	       }

	       word_start=url_char+1;
	       parse_mode=PM_PARA;
	       if(*url_char!='?')
		  recycle=1;
	       break;

	    /* parameter list char */
	    case '$'|PM_PARA:
	    case ':'|PM_PARA:
	    case '/'|PM_PARA:    /* '/' and '?' treated as normal chars in parameter list */
	    case '?'|PM_PARA:
	       break;

	    /* parameter list end */
	    case '#'|PM_PARA:
	    case '\0'|PM_PARA:
	       store_component(word_start, url_char, &components->params);

	       word_start=url_char+1;
	       parse_mode=PM_FRAG;
	       if(*url_char!='#')
		  recycle=1;
	       break;

	    /* fragment identifier char */
	    case '$'|PM_FRAG:
	       break;

	    /* fragment identifier end == url end */
	    case '\0'|PM_FRAG:
	       store_component(word_start, url_char, &components->frag);

	       parse_mode=PM_END;
	       break;

	    default:
	       fprintf(stderr, "\nURL parse error (unexpected character)\n");
	       components->proto.type=PT_INTERNAL;
	       return components;
	 }    /* switch(dispatch) */
      } while(recycle);    /* additional dispatch pass after parsing mode change */
   }    /* for all chars in URL string */

#ifdef DEBUG
   if(cfg.debug) {
      debug_printf("\nextracted URL components:\n");
      dump_url(components);
   }
#endif

   return components;
}

/* free memory used by a parsed URL struct */
void free_url(url)
struct Url	*url;
{
   DMSG(("freeing url components...\n"));

   if(url->full_url!=NULL)
      free(url->full_url);
   if(url->proto.str!=NULL)
      free(url->proto.str);
   if(url->host!=NULL)
      free(url->host);
   if(url->dir!=NULL)
      free(url->dir);
   if(url->name!=NULL)
      free(url->name);
   if(url->params!=NULL)
      free(url->params);
   if(url->frag!=NULL)
      free(url->frag);

   free(url);
}

/* append string to end of other string, reallocating memory;
 * create new string, if str was NULL */
static void str_append(str, append)
char		**str;
const char	*append;
{
   if(*str==NULL) {    /* no string to append to -> create empty string */
      *str=strdup("");
      if(*str==NULL) {
	 fprintf(stderr, "memory allocation error while creating URL (in function str_append)\n");
	 exit(2);
      }
   }

   *str=realloc(*str, sizeof(char[strlen(*str)+strlen(append)+1]));
   if(*str==NULL) {
      fprintf(stderr, "memory allocation error while creating URL (in function str_append)\n");
      exit(2);
   }

   strcat(*str, append);
}

/*
 * Merges two URLs to create a new one.
 *
 * The "url_string" is split into components, and "base_url" is used to
 * complete the URL: Components are taken from "base_url" (or defaults, if no
 * "base_url" supplied) until the first field is specified in the main URL;
 * afterwards, all components are taken from the main URL (or defaults, if none
 * supplied). If the main URL path is relative, it is merged with the base
 * path.
 *
 * The "form" parameter gives an optional form data string which is to be
 * submitted with the URL. (Method GET.) NULL means nothing to submit.
 * Otherwise, the form data is attached as part of the merged URL.
 */

struct Url *merge_urls(base, url_string, form)
const struct Url	*base;    /* default values/base path */
const char		*url_string;    /* URL to be split and completed by "base_url" */
const char		*form;
{
   struct Url	*url;    /* merged URL */
   struct Url	*base_url;
   struct Url	*main_url;

   base_url=(struct Url *)base;
#ifdef DEBUG
   if(cfg.debug) {
      if(base_url!=NULL) {
	 debug_printf("Base URL components:\n");
	 dump_url(base_url);
      } else
	 debug_printf("No base URL.\n");
   }
#endif

   main_url=split_url(url_string);
   if(main_url->proto.type==PT_INTERNAL)    /* splitting failed */
      fprintf(stderr, "can't get absolute target URL\n");

   DMSG(("Merging...\n"));

   /* alloc struct for merged URL */
   url=malloc(sizeof(struct Url));
   if(url==NULL) {
      fprintf(stderr, "memory allocation error while parsing URL\n");
      exit(1);
   }

   /* merge with fallback values from "base_url" */

   if(main_url->proto.type!=PT_UNKNOWN) {    /* some protocol set in "main_url" -> take it */
      base_url=NULL;    /* take following components from "main_url" also (use defaults if not supplied) */
      url->proto.type=main_url->proto.type;
      url->proto.str=strdup(main_url->proto.str);
   } else {    /* no protocol in "main_url" */
      if(base_url!=NULL) {    /* has "base_url" -> take fallback from it */
	 url->proto.type=base_url->proto.type;
	 url->proto.str=strdup(base_url->proto.str);
      } else {    /* no "base_url" -> use default */
	 url->proto.type=PT_UNKNOWN;
	 url->proto.str=strdup("");
      }
   }

   if(base_url==NULL)    /* absolute URL (no "base_url" was given, or "main_url" contained a protocol specification, so "base_url" isn't used) */
      url->absolute=1;
   else
      url->absolute=0;

   if(main_url->host!=NULL) {
      base_url=NULL;
      url->host=strdup(main_url->host);
      url->port=main_url->port;    /* port always set with host */
   } else {
      if(base_url!=NULL) {    /* has "base_url" (and no components from "main_url" yet) */
	 url->host=strdup(base_url->host);
	 url->port=base_url->port;
      } else {
	 url->host=strdup("");
	 url->port=80;
      }
   }

   if(main_url->dir!=NULL) {
      if(main_url->dir[0]=='/' || base_url==NULL)    /* dir in "main_url" is absolute (or only one supplied) -> take it */
	 url->dir=strdup(main_url->dir);
      else {    /* dir in "main_url" is relative -> merge with "base_url" */
	 char	merged_dir[strlen(base_url->dir)+strlen(main_url->dir)+1];
	 char	*merge_part=main_url->dir;

	 strcpy(merged_dir, base_url->dir);

	 if(!strncmp(merge_part, "./", 2))    /* skip "./" */
	    merge_part+=2;

	 /* handle ".." */
	 for(; *merged_dir && !strncmp(merge_part, "../", 3); merge_part+=3) {    /* relative path starts with "..", and there is anything left in original path -> remove last component */
	    char	*trunc_pos;

	    /* find beginning of last component */
	    for(trunc_pos=strchr(merged_dir, 0)-1; trunc_pos>merged_dir && *(trunc_pos-1)!='/'; --trunc_pos);    /* look for '/' ending previous component, or string start (skip last character, which is the '/' terminating last component) */
	    *trunc_pos=0;
	 }    /* for all ".." */

	 strcat(merged_dir, merge_part);    /* concatenate the part of the relative path remaining after skipping all the "./" and "../" */
	 url->dir=strdup(merged_dir);
      }
      base_url=NULL;
   } else {
      if(base_url!=NULL)
	 url->dir=strdup(base_url->dir);
      else
	 url->dir=strdup(url->proto.type==PT_FILE || url->proto.type==PT_UNKNOWN?"":"/");
   }

   if(main_url->name!=NULL) {
      base_url=NULL;
      url->name=strdup(main_url->name);
   } else {
      if(base_url!=NULL)
	 url->name=strdup(base_url->name);
      else
	 url->name=strdup("");
   }


   if(form!=NULL) {    /* submit form data -> store in place of any other CGI paramters */
      base_url=NULL;    /* handle as if the data was passed as part of main URL (force loading new document even if URL otherwise identical; discard old fragment identifier) */
      url->params=strdup(form);
   } else if(main_url->params!=NULL) {    /* no form to submit, but paramters given in main URL -> take them */
      base_url=NULL;
      url->params=strdup(main_url->params);
   } else {
      if(base_url!=NULL)
	 url->params=strdup(base_url->params);
      else
	 url->params=strdup("");
   }

   if(base_url!=NULL)    /* same document as "base", only fragment identifier may differ (nothing taken from "main_url" yet) */
      url->local=1;
   else
      url->local=0;

   if(main_url->frag!=NULL) {
      base_url=NULL;
      url->frag=strdup(main_url->frag);
   } else {
      if(base_url!=NULL)
	 url->frag=strdup(base_url->frag);
      else
	 url->frag=strdup("");
   }

   if(url->proto.str==NULL
      || url->host==NULL
      || url->dir==NULL
      || url->name==NULL
      || url->params==NULL
      || url->frag==NULL
   ) {
      fprintf(stderr, "memory allocation error while creating URL (in function merge_urls)\n");
      exit(1);
   }

   free_url(main_url);

   /* create "full_url" string (and set "path" pointer) */
   {
      int	path_pos;

      DMSG(("creating new URL...\n"));
      url->full_url=NULL;

      if(url->proto.type==PT_HTTP || url->proto.type==PT_FTP) {
	 str_append(&url->full_url, url->proto.str);
	 str_append(&url->full_url, "://");
      }

      str_append(&url->full_url, url->host);

      if(url->port!=80) {
	 char	port_str[7];
	 snprintf(port_str, sizeof(port_str), ":%d", url->port);
	 str_append(&url->full_url, port_str);
      }

      path_pos=strlen(url->full_url);    /* path starts where "dir" and following components will be appended (present string end) */

      str_append(&url->full_url, url->dir);

      str_append(&url->full_url, url->name);

      if(*url->params) {
	 str_append(&url->full_url, "?");
	 str_append(&url->full_url, url->params);
      }

      url->path=url->full_url+path_pos;    /* store pointer to full path (can store pointer now, as full_url won't be moved anymore) */
   }

#ifdef DEBUG
   if(cfg.debug) {
      debug_printf("merged URL components:\n");
      dump_url(url);
   }
#endif

   return url;
}

/* Returns a string with %xx URL escape codes from the source string replaced by the respective characters. The result is newly allocated; the source remains untouched. */
char *url_unescape(escaped)
const char *escaped;
{
   char *rpos, *wpos, chr;
   char *result;

   result=malloc(strlen(escaped)+1);
   if(result==NULL) {
      fprintf(stderr, "memory allocation error while unescaping path (in function url_unescape)\n");
      exit(1);
   }

   wpos=result; rpos=(char *)escaped;
   do {
      if(*rpos=='%') {
	 int code;
	 sscanf(rpos+1, "%2x", &code);
	 chr=(unsigned char)code;
	 rpos+=3;
      } else
	 chr=*rpos++;
   } while((*wpos++=chr));

   return result;
}
