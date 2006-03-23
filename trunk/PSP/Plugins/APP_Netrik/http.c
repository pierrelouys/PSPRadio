/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * http.c -- load files via http
 *
 * (C) 2001, 2002 Patrice Neff
 *     2001, 2002, 2003 antrik
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* network includes */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include "debug.h"
#include "forms.h"    /* to get form data */
#include "http.h"
#include "http-parse-header.h"
#include "interrupt.h"
#include "url.h"

static int get_http_socket(const struct Url *url, const struct Item *form);    /* open connection to the server */
static struct Data_string get_http_cmd(const struct Url *url, const struct Url *proxy, const struct Item *form);    /* construct a HTTP command */

/* Patrice, antrik --> */

/* construct HTTP GET command */
static struct Data_string get_http_cmd(url, proxy, form)
const struct Url	*url;
const struct Url	*proxy;
const struct Item	*form;    /* only for POST */
{
   struct Data_string cmd={NULL, 0};
   int len;

   struct Data_string form_data;
   char	content_len[15];


   if(form!=NULL) {    /* submit form data */
      form_data= form->data.form->method==METHOD_POST_MIMEENC ? mime_encode(form) : url_encode(form);
      if(form_data.size < 0)    /* failed to retrieve form data */
	 return form_data;    /* pass on error handle */

      snprintf(content_len, sizeof(content_len), "%d", form_data.size);
   }

   /* calculate length of HTTP request */
   len=strlen(form==NULL?"GET ":"POST ");
   len+= proxy!=NULL ? strlen(url->full_url) : strlen(url->path);
   len+=strlen(" HTTP/1.0\r\nHost: ");
   len+=strlen(url->host);
   len+=strlen("\r\nConnection: Close");
   if(form!=NULL) {
      len+= form->data.form->method==METHOD_POST_MIMEENC ? strlen("\r\nContent-Type: multipart/form-data; boundary="MIME_BOUNDRY"\r\nContent-length: ") : strlen("\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-length: ");
      len+=strlen(content_len);
   }
   len+=strlen("\r\n\r\n");
   if(form!=NULL)
      len+=form_data.size;

   /* build request */
   cmd.data=malloc(len+1);
   if(form==NULL)
      strcpy(cmd.data, "GET ");
   else
      strcpy(cmd.data, "POST ");
   if(proxy==NULL)
      strcat(cmd.data, url->path);
   else
      strcat(cmd.data, url->full_url);
   strcat(cmd.data, " HTTP/1.0\r\nHost: ");
   strcat(cmd.data, url->host);
   strcat(cmd.data, "\r\nConnection: Close");
   if(form!=NULL) {
      if(form->data.form->method==METHOD_POST_MIMEENC)
	 strcat(cmd.data, "\r\nContent-Type: multipart/form-data; boundary="MIME_BOUNDRY"\r\nContent-length: ");
      else
	 strcat(cmd.data, "\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-length: ");
      strcat(cmd.data, content_len);
   }
   strcat(cmd.data, "\r\n\r\n");

   cmd.size=strlen(cmd.data);
   if(form!=NULL) {
      memcpy(&cmd.data[cmd.size], form_data.data, form_data.size);
      cmd.size+=form_data.size;
      free(form_data.data);
   }

   return cmd;
}

/* <-- Patrice, antrik */

/*
 * Prepare http connection for reading data.
 * 
 * Creates an HTTP handle, opens a socket, connects to the server, sends HTTP
 * request (optionally submitting form data), loads and parses the response
 * HTTP header.
 */
