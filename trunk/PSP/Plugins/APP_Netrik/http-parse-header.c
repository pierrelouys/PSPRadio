/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/

/*
 * http-parse-header.c -- parse the HTTP response head
 *
 * (C) 2002 antrik
 *
 * This file contains the parse_header() function, which reads the complete
 * HTTP head from the resource, and extracts all headers.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "debug.h"
#include "http-parse-header.h"
#include "load.h"

#define NODATA 0x4000    /* don't interfere with normal chars or PM_-values */

static int sgetc(struct Resource *input);    /* get one character from the socket */
static void add_char(char **str, char chr);    /* append character to string */

#ifdef DEBUG
static void dump_headers(const struct Http_headers *headers);    /* dump all extracted headers */
#endif


#ifdef DEBUG
/* dump all extracted headers */
static void dump_headers(headers)
const struct Http_headers       *headers;
{
   int  cur_header;

   debug_printf("extracted headers:\n");

   for(cur_header=0; cur_header<headers->count; ++cur_header)
      debug_printf("%s: %s\n", headers->header[cur_header].name, headers->header[cur_header].value);

   debug_printf("\n");
}
#endif

/* read a character from socket */
static int sgetc(input)
struct Resource	*input;
{
   /* fill buffer */
   if(input->buf_ptr==input->buf_end) {    /* buf empty */
      load(input);    /* -> read next block */
      input->buf_ptr=input->buf;    /* reset read pointer */
      if(input->buf_ptr==input->buf_end)    /* still empty => eof */
	 return EOF;
   }

   /* read char from buffer */
   return (unsigned)*input->buf_ptr++;
}

/* append a character to a string */
static void add_char(str, chr)
char	**str;
char	chr;
{
   *str=realloc(*str, sizeof(char[strlen(*str)+2]));
   sprintf(strchr(*str, '\0'), "%c", chr);
}

void parse_header(res)
struct Resource	*res;
{
   struct Http_headers	*headers=&res->handle.http->headers;

   enum Parse_mode {
      PM_START=0x000,    /* before anything read */
      PM_STATUSLINE=0x100,    /* inside response status line */
      PM_FIRST_NEWLINE=0x200,    /* after end of status line (like PM_NEWLINE, except no folding possible) */
      PM_NAME=0x300,    /* inside header name */
      PM_SPACE=0x400,    /* ' ' after ':' at end of header name */
      PM_VALUE=0x500,    /* inside header value */
      PM_NEWLINE=0x600,    /* after '\n' */
      PM_FOLDING=0x700,    /* ' ' after newline (folded header value) */
      PM_END=0x800    /* whole header parsed */
   } parse_mode=PM_START;

   int	recycle;    /* have to do more handling with same input char */

   DMSG(("parsing HTTP headers...\n"));

