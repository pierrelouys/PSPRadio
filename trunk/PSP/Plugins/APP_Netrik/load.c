/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * load.c -- load file from ressource
 *
 * (C) 2001, 2002 antrik
 *     2001, 2002 Patrice Neff
 *     2003 Witold Filipczyk
 *
 * Open a file, HTTP connection etc, and then read data blockwise.
 */
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"

#include "cfg.h"
#include "debug.h"
#include "forms.h"    /* for url_encode() */
#include "http.h"
#include "interrupt.h"
#include "load.h"

#ifdef DEBUG
   #define BUF_SIZE 16
#else
   #define BUF_SIZE 4096
#endif

static void init_wget(struct Resource *res);    /* prepare reading HTTP resource via wget */

/* prepare reading HTTP resource via wget;
 * calls wget in background, using its standard output as our input stream */
static void init_wget(res)
struct Resource	*res;
{
   char	cmd[strlen(res->url->full_url)+sizeof(WGET_CMD)];

   res->type=RES_PIPE;

   sprintf(cmd, WGET_CMD, res->url->full_url);    /* create command line for wget */
   res->handle.stream=popen(cmd, "r");    /* call in background, get output as pipe */
   if(res->handle.stream==NULL) {
      fprintf(stderr, "error executing wget\n");
      if(res->url->proto.type==PT_HTTP)
	 fprintf(stderr, "(try using --builtin-http)\n");
      exit(1);
   }
}

/*
 * build new URL, and prepare addressed resource for reading data
 * (create descriptor, open file/HTTP connection/wget pipe, alloc input buffer)
 */