void http_init_load(res, form)
struct Resource		*res;
const struct Item	*form;
{
   res->type=RES_HTTP;

   /* alloc http struct */
   res->handle.http=malloc(sizeof(struct Http_handle));
   if(res->handle.http==NULL) {
      fprintf(stderr, "memory allocation failure while opening resource\n");
      exit(1);
   }

   res->handle.http->headers.count=0;
   res->handle.http->headers.header=NULL;

   if(setjmp(label_int)) {    /* return from SIGINT handler */
      res->user_break=1;
   } else {    /* normal handling */
      enable_int();    /* enable interrupt while establishing HTTP connection */

      res->handle.http->socket=get_http_socket(res->url, form);
      if(res->handle.http->socket!=-1) {    /* could open HTTP connection */
	 parse_header(res);
      } else {    /* HTTP connection failed */
	 res->type=RES_FAIL;
	 res->url->proto.type=PT_INTERNAL;    /* don't keep in history */
      }
   }
   hold_int();    /* hold following interrupts till call of read()/fread() */
}

/* Patrice, antrik --> */

/* open a connection to the http server */
static int get_http_socket(url, form)
const struct Url	*url;
const struct Item	*form;
{
   struct hostent *hp;		/* host info of the target host */
   struct sockaddr_in sap;	/* connect information */
   int sock;			/* the socket */
   struct Data_string http_command;		/* http command to be sent to the server */
   char *proxy;
   struct Url *proxy_url;
   char *connect_host;    /* server connected to get the page (proxy when used, target server otherwise) */
   int connect_port;

   if(!cfg.dump)
      fprintf(stderr, "loading page: %s\n", url->full_url);

   /* get url information */
   if(cfg.proxy) {
      proxy=getenv("http_proxy");
      if(proxy==NULL)
	 proxy=getenv("HTTP_PROXY");
   } else
      proxy=NULL;

   if(proxy!=NULL) {
      DMSG(("parsing proxy URL...\n"));
      proxy_url=split_url(proxy);
      if(proxy_url->proto.type==PT_INTERNAL) {    /* splitting failed */
	 fprintf(stderr, "can't parse proxy URL\n");
	 return -1;
      }
   } else
      proxy_url=NULL;

   if(proxy==NULL) {
      connect_host=url->host;
      connect_port=url->port;
   } else {
      connect_host=proxy_url->host;
      connect_port=proxy_url->port;
   }
   if(connect_port==0)
      connect_port=80;

   /* start to fill the sockaddr struct */
   memset(&sap, 0, sizeof(sap));
   sap.sin_family = AF_INET;
   sap.sin_port = htons(connect_port);

   /* address to connect to */
   if(connect_host!=NULL) {
      if(!inet_aton(connect_host, &sap.sin_addr)) {
	 fprintf(stderr, "looking up IP address of %s...", connect_host); fflush(stderr);
         hp = gethostbyname(connect_host);
         if(hp==NULL) {
            fprintf(stderr, "unknown host\n");
	    return -1;
         }
         sap.sin_addr = *(struct in_addr*)hp->h_addr;
      }
   } else {
      sap.sin_addr.s_addr = htonl(INADDR_ANY);
   }

   /* connect to the server */
   fprintf(stderr, "\nconnecting to %s:%i...", inet_ntoa(sap.sin_addr), connect_port); fflush(stderr);
   sock = socket(AF_INET, SOCK_STREAM, 0);
   if(connect(sock, (struct sockaddr*)&sap, sizeof(sap))) {
      fprintf(stderr, "connect failed\n");
      return -1;
   }

   /* build the http commands */
   http_command=get_http_cmd(url, proxy_url, form);
   if(http_command.size < 0)    /* failed */
      return -1;
   DMSG(("\nsubmitting HTTP command:\n%s\n", http_command.data));

   /* send the http commands to fetch the URL */
   fprintf(stderr, "\nsending HTTP request..."); fflush(stderr);
   if(write(sock, http_command.data, http_command.size)==-1) {
      fprintf(stderr, "sending failed\n");
      return -1;
   }
   fprintf(stderr, "\nloading...\n\n");

   /* free memory */
   free(http_command.data);
   if(proxy_url!=NULL)
      free_url(proxy_url);

   return sock;
}

/* <-- Patrice, antrik */