   do {    /* while not PM_END */
      const int	in=sgetc(res);    /* input character from HTTP */
      int	dispatch_char;    /* character class used in switch */

#ifdef DEBUG
      if(cfg.debug)
	 fputc(in, stderr); fflush(stderr);
#endif

      if(isalpha(in))
	 dispatch_char='a';
      else if(in==' ' || in=='\t')
	 dispatch_char=' ';
      else if(isdigit(in))
	 dispatch_char='0';
      else if(strchr("-:\r\n", in))
	 dispatch_char=in;
      else if(in==EOF)
	 dispatch_char=NODATA;
      else
	 dispatch_char='*';

      do {    /* while recycle */
	 recycle=0;
	 switch(dispatch_char|parse_mode) {
	    /* EOF */
	    case NODATA|PM_START:
            case NODATA|PM_STATUSLINE:
            case NODATA|PM_NEWLINE:
            case NODATA|PM_NAME:
            case NODATA|PM_SPACE:
            case NODATA|PM_VALUE:
            case NODATA|PM_FOLDING:
	       if(!res->user_break) {
		  fprintf(stderr, parse_mode==PM_START ? "\nNo HTTP response.\n" : "\nUnexpected end of file while parsing HTTP header.\n");
		  res->type=RES_FAIL;
		  res->url->proto.type=PT_INTERNAL;    /* don't keep in history */
	       }
	       return;

	    case 'a'|PM_START:
	       parse_mode=PM_STATUSLINE;
	       /* fallthrough */

	    case ' '|PM_STATUSLINE:
	    case 'a'|PM_STATUSLINE:
	    case '0'|PM_STATUSLINE:
	    case '*'|PM_STATUSLINE:
	    case '\r'|PM_STATUSLINE:
	       break;    /* we don't handle status line... */

	    case '\n'|PM_STATUSLINE:
	       parse_mode=PM_FIRST_NEWLINE;
	       break;

	    /* beginning of header line */
	    case 'a'|PM_FIRST_NEWLINE:
	    case 'a'|PM_NEWLINE:
	       headers->header=realloc(headers->header, sizeof(struct Header[++headers->count]));
	       headers->header[headers->count-1].name=strdup("");
	       headers->header[headers->count-1].value=strdup("");

	       parse_mode=PM_NAME;
	       /* fallthrough */

	    case 'a'|PM_NAME:
	    case '0'|PM_NAME:
	    case '-'|PM_NAME:
	       add_char(&headers->header[headers->count-1].name, in);
	       break;
	       
	    case ':'|PM_NAME:
	       parse_mode=PM_SPACE;
	       break;

	    case ' '|PM_SPACE:
	       parse_mode=PM_VALUE;
	       break;

	    case 'a'|PM_VALUE:
	    case '0'|PM_VALUE:
	    case ' '|PM_VALUE:
	    case '-'|PM_VALUE:
	    case ':'|PM_VALUE:
	    case '*'|PM_VALUE:
	       add_char(&headers->header[headers->count-1].value, in);
	       break;

	    case '\r'|PM_VALUE:
	       break;

	    case '\n'|PM_VALUE:
	       parse_mode=PM_NEWLINE;
	       break;

	    /* ' ' at line start => folded header line */
	    case ' '|PM_NEWLINE:
	       add_char(&headers->header[headers->count-1].value, ' ');    /* add one space */
	       
	       parse_mode=PM_FOLDING;
	       break;

	    case ' '|PM_FOLDING:    /* skip more blank space at folding start */
	       break;

	    case 'a'|PM_FOLDING:
	    case '0'|PM_FOLDING:
	    case '-'|PM_FOLDING:
	    case ':'|PM_FOLDING:
	    case '*'|PM_FOLDING:
	       parse_mode=PM_VALUE;
	       recycle=1;
	       break;

	    /* empty line? */
	    case '\r'|PM_NEWLINE:
	    case '\r'|PM_FIRST_NEWLINE:
	       break;

	    case '\n'|PM_NEWLINE:
	    case '\n'|PM_FIRST_NEWLINE:
	       parse_mode=PM_END;
	       break;

	    /* errors handling */

	    case 'a'|PM_SPACE:
	    case '0'|PM_SPACE:
	    case '-'|PM_SPACE:
	    case ':'|PM_SPACE:
	    case '*'|PM_SPACE:
	       DMSG(("\n"));
	       fprintf(stderr, "HTTP header parsing error (missing ' ' after ':')\n");
	       parse_mode=PM_VALUE;
	       recycle=1;
	       break;

	    case '\n'|PM_NAME:
	    case '\n'|PM_SPACE:
	    case '\n'|PM_FOLDING:
	       DMSG(("\n"));
	       fprintf(stderr, "HTTP header parsing error (unexpected line end)\n");
	       parse_mode=PM_NEWLINE;
	       break;

	    default:
	       DMSG(("\n"));
	       fprintf(stderr, "HTTP header parsing error (unexpected character)\n");
	 }    /* switch */
      } while(recycle);
   } while(parse_mode!=PM_END);

#ifdef DEBUG
   if(cfg.debug)
      dump_headers(&res->handle.http->headers);
#endif
}