struct Resource *init_load(base, url, form_data)
const struct Url	*base;    /* main URL is relative to this one */
const char		*url;    /* main URL, to be merged with "base" */
const struct Item	*form_data;    /* form item of a form to submit */
{
   struct Resource	*res;    /* opened ressource */
   struct Url		*base_url=(struct Url *)base;    /* avoid warnings about modifying "base" */
   char			*main_url=(char *)url;    /* avoid warnings about modifying "url" */
   struct Item		*form=(struct Item *)form_data;    /* avoid warnings about modifying "form_data" */

   int			redir;    /* number of HTTP redirections */
   struct Header	*redir_header;    /* HTTP header causing redirection, if any */

   hold_int();    /* enable SIGINT during whole loading process, but do not handle until read()/fread() called */

   for(redir_header=NULL, redir=0; !redir || (redir_header!=NULL && redir<=10); ++redir) {    /* repeat HTTP load for up to 10 redirections */

      if(redir_header) {
	 fprintf(stderr, "redirect to: %s\n", redir_header->value);

	 base_url=res->url;    /* redirect URL is relative to original target URL */
	 res->url=NULL;    /* remove original link (prevent URL string from being freed) */

	 main_url=redir_header->value;
	 redir_header->value=NULL;

	 form=NULL;    /* redirect is always a simple GET */

	 uninit_load(res);    /* redirect => don't need original connection anymore */

	 redir_header=NULL;    /* this redirection is accounted for */
      }    /* redirect */

      /* alloc resource descriptor */
      res=malloc(sizeof(struct Resource));
      if(res==NULL) {
	 fprintf(stderr, "memory allocation failure while opening resource\n");
	 exit(1);
      }

      res->user_break=0;

      /* alloc input buffer */
      res->buf=malloc(sizeof(char[BUF_SIZE]));
      if(res->buf==NULL) {
	 fprintf(stderr, "memory allocation failure while opening resource\n");
	 exit(1);
      }
      res->buf_ptr=res->buf_end=res->buf;    /* empty */

      /* open file/connection/pipe */

      if(main_url!=NULL) {    /* not already a split/merged URL (from history) */
	 char	*form_data=NULL;

	 if(form!=NULL && form->data.form->method==METHOD_GET) {
	    struct Data_string	form_data_string=url_encode(form);
	    if(form_data_string.size < 0) {    /* error in retrieving form data */
	       res->url->proto.type=PT_INTERNAL;
	       res->type=RES_FAIL;
	       return res;
	    }
	    form_data=form_data_string.data;    /* just get the data (URL encoded strings can be handled as normal C strings) */
	 }

	 res->url=merge_urls(base_url, main_url, form_data);
	 free(form_data);
	 if(res->url->proto.type==PT_INTERNAL) {    /* couldn't merge URLs */
	    res->type=RES_FAIL;
	    return res;
	 }

	 if(strcmp(main_url, "-")==0)    /* load from stdin */
	    res->url->proto.type=PT_INTERNAL;

	 if(redir && res->url->proto.type!=PT_HTTP) {
	    fprintf(stderr, "illegal redirect (not to HTTP resource)\n");
	    res->type=RES_FAIL;
	    return res;
	 }
      } else    /* already split URL */
	 res->url=base_url;

      switch(res->url->proto.type) {
	 case PT_INTERNAL:    /* use stdin */
	    if(!res->url->absolute) {
	       fprintf(stderr, "error: can't follow relative link from stdin\n");
	       exit(2);
	    }

	    /* reopen stdin; use for data input */
	    res->handle.stream=fdopen(dup(0), "r");
	    if(res->handle.stream==NULL) {
	       fprintf(stderr, "can't read stdin (reopen failed)\n");
	       exit(1);
	    }

	    /* use stderr as stdin */
	    if(dup2(2, 0)<0) {
	       fprintf(stderr, "can't reopen stderr for reading; don't know how to get interactive user input\n");
	       exit(1);
	    }

	    res->type=RES_STDIN;
	    break;

	 case PT_FILE:
	 case PT_UNKNOWN: {    /* assume local file first */
	    char	*path=url_unescape(res->url->path);
	    char	real_path[strlen(path)+sizeof(".bz2")];    /* real file name of (possibly compressed) local file */

	    if(res->url->proto.type==PT_UNKNOWN)
	       DMSG(("trying file: %s\n", path));

	    /* get "real_path" */
	    {
	       static const char	*ext[]={
		  "",
		  ".gz",
		  ".bz2",
		  NULL
	       };

	       int	try;

	       for(try=0; ext[try]!=NULL; ++try) {    /* all possible extensions */
		  snprintf(real_path, sizeof(real_path), "%s%s", path, ext[try]);
		  DMSG(("   trying %s...", real_path));
		  if(access(real_path, F_OK)==0) {    /* file exists -> keep this one */
		     DMSG(("OK\n"));
		     break;
		  } else
		     DMSG(("no\n"));
	       }
	       if(ext[try]==NULL)    /* none matched -> no file */
		  *real_path=0;

	       free(path);    /* no longer needed */
	    }    /* get "real_path" */

	    if(*real_path==0 && res->url->proto.type==PT_UNKNOWN && res->url->path[0]!='/') {    /* no local file of that name, but may be inclomplete HTTP URL */
	       char	x_url[sizeof("http://")+strlen(main_url)];    /* URL string with prepended protocol specification */

	       DMSG(("can't open local file\ntrying HTTP...\n"));
	       sprintf(x_url, "http://%s", main_url);

	       free_url(res->url);
	       res->url=merge_urls(base_url, x_url, NULL);    /* need to parse again (can't have form data -- if submitting form, always have some base URL!) */
	       if(res->url->proto.type==PT_INTERNAL) {    /* couldn't merge URLs */
		  res->type=RES_FAIL;
		  return res;
	       }
	       /* fallthrough (to PT_HTTP) */
	    } else {    /* must be local file */
	       res->handle.stream=NULL;    /* assume failed open (important if "*real_path==0"!) */
	       if(*real_path!=0) {    /* file exists */
/* Witold, antrik --> */
		  char	*ext=strrchr(real_path, '.');
		  char	*slash=strrchr(real_path, '/');

		  res->type=RES_FILE;    /* assume normal file */

		  if(ext!=NULL && (slash==NULL || slash<ext)) {    /* file name has extension => may be compressed */
		     const struct {
			char	*ext;
			char	*cmd;
		     } compress[]={
			{".gz", "gunzip -c '%s'"},
			{".bz2", "bunzip2 -c '%s'"},
			{NULL, NULL}
		     };

		     int	try;

		     DMSG(("has file extension (%s), maybe compressed\n", ext));
		     for(try=0; compress[try].ext!=NULL; ++try) {    /* all possible compression types */
			DMSG(("   testing for %s ...", compress[try].ext));
			if(!strcmp(compress[try].ext, ext)) {    /* match -> open using this compression */
			   char	cmd[strlen(compress[try].cmd)+strlen(real_path)];

			   /* security workaround: prevent escaping shell quote */
			   {
			      char *chr;
			      for(chr=real_path; *chr; ++chr)
				 if(*chr=='\'')
				    *chr=' ';
			   }

			   snprintf(cmd, sizeof(cmd), compress[try].cmd, real_path);

			   DMSG(("OK\nopening compressed file with command:\n%s\n", cmd));
			   res->handle.stream = popen(cmd, "r");
			   res->type=RES_PIPE;
			   break;    /* don't search further */
			} else    /* compress type doesn't match */
			   DMSG(("no\n"));
		     }    /* for all possible compression types */
		  }    /* has extension */
/* <-- Witold, antrik */

		  if(res->type==RES_FILE) {    /* not compressed -> open normal file */
		     res->handle.stream=fopen(real_path, "r");
		  }
	       }    /* file exists */

	       if(res->handle.stream==NULL) {
		  fprintf(stderr, "can't open file %s\n", main_url);
		  res->type=RES_FAIL;
		  res->url->proto.type=PT_INTERNAL;    /* don't keep in history */
		  return res;
	       }

	       if(!cfg.dump)
		  fprintf(stderr, "loading file: %s\n\n", real_path);
	       else
		  DMSG(("loading file: %s\n", real_path));

	       res->url->proto.type=PT_FILE;
	       break;
	    }    /* must be local file */
	 }    /* PT_FILE */

	 case PT_HTTP:
	    if(cfg.wget)
	       init_wget(res);
	    else {
	       int	cur_header;    /* currently scanned header */

	       http_init_load(res, (form!=NULL && form->data.form->method!=METHOD_GET) ? form : NULL);    /* open connection and load HTTP head */

	       if(redir) {    /* not original URLs (function arguments) -> don't keep */
		  free_url(base_url);
		  free(main_url);
	       }

	       /* check for redirect */
	       for(cur_header=0; cur_header<res->handle.http->headers.count; ++cur_header) {    /* all headers */
		  if(strcmp(res->handle.http->headers.header[cur_header].name, "Location")==0) {    /* found */
		     redir_header=&res->handle.http->headers.header[cur_header];
		     break;    /* don't search further */
		  }
	       }
	    }    /* not wget */
	    break;

	 case PT_FTP:
	    init_wget(res);
      }    /* switch proto.type */

   }    /* while redirect, up to 10 times */
   if(redir>10) {    /* still wants to redirect after last iteration */
      fprintf(stderr, "Too many redirections.\n");
      res->type=RES_FAIL;
      res->url->proto.type=PT_INTERNAL;    /* don't keep in history */
   }

   return res;
}

/* read data block from ressource into buffer */
void load(res)
struct Resource	*res;
{
   int chars_read;

   if(setjmp(label_int)) {    /* longjmp from SIGINT handler */
      res->user_break=1;
      chars_read=0;
   } else {    /* normal action */
      enable_int();    /* allow SIGINT during read()/fread() */

      if(res->type==RES_FAIL || res->user_break)    /* can't/shouldn't read anything */
	 chars_read=0;
/* Patrice, antrik --> */
      else if(res->type == RES_HTTP) {
	 chars_read=read(res->handle.http->socket, res->buf, BUF_SIZE);
	 if(chars_read<0) {
	    fprintf(stderr, "error while reading data from HTTP connection\n");
	    res->type=RES_FAIL;
	    chars_read=0;    /* return EOF */
	 }
      } else {    /* RES_FILE, RES_PIPE */
/* <-- Patrice, antrik */
	 chars_read=fread(res->buf, sizeof(char), BUF_SIZE, res->handle.stream);
	 if(ferror(res->handle.stream)) {
	    fprintf(stderr, "error while reading from file\n");
	    res->type=RES_FAIL;
	    chars_read=0;    /* return EOF */
	 }
      }    /* RES_FILE, RES_PIPE */
   }    /* normal action (not longjmp return) */

   hold_int();    /* put SIGINT on hold again (till next call of load()) */
      
   res->buf_end=res->buf+chars_read;
}

/* tidy up after loading a file;
 * closes stream and then frees memory allocated for the buffer and the resource
 * handle */
void uninit_load(res)
struct Resource *res;
{
   if(res->type==RES_PIPE) {
      if(pclose(res->handle.stream)!=0 && !res->user_break) {    /* care about wget error only if not user break (which always causes broken pipe...) */
	 fprintf(stderr, "error loading HTTP page\n");
	 exit(1);
      }
   } else if(res->type==RES_HTTP) {
      close(res->handle.http->socket);

      {
	 int	header;
	 for(header=0; header < res->handle.http->headers.count; ++header) {
	    free(res->handle.http->headers.header[header].name);
	    free(res->handle.http->headers.header[header].value);
	 }
	 free(res->handle.http->headers.header);
      }

      free(res->handle.http);
   } else if(res->type==RES_FILE || res->type==RES_STDIN)
      fclose(res->handle.stream);

   disable_int();    /* ignore ^C outside of file/HTTP loads */

   free(res->buf);
   free(res);
}
