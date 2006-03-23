/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * parse-syntax.c -- this one parses the HTML file to a parse tree.
 *
 * (C) 2001 - 2004 antrik
 *
 * The generated parse tree contains all HTML elements, their attributes, and
 * all content. (Text inside the elements.)
 */
#include <ctype.h>
#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "cfg.h"
#include "screen.h"
#include "syntax.h"
#include "load.h"

static struct Element *add_element(void);    /* create new element node in parse tree */
static void buf_add_char(char chr);    /* append single character to text buffer */
static void insert_buf(char *ptr[]);    /* insert "text_buf" into parse tree at specified position */
static int html_error(struct Resource *input_res, enum Syntax_error old_level, enum Syntax_error level, const char *err_msg, const char *handling_msg, ...);    /* handle syntax error */

static struct Element	*cur_el;    /* element whose content is currently parsed */
static struct Element	*last_el;    /* last encountered sub-element (if currently parsing a tag, this is already the new element opened by that tag; if no children, this is identical to cur_el) */

static char		*text_buf;    /* buffer containing currently extracted content before inserted into parse tree (can also contain tag name, attribute name, or attribute value in respective parsing modes) */
static int		text_buf_len;    /* current length of "text_buf" (=position to append next char) */

/* create new element node in parse tree;
 * returns pointer to new node */
static struct Element *add_element(void)
{
   struct Element	*new_el;    /* newly created element node */

   /* alloc element */
   new_el=malloc(sizeof(struct Element));
   if(new_el==NULL) {
      fprintf(stderr, "memory allocation error while syntax parsing (in function add_element)\n");
      exit(1);
   }

   /* insert into tree */
   new_el->parent=cur_el;    /* my child, so I am parent... */
   new_el->list_next=NULL;    /* no elements yet behind the new one */
   if(cur_el!=NULL)    /* has parent (shouldn't happen only while creating root) -> insert (append) to existing tree */
      last_el->list_next=new_el;

   /* no other data yet */
   new_el->name.str=NULL;
   new_el->content=NULL;
   new_el->attr_count=0;
   new_el->attr=NULL;

   new_el->closed=0;

   return(new_el);
}

/* append a char to text_buf */
static void buf_add_char(chr)
char	chr;    /* char to append */
{
   text_buf=realloc(text_buf, ++text_buf_len);    /* resize buf */
   if(text_buf==NULL) {
      fprintf(stderr, "memory allocation error while syntax parsing (in function buf_add_char)\n");
      exit(1);
   }
   text_buf[text_buf_len-1]=chr;    /* append char */
}

/* insert text_buf into parse tree at specified position;
 * text_buf can be used for new text blocks afterwards */
static void insert_buf(ptr)
char	*ptr[];    /* string to be set to "text_buf" (by reference) */
{
   buf_add_char(0);    /* 0-terminate string */
   *ptr=text_buf;    /* insert */
   text_buf=NULL;    /* free "text_buf" pointer for further use */
   text_buf_len=0;
}

/*
 * Handle HTML syntax errors.
 *
 * Print error message; if "XHTML_ONLY" or if "CORRECT_HTML" mode, quit
 * immediately; otherwise, print how error will be handled, and return highest
 * error level up to now.
 */
static int html_error(
   struct Resource	*input_res,    /* resource descriptor of input page (may need to close input pipe) */
   enum Syntax_error	old_level,    /* highest error level up to now */
   enum Syntax_error	level,    /* new error level */
   const char		*err_msg,    /* what error occured */
   const char		*handling_msg,    /* how error will be handled */
   ...    /* additional paramters for err_msg or handling_msg */
) {
   va_list	arg_ptr;

   if(!cfg.debug) {    /* handle repeated errors */
      static int	ignored=0;
      static char	**ignore=NULL;

      if(err_msg==NULL) {    /* flush ignore list */
	 if(ignored)
	    fprintf(stderr, "\n(%d more errors suppressed)\n", ignored);

	 ignored=0;
	 free(ignore); ignore=NULL;

	 return old_level;
      } else {    /* normal operation -> check whether this error already occured */
	 int		msg;

	 if(ignore==NULL) {
	    ignore=malloc(sizeof(char *));
	    if(ignore==NULL) {
	       fprintf(stderr, "Memory allocation error while parsing syntax (in function html_error())\n");
	       exit(1);
	    }

	    ignore[0]=NULL;
	 }

	 for(msg=0; ignore[msg]!=NULL; ++msg)
	    if(!strcmp(ignore[msg], err_msg))
	       break;
	 if(ignore[msg]!=NULL) {    /* found in list */
	    ++ignored;
	    return old_level;
	 } else {    /* new error */
	    ignore=realloc(ignore, sizeof(char *[msg+2]));
	    if(ignore==NULL) {
	       fprintf(stderr, "Memory allocation error while parsing syntax (in function html_error())\n");
	       exit(1);
	    }

	    ignore[msg]=(char *)err_msg;
	    ignore[msg+1]=NULL;
	 }    /* new error */
      }    /* normal operation */
   }    /* handle repeated errors */

   /* print error message */
   DMSG(("\n"));
#ifdef DEBUG
   if(cfg.debug)
      set_color_raw(COLOR_WHITE|8);
#endif
   printf("HTML %s: ", level>=SE_WORKAROUND?"error":"warning");
#ifdef DEBUG
   reset_color_raw();
#endif

   va_start(arg_ptr, handling_msg);
   vprintf(err_msg, arg_ptr);
   va_end(arg_ptr);

   printf("\n");

   /* exit if necessary */
#ifndef XHTML_ONLY
   if(cfg.parser==FUSSY_HTML)    /* break on all errors */
#endif
   {
      if(input_res->type==RES_PIPE)
	 pclose(input_res->handle.stream);
      exit(2);
   }

   /* print handling message */
   va_start(arg_ptr, handling_msg);
   vprintf(handling_msg, arg_ptr);
   va_end(arg_ptr);

   printf("\n");
   fflush(stdout);

   /* return new err_level */
   if(level>old_level)
      return level;
   else
      return old_level;
}

/* parse syntax of (X)HTML file;
 * returns pointer to top of resulting syntax tree */
struct Element *parse_syntax(input, err_level)
struct Resource		*input;
enum Syntax_error	*err_level;    /* return: syntax errors encountered in page */
{
   /* mode to parse next char in (<<8 to allow simple combining with "dispatch_char") */
   enum {
      PM_CONTENT=0x0000,    /* content (text between/inside elements) */
      PM_BLANK=0x0100,    /* spaces/newlines in content mode */
      PM_PRE=0x0200,    /* inside <pre> element */

      PM_AMP=0x1000,    /* beginning of entity/character reference (after '&') */
      PM_REF=0x1100,    /* inside character entity reference */
      PM_REF_NUM_START=0x1200,    /* beginning of numerical character reference (after "&#") */
      PM_REF_NUM=0x1300,    /* inside numerical character reference */
      PM_REF_HEX=0x1400,    /* inside hexal character reference */

      PM_TAG_START=0x2000,    /* inside tag, before name (after '<') */
      PM_TAG_NAME=0x2100,    /* name of start tag (or single tag) */
      PM_END_TAG_START=0x2200,    /* after '/' */
      PM_END_TAG_NAME=0x2300,    /* name of end tag */
      PM_END_TAG_SPACE=0x2400,    /* (optional) space between name of end tag and '>' */
      PM_TAG=0x2500,    /* inside tag, neither name nor attribute (between attributes) */
      PM_SINGLE_TAG=0x2600,    /* end of empty element tag (after '/') */
      PM_ATTR_NAME=0x2700,
      PM_ATTR_NAME_END=0x2800,    /* (optional) space between attribute name and '=' */
      PM_ATTR_VALUE=0x2900,    /* after '=', but before actual data */
      PM_ATTR_DATA_QUOT=0x2a00,    /* inside attribute value string (after the starting '"') */
      PM_ATTR_DATA_APOS=0x2b00,    /* inside attribute value string (after the starting '\'') */
#ifndef XHTML_ONLY
      PM_ATTR_DATA_NOQUOTE=0x2c00,    /* not quoted attribute value */
#endif

      PM_EXCLAM=0x3000,    /* '!' after '<' (comment, DOCTYPE declaration or CDATA section) */
      PM_COMMENT_START=0x3100,    /* first '-' after "<!" */
      PM_COMMENT=0x3200,    /* inside comment */
      PM_COMMENT_END1=0x3300,    /* '-' inside comment (end?) */
      PM_COMMENT_END2=0x3400,    /* second '-' */
      PM_COMMENT_RESTART=0x3500,    /* another comment in this declaration ('-' after "--" ending first comment) */

      PM_DOCTYPE=0x4000,    /* inside DOCTYPE declaration */

      PM_CDATA_START=0x5000,    /* '[' after '!' */
      PM_CDATA=0x5100,    /* inside CDATA section */

      PM_INSTR=0x6000,    /* inside processing instruction */
      PM_INSTR_END=0x6100    /* '?' inside processing instruction (end?) */
   } parse_mode=PM_CONTENT,    /* as long as nothing special encountered, everything treated as normal text */
   prev_mode_tag=0,    /* save mode (text/blank) during tag processing */
   prev_mode_amp=0;    /* save mode (text/tag*) during reference processing */

   struct Element	*tree_top;

   int			in;    /* current character from input */
   int			have_data=0;    /* have already read someting from resource */

   int			cr_mode=0;    /* after '\r' (if a '\n' follows, it has to be discarded) */
   int			new_cr_mode;    /* "cr_mode" for next character */

   int			textarea=0;    /* flag: this PM_PRE is really a <textarea> */


   *err_level=SE_NO;    /* no errors yet */

   text_buf=NULL; text_buf_len=0;    /* nothing extracted yet */
   cur_el=NULL;    /* no elements yet */
   last_el=cur_el=tree_top=add_element();    /* create parse tree root */

   for(;;) {    /* process file char-wise */
      int	dispatch_char;    /* input character after modification for dispatch */
      int	dispatch;    /* combined state variable used to determine action in main dispatcher */
      int	recycle=0;    /* need additional dispatch pass (sometimes necessary after switching parsing mode */

      if(input->buf_ptr==input->buf_end) {    /* buf empty */
	 load(input);    /* -> read next block */
	 input->buf_ptr=input->buf;    /* reset read pointer */
      }

      if(input->buf_ptr<input->buf_end) {    /* buf nonempty */
	 in=*input->buf_ptr++;    /* read char */
	 have_data=1;
      } else {    /* still empty => eof */
	 if(!have_data && !input->user_break)
	    *err_level=SE_NODATA;
	 break;
      }

      DMSG(("%c", in));

      new_cr_mode=0;
      if(tolower(in)>='a' && tolower(in)<='z')    /* letter */
	 dispatch_char='a';    /* indicated by 'a' (shared dispatch case) */
      else if(in>='0' && in<='9')    /* digit */
	 dispatch_char='0';
      else if(strchr("_.:", in)!=NULL)    /* other chars allowed in names */
	 dispatch_char='~';
      else if(strchr(" \t\r\n\f", in)!=NULL) {    /* white space character */
	 if(cr_mode && in=='\n') {    /* '\n' after '\r' -> discard */
	    new_cr_mode=0;    /* discard only one '\n' */
	    continue;
	 } else if(in=='\r') {
	    in='\n';    /* treat '\r' like '\n' */
	    new_cr_mode=1;    /* if real '\n' follows, it has to be discarded */
	 }
	 dispatch_char=' ';
      } else if(!iscntrl(in) && strchr("&<>\";=/\'-![?#", in)==NULL)    /* other normal character (not control char and no html function in any situation) */
	 dispatch_char='$';
      else    /* characters which may have html function */
	 dispatch_char=in;    /* handled each seperately in dispatcher */
      cr_mode=new_cr_mode;

      do {    /* while recycle (additional pass needed) */
	 recycle=0;
	 dispatch=dispatch_char|parse_mode;    /* dispatch on new char and current state */
	 switch(dispatch) {

	    /* normal text char */
	    case 'a'|PM_CONTENT:
	    case '0'|PM_CONTENT:
	    case '~'|PM_CONTENT:
	    case '$'|PM_CONTENT:
	    case ';'|PM_CONTENT:
	    case '='|PM_CONTENT:
	    case '/'|PM_CONTENT:
	    case '>'|PM_CONTENT:
	    case '"'|PM_CONTENT:
	    case '\''|PM_CONTENT:
	    case '-'|PM_CONTENT:
	    case '!'|PM_CONTENT:
	    case '['|PM_CONTENT:
	    case '?'|PM_CONTENT:
	    case '#'|PM_CONTENT:
	       buf_add_char(in);
	       break;

	    /* blank: don't insert blanks immediately; only switch parse mode */
	    case ' '|PM_CONTENT:
	       parse_mode=PM_BLANK;
	       break;

	    /* successing blanks -> do nothing */
	    case ' '|PM_BLANK:
	       break;

	    /* normal char after blanks (any amount) -> insert single ' ' */
	    case 'a'|PM_BLANK:
	    case '0'|PM_BLANK:
	    case '~'|PM_BLANK:
	    case '$'|PM_BLANK:
	    case ';'|PM_BLANK:
	    case '='|PM_BLANK:
	    case '/'|PM_BLANK:
	    case '>'|PM_BLANK:
	    case '"'|PM_BLANK:
	    case '\''|PM_BLANK:
	    case '-'|PM_BLANK:
	    case '!'|PM_BLANK:
	    case '['|PM_BLANK:
	    case '?'|PM_BLANK:
	    case '#'|PM_BLANK:
	    case '&'|PM_BLANK:    /* reference after blank */
	       buf_add_char(' ');
	       parse_mode=PM_CONTENT;
	       recycle=1;    /* still need to process the new char */
	       break;

	    /* normal text char inside <pre> element */
	    case 'a'|PM_PRE:
	    case '0'|PM_PRE:
	    case '~'|PM_PRE:
	    case '$'|PM_PRE:
	    case ';'|PM_PRE:
	    case '='|PM_PRE:
	    case '/'|PM_PRE:
	    case '>'|PM_PRE:
	    case '"'|PM_PRE:
	    case '\''|PM_PRE:
	    case '-'|PM_PRE:
	    case '!'|PM_PRE:
	    case '['|PM_PRE:
	    case '?'|PM_PRE:
	    case '#'|PM_PRE:
	       buf_add_char(in);
	       break;

	    case ' '|PM_PRE:    /* whitespace in <pre> -> store as normal char */
	       if(in=='\n' || textarea)    /* newline (or anything in <textarea>) -> store directly */
		  buf_add_char(in);
	       else    /* other blank space -> store nbsp */
		  buf_add_char('\xa0');
	       break;

	    /* character references (escape sequences) */
	    {
	       int	amp_pos=0;    /* starting position of current reference in "text_buf" */

	       /* beginning of reference */
	       case '&'|PM_CONTENT:
	       case '&'|PM_PRE:
	       case '&'|PM_ATTR_DATA_QUOT:    /* references can occur in attribute value */
	       case '&'|PM_ATTR_DATA_APOS:
		  amp_pos=text_buf_len;    /* save starting position */
		  prev_mode_amp=parse_mode;    /* save current parsing mode */
		  parse_mode=PM_AMP;
		  buf_add_char(in);
		  break;

	       /* character entity reference start */
	       case 'a'|PM_AMP:
		  parse_mode=PM_REF;
		  /* fallthrough */

	       /* normal reference char */
	       case 'a'|PM_REF:
	       case '0'|PM_REF:
	       case '~'|PM_REF:
	       case '-'|PM_REF:
		  buf_add_char(in);
		  break;

	       /* numerical character reference */
	       case '#'|PM_AMP:
		  parse_mode=PM_REF_NUM_START;
		  buf_add_char(in);
		  break;

	       /* first digit */
	       case '0'|PM_REF_NUM_START:
		  parse_mode=PM_REF_NUM;
		  /* fallthrough */

	       /* following digits */
	       case '0'|PM_REF_NUM:
		  buf_add_char(in);
		  break;

	       /* letter after "&#" => probably hex reference start (or random garbage string) */
	       case 'a'|PM_REF_NUM_START:
		  parse_mode=PM_REF_HEX;
		  /* fallthrough */

	       /* following hex digits */
	       case 'a'|PM_REF_HEX:
	       case '0'|PM_REF_HEX:
		  buf_add_char(in);
		  break;

#ifndef XHTML_ONLY
	       /* not really a reference */
	       case '0'|PM_AMP:
	       case '~'|PM_AMP:
	       case '$'|PM_AMP:
	       case '&'|PM_AMP:
	       case '<'|PM_AMP:
	       case '>'|PM_AMP:
	       case '"'|PM_AMP:
	       case ' '|PM_AMP:
	       case ';'|PM_AMP:
	       case '='|PM_AMP:
	       case '/'|PM_AMP:
	       case '\''|PM_AMP:
	       case '-'|PM_AMP:
	       case '!'|PM_AMP:
	       case '['|PM_AMP:
	       case '?'|PM_AMP:
	       case '$'|PM_REF_NUM_START:
	       case '~'|PM_REF_NUM_START:
	       case '&'|PM_REF_NUM_START:
	       case '<'|PM_REF_NUM_START:
	       case '>'|PM_REF_NUM_START:
	       case '"'|PM_REF_NUM_START:
	       case ' '|PM_REF_NUM_START:
	       case ';'|PM_REF_NUM_START:
	       case '='|PM_REF_NUM_START:
	       case '/'|PM_REF_NUM_START:
	       case '\''|PM_REF_NUM_START:
	       case '-'|PM_REF_NUM_START:
	       case '!'|PM_REF_NUM_START:
	       case '['|PM_REF_NUM_START:
	       case '?'|PM_REF_NUM_START:
	       case '#'|PM_REF_NUM_START:
		  *err_level=html_error(input, *err_level, SE_DISCOURAGED, "Unescaped '&' character outside reference.", NULL);
		  parse_mode=prev_mode_amp;    /* abort */
		  recycle=1;
		  break;
#endif    /* !XHTML_ONLY */

	       /* end of reference */
	       case ';'|PM_REF:
	       case ';'|PM_REF_NUM:
	       case ';'|PM_REF_HEX:
#ifndef XHTML_ONLY
	       /* any non-identifier char can end a reference in SGML... */
	       case '$'|PM_REF:
	       case '&'|PM_REF:
	       case '<'|PM_REF:
	       case '>'|PM_REF:
	       case '"'|PM_REF:
	       case ' '|PM_REF:
	       case '='|PM_REF:
	       case '/'|PM_REF:
	       case '\''|PM_REF:
	       case '!'|PM_REF:
	       case '['|PM_REF:
	       case '?'|PM_REF:
	       case '#'|PM_REF:

	       case '&'|PM_REF_NUM:
	       case '<'|PM_REF_NUM:
	       case '>'|PM_REF_NUM:
	       case '"'|PM_REF_NUM:
	       case ' '|PM_REF_NUM:
	       case '='|PM_REF_NUM:
	       case '/'|PM_REF_NUM:
	       case '\''|PM_REF_NUM:
	       case '-'|PM_REF_NUM:
	       case '!'|PM_REF_NUM:
	       case '['|PM_REF_NUM:
	       case '?'|PM_REF_NUM:
	       case '#'|PM_REF_NUM:
	       case 'a'|PM_REF_NUM:    /* numerical reference ends when non-digits encountered */
	       case '~'|PM_REF_NUM:
	       case '$'|PM_REF_NUM:

	       case '&'|PM_REF_HEX:
	       case '<'|PM_REF_HEX:
	       case '>'|PM_REF_HEX:
	       case '"'|PM_REF_HEX:
	       case ' '|PM_REF_HEX:
	       case '='|PM_REF_HEX:
	       case '/'|PM_REF_HEX:
	       case '\''|PM_REF_HEX:
	       case '-'|PM_REF_HEX:
	       case '!'|PM_REF_HEX:
	       case '['|PM_REF_HEX:
	       case '?'|PM_REF_HEX:
	       case '#'|PM_REF_HEX:
	       case '~'|PM_REF_HEX:
	       case '$'|PM_REF_HEX:
#endif    /* !XHTML_ONLY */
	       {
		  int	i;
		  int	replace=0;    /* replace char for reference */

		  if(parse_mode==PM_REF) {
		     /* search for known entities in table */
		     for(i=0; *ref_table[i].str; ++i) {    /* until empty entry (table end) */
			int	len=strlen(ref_table[i].str);

			if(len==text_buf_len-(amp_pos+1) && !strncmp(ref_table[i].str, &text_buf[amp_pos+1], len)) {    /* found */
			   replace=(int)ref_table[i].replace;    /* -> take */
			   break;    /* don't search further */
			}
		     }
		  } else {    /* numerical */
		     buf_add_char('\0');    /* make sure string is terminated */
		     --text_buf_len;    /* ...but don't keep the '\0' when appending more text! */

		     /* extract number from numerical character reference */
		     if(text_buf[amp_pos+2]=='x')    /* hex number */
			replace=(int)strtol(&text_buf[amp_pos+3], NULL, 16);
		     else    /* decimal number */
			replace=(int)strtol(&text_buf[amp_pos+2], NULL, 10);

		     if(replace==0)    /* no number */
			*err_level=html_error(input, *err_level, SE_WORKAROUND, "Invalid reference. (Unescaped '&' character?)", "Using workaround.");
		  }    /* PM_REF_NUM */

		  /* replace reference by (single) replace char in text buffer */
		  if(replace>=32 && replace<256) {    /* valid replace found */
		     text_buf[amp_pos]=(char)replace;    /* -> store "replace" in place of the reference */
		     text_buf=realloc(text_buf, (text_buf_len=amp_pos+1));    /* adjust buffer length */
		     if(text_buf==NULL) {
			fprintf(stderr, "memory allocation error while syntax parsing (in function parse_syntax)\n");
			exit(1);
		     }
		  } else
		     replace=0;    /* drop invalid replace */

		  parse_mode=prev_mode_amp;    /* now back to previous mode... */
		  if(!(in==';' && replace))    /* ';' ending a valid reference is swallowed; in all other cases, handle ending char */
		     recycle=1;
		  break;
	       }    /* end of reference */

	    }    /* character/entity references */

	    /* tags */
	    {

	       /* start of tag */
	       case '<'|PM_BLANK:
	       case '<'|PM_CONTENT:
	       case '<'|PM_PRE:
		  prev_mode_tag=parse_mode;    /* save current mode */
		  parse_mode=PM_TAG_START;
		  break;

	       /* '!' at beginning => no tag (comment, DOCTYPE declaration, or CDATA section) */
	       case '!'|PM_TAG_START:
		  parse_mode=PM_EXCLAM;
		  break;

	       /* no '/' before tag name => start tag (or single tag; we'll see later...) */
	       case 'a'|PM_TAG_START:
		  last_el=add_element();    /* create new element node */
		  if(text_buf_len!=0)    /* have some pending content */
		     insert_buf(&last_el->content);    /* -> save to new node */
		  parse_mode=PM_TAG_NAME;
		  recycle=1;    /* still need to add the current char */
		  break;

	       /* '/' at beginning => end tag */
	       case '/'|PM_TAG_START:
		  if(text_buf_len!=0) {    /* have some pending content */
		     last_el=add_element();    /* -> create dummy tag */
		     insert_buf(&last_el->content);    /* and save to it */
		  }
		  parse_mode=PM_END_TAG_START;
		  break;

	       case 'a'|PM_END_TAG_START:
		  parse_mode=PM_END_TAG_NAME;
		  /* fallthrough */

	       /* tag name char */
	       case 'a'|PM_TAG_NAME:
	       case '0'|PM_TAG_NAME:
	       case '~'|PM_TAG_NAME:
	       case '-'|PM_TAG_NAME:
	       case 'a'|PM_END_TAG_NAME:
	       case '0'|PM_END_TAG_NAME:
	       case '~'|PM_END_TAG_NAME:
	       case '-'|PM_END_TAG_NAME:
		  buf_add_char(tolower(in));
		  break;

	       /* space after end tag name */
	       case ' '|PM_END_TAG_NAME:
		  parse_mode=PM_END_TAG_SPACE;
		  break;

	       /* more spaces after end tag name */
	       case ' '|PM_END_TAG_SPACE:
		  break;    /* ignore */

#ifndef XHTML_ONLY
	       /* ended by another tag start */
	       case '<'|PM_END_TAG_NAME:
	       case '<'|PM_END_TAG_SPACE:
		  *err_level=html_error(input, *err_level, SE_DISCOURAGED, "Unclosed tag.", NULL);
		  recycle=1;
		  /* fallthrough */
#endif

	       /* end of end tag -> go back to parent */
	       case '>'|PM_END_TAG_NAME:
	       case '>'|PM_END_TAG_SPACE:
		  buf_add_char(0);    /* 0-terminate text_buf (end tag name) */
#ifndef XHTML_ONLY
		  /* find matching start tag */
		  {
		     struct Element	*new_el;
		     int		leave_pre=0;

		     for(new_el=cur_el; new_el->parent!=NULL; new_el=new_el->parent) {    /* ascend to tree top */
			if(prev_mode_tag==PM_PRE && (strcmp(new_el->name.str, element_table[EL_PRE].name)==0 || strcmp(cur_el->name.str, element_table[EL_TEXTAREA].name)==0))    /* closing <pre> (or <textarea>) element */
			   leave_pre=1;

			if(new_el->name.str!=NULL && strcmp(text_buf, new_el->name.str)==0) {    /* matching start tag found */
			   cur_el=new_el->parent;    /* close the matching element, and all elements in between */
			   if(leave_pre)
			      prev_mode_tag=PM_CONTENT;
			   break;
			}
		     }
		     if(new_el->parent==NULL)    /* tree top reached => no matching start tag found */
			*err_level=html_error(input, *err_level, SE_WORKAROUND, "No matching start tag found.", "Ignoring.");
		  }
#else    /* XHTML_ONLY */
		  /* test whether end tag matches start tag */
		  if(strcmp(text_buf, cur_el->name.str)!=0)
#ifdef DEBUG
		     if(cfg.debug)
			html_error(input, 0, 0, "End tag doesn't match. (Should be: %s.)", NULL, cur_el->name.str);
		     else
#endif
			html_error(input, 0, 0, "End tag doesn't match.", NULL);

		  if(prev_mode_tag==PM_PRE && strcmp(cur_el->name.str, element_table[EL_PRE].name)==0)    /* closing <pre> element */
		     prev_mode_tag=PM_CONTENT;
		  cur_el=cur_el->parent;    /* end tag closes current element */
#endif    /* XHTML_ONLY */

		  /* discard end tag name */
		  text_buf_len=0;
		  free(text_buf);
		  text_buf=NULL;

		  parse_mode=prev_mode_tag;    /* back to normal mode */
		  break;

	       /* start tag name ended by tag end or single tag indicator */
	       case '>'|PM_TAG_NAME:    /* ends immediatly after name */
	       case '/'|PM_TAG_NAME:    /* single tag */
#ifndef XHTML_ONLY
	       case '<'|PM_TAG_NAME:    /* ended by another tag start */
#endif
		  recycle=1;    /* will have to process tag end/single tag indicator also */
		  /* fallthrough */

	       /* end of start tag name */
	       case ' '|PM_TAG_NAME:
		  insert_buf(&last_el->name.str);    /* set the new element's name to the name just extracted */
		  parse_mode=PM_TAG;
		  break;

	       /* additional text in start tag => attribute */
	       case 'a'|PM_TAG:
		  parse_mode=PM_ATTR_NAME;
		  recycle=1;    /* still need to add the current char */
		  break;

	       /* attribute name char */
	       case 'a'|PM_ATTR_NAME:
	       case '0'|PM_ATTR_NAME:
	       case '~'|PM_ATTR_NAME:
	       case '-'|PM_ATTR_NAME:
		  buf_add_char(tolower(in));
		  break;

	       /* attribute name end */
	       case ' '|PM_ATTR_NAME:
	       case '='|PM_ATTR_NAME:
#ifndef XHTML_ONLY
	       case '>'|PM_ATTR_NAME:    /* tag ends after attribute name (attribute without value) */
	       case '<'|PM_ATTR_NAME:    /* ended by another tag start */
#endif
		  /* store attribute name */
		  last_el->attr=realloc(last_el->attr, (++last_el->attr_count)*sizeof(struct Attr));    /* resize attribute array to hold new attribute */
		  if(last_el->attr==NULL) {
		     fprintf(stderr, "memory allocation error while syntax parsing (in function parse_syntax)\n");
		     exit(1);
		  }
		  insert_buf(&last_el->attr[last_el->attr_count-1].name.str);    /* set attribute name to the name just extracted */
		  last_el->attr[last_el->attr_count-1].value.str=NULL;    /* make sure we have a definend (non-)value, if no real value will be stored for some reason */

		  parse_mode=PM_ATTR_NAME_END;
		  recycle=1;    /* may need additional processing */
		  break;

	       /* space after attribute name */
	       case ' '|PM_ATTR_NAME_END:
		  break;    /* ignore */

	       /* '=' between attribute name and value */
	       case '='|PM_ATTR_NAME_END:
		  parse_mode=PM_ATTR_VALUE;    /* attribute value string now has to follow */
		  break;

	       /* space between '=' and attribute value */
	       case ' '|PM_ATTR_VALUE:
		  break;    /* ignore */

	       /* starting '"' of attribute value string => now real data begins */
	       case '"'|PM_ATTR_VALUE:
		  parse_mode=PM_ATTR_DATA_QUOT;
		  break;

	       /* starting '\'' of attribute value string => now real data begins */
	       case '\''|PM_ATTR_VALUE:
		  parse_mode=PM_ATTR_DATA_APOS;
		  break;

#ifndef XHTML_ONLY
	       /* unquoted attribute value starting with illegal char */
	       case '$'|PM_ATTR_VALUE:
	       case '='|PM_ATTR_VALUE:
	       case '/'|PM_ATTR_VALUE:
	       case ';'|PM_ATTR_VALUE:
	       case '!'|PM_ATTR_VALUE:
	       case '['|PM_ATTR_VALUE:
	       case '?'|PM_ATTR_VALUE:
	       case '#'|PM_ATTR_VALUE:
	       case '&'|PM_ATTR_VALUE:    /* probably not meant as a reference */
		  *err_level=html_error(input, *err_level, SE_WORKAROUND, "Illegal character in unquoted attribute value.", "Using workaround.");
		  /* fallthrough */

	       /* not quoted attribute value */
	       case 'a'|PM_ATTR_VALUE:
	       case '0'|PM_ATTR_VALUE:
	       case '~'|PM_ATTR_VALUE:
	       case '-'|PM_ATTR_VALUE:
		  parse_mode=PM_ATTR_DATA_NOQUOTE;
		  recycle=1;
		  break;

	       case '"'|PM_ATTR_DATA_NOQUOTE:
	       case '\''|PM_ATTR_DATA_NOQUOTE:
		  *err_level=html_error(input, *err_level, SE_WORKAROUND, "Non-matching quote in attribute value.", "Using workaround.");
		  break;    /* ignore */

	       /* illegal unquoted attribute value char */
	       case '$'|PM_ATTR_DATA_NOQUOTE:
	       case '='|PM_ATTR_DATA_NOQUOTE:
	       case '/'|PM_ATTR_DATA_NOQUOTE:
	       case ';'|PM_ATTR_DATA_NOQUOTE:
	       case '!'|PM_ATTR_DATA_NOQUOTE:
	       case '['|PM_ATTR_DATA_NOQUOTE:
	       case '?'|PM_ATTR_DATA_NOQUOTE:
	       case '#'|PM_ATTR_DATA_NOQUOTE:
	       case '&'|PM_ATTR_DATA_NOQUOTE:    /* probably not meant as a reference */
		  *err_level=html_error(input, *err_level, SE_WORKAROUND, "Illegal character in unquoted attribute value.", "Using workaround.");
		  /* fallthrough */

	       /* unquoted attribute value */
	       case 'a'|PM_ATTR_DATA_NOQUOTE:
	       case '0'|PM_ATTR_DATA_NOQUOTE:
	       case '~'|PM_ATTR_DATA_NOQUOTE:
	       case '-'|PM_ATTR_DATA_NOQUOTE:
#endif    /* !XHTML_ONLY */

	       /* attribute value char */
	       case 'a'|PM_ATTR_DATA_QUOT:
	       case '0'|PM_ATTR_DATA_QUOT:
	       case '~'|PM_ATTR_DATA_QUOT:
	       case '$'|PM_ATTR_DATA_QUOT:
	       case ' '|PM_ATTR_DATA_QUOT:
	       case '>'|PM_ATTR_DATA_QUOT:
	       case '='|PM_ATTR_DATA_QUOT:
	       case '/'|PM_ATTR_DATA_QUOT:
	       case ';'|PM_ATTR_DATA_QUOT:
	       case '\''|PM_ATTR_DATA_QUOT:
	       case '-'|PM_ATTR_DATA_QUOT:
	       case '!'|PM_ATTR_DATA_QUOT:
	       case '['|PM_ATTR_DATA_QUOT:
	       case '?'|PM_ATTR_DATA_QUOT:
	       case '#'|PM_ATTR_DATA_QUOT:

	       case 'a'|PM_ATTR_DATA_APOS:
	       case '0'|PM_ATTR_DATA_APOS:
	       case '~'|PM_ATTR_DATA_APOS:
	       case '$'|PM_ATTR_DATA_APOS:
	       case ' '|PM_ATTR_DATA_APOS:
	       case '>'|PM_ATTR_DATA_APOS:
	       case '='|PM_ATTR_DATA_APOS:
	       case '/'|PM_ATTR_DATA_APOS:
	       case ';'|PM_ATTR_DATA_APOS:
	       case '"'|PM_ATTR_DATA_APOS:
	       case '-'|PM_ATTR_DATA_APOS:
	       case '!'|PM_ATTR_DATA_APOS:
	       case '['|PM_ATTR_DATA_APOS:
	       case '?'|PM_ATTR_DATA_APOS:
	       case '#'|PM_ATTR_DATA_APOS:

#ifndef XHTML_ONLY
	       case '<'|PM_ATTR_DATA_QUOT:
	       case '<'|PM_ATTR_DATA_APOS:
#endif
		  buf_add_char(in);
		  break;

#ifndef XHTML_ONLY
	       /* attribute without value -> store empty string */
	       case '>'|PM_ATTR_NAME_END:    /* tag end */
	       case '<'|PM_ATTR_NAME_END:    /* ended by next tag start */
	       case 'a'|PM_ATTR_NAME_END:    /* next attribute */
		  /* fallthrough */

	       /* attribute value string ending with tag end */
	       case '>'|PM_ATTR_DATA_NOQUOTE:
	       case '<'|PM_ATTR_DATA_NOQUOTE:    /* ended by next tag start */
		  recycle=1;    /* will have to process tag end or next attribute also */
		  /* fallthrough */
#endif

	       /* end of attribute string */
	       case '"'|PM_ATTR_DATA_QUOT:
	       case '\''|PM_ATTR_DATA_APOS:
#ifndef XHTML_ONLY
	       case ' '|PM_ATTR_DATA_NOQUOTE:    /* not quoted attribute value ends with blank */
#endif
		  insert_buf(&last_el->attr[last_el->attr_count-1].value.str);    /* set attribute value to the value just extracted */
		  parse_mode=PM_TAG;
		  break;

	       /* ' ' between attributes -> nothing to be done */
	       case ' '|PM_TAG:
		  break;

#ifndef XHTML_ONLY
	       /* ended by another tag start */
	       case '<'|PM_TAG:
		  *err_level=html_error(input, *err_level, SE_DISCOURAGED, "Unclosed tag.", NULL);
		  recycle=1;
		  /* fallthrough */
#endif

	       /* end of start tag */
	       case '>'|PM_TAG:
		  cur_el=last_el;    /* start tag, so now we will parse the element's content */
		  if(strcmp(cur_el->name.str, element_table[EL_PRE].name)==0)    /* entering <pre> element */
		     parse_mode=PM_PRE;
		  else if(strcmp(cur_el->name.str, element_table[EL_TEXTAREA].name)==0) {    /* entering <textarea> element */
		     parse_mode=PM_PRE;    /* handle (nearly) like <pre> */
		     textarea=1;    /* nearly... */
		  } else {
		     parse_mode=prev_mode_tag;    /* back to normal mode */
		     textarea=0;
		  }
		  break;

	       /* '/' as parameter => single tag */
	       case '/'|PM_TAG:
		  parse_mode=PM_SINGLE_TAG;
		  break;

	       /* space between '/' and '>' in single tag */
	       case ' '|PM_SINGLE_TAG:
		  break;    /* skip */

	       /* end of single tag -> nothing more to be done */
	       case '>'|PM_SINGLE_TAG:
		  parse_mode=prev_mode_tag;    /* back to normal mode */
		  break;


#ifndef XHTML_ONLY
	       /* not really a tag */
	       case '0'|PM_TAG_START:
	       case '~'|PM_TAG_START:
	       case '$'|PM_TAG_START:
	       case ';'|PM_TAG_START:
	       case '='|PM_TAG_START:
	       case '"'|PM_TAG_START:
	       case '\''|PM_TAG_START:
	       case ' '|PM_TAG_START:
	       case '<'|PM_TAG_START:
	       case '&'|PM_TAG_START:
	       case '-'|PM_TAG_START:
	       case '['|PM_TAG_START:
	       case '#'|PM_TAG_START:
		  *err_level=html_error(input, *err_level, SE_DISCOURAGED, "Unescaped '<' character outside tag.", NULL);
		  buf_add_char('<');    /* store the preceeding '<' literally */
		  parse_mode=prev_mode_tag;    /* back to normal mode (don't parse as tag) */
		  recycle=1;    /* process current char as normal text */
		  break;

	       /* empty tag */
	       case '>'|PM_TAG_START:
		  *err_level=html_error(input, *err_level, SE_UNIMPLEMENTED, "Empty tag.", "Treating as content.");
		  buf_add_char('<'); buf_add_char('>');    /* store the "<>" literally */
		  parse_mode=prev_mode_tag;    /* back to normal mode (don't parse as tag) */
		  break;

	       /* not really an (end) tag */
	       case '0'|PM_END_TAG_START:
	       case '~'|PM_END_TAG_START:
	       case '$'|PM_END_TAG_START:
	       case ';'|PM_END_TAG_START:
	       case '='|PM_END_TAG_START:
	       case '"'|PM_END_TAG_START:
	       case '\''|PM_END_TAG_START:
	       case ' '|PM_END_TAG_START:
	       case '<'|PM_END_TAG_START:
	       case '&'|PM_END_TAG_START:
	       case '-'|PM_END_TAG_START:
	       case '['|PM_END_TAG_START:
	       case '#'|PM_END_TAG_START:
	       case '/'|PM_END_TAG_START:
	       case '?'|PM_END_TAG_START:
	       case '!'|PM_END_TAG_START:
		  *err_level=html_error(input, *err_level, SE_DISCOURAGED, "Unescaped '<' character outside tag.", NULL);
		  buf_add_char('<'); buf_add_char('/');    /* store the preceeding "</" literally */
		  parse_mode=prev_mode_tag;    /* back to normal mode (don't parse as tag) */
		  recycle=1;    /* process current char as normal text */
		  break;

	       /* empty end tag */
	       case '>'|PM_END_TAG_START:
		  *err_level=html_error(input, *err_level, SE_UNIMPLEMENTED, "Empty end tag.", "Treating as content.");
		  buf_add_char('<'); buf_add_char('/');    /* store "</" literally */
		  parse_mode=prev_mode_tag;    /* back to normal mode (don't parse as tag) */
		  recycle=1;    /* process current char as normal text */
		  break;

	       /* normal content after '/' (single tag) => net mode */
	       case 'a'|PM_SINGLE_TAG:
	       case '0'|PM_SINGLE_TAG:
	       case '~'|PM_SINGLE_TAG:
	       case '$'|PM_SINGLE_TAG:
	       case ';'|PM_SINGLE_TAG:
	       case '='|PM_SINGLE_TAG:
	       case '/'|PM_SINGLE_TAG:
	       case '"'|PM_SINGLE_TAG:
	       case '\''|PM_SINGLE_TAG:
	       case '-'|PM_SINGLE_TAG:
	       case '!'|PM_SINGLE_TAG:
	       case '['|PM_SINGLE_TAG:
	       case '?'|PM_SINGLE_TAG:
	       case '#'|PM_SINGLE_TAG:
	       case '<'|PM_SINGLE_TAG:
		  *err_level=html_error(input, *err_level, SE_UNIMPLEMENTED, "Net-enabling start tag.", "Treating as single tag.");
		  parse_mode=prev_mode_tag;    /* back to normal mode */
		  recycle=1;
		  break;
#endif    /* no XHTML_ONLY */
	    }    /* tags */

	    /* comments */
	    {
	       int	dash_count=0;    /* current number of consecutive dashes while parsing comment */
	       int	broken=0;    /* syntax error detected in this comment */

	       /* first '-' of comment start */
	       case '-'|PM_EXCLAM:
		  dash_count=-1;    /* prevent warning if '>' follows first '--' */
		  broken=0;
		  parse_mode=PM_COMMENT_START;
		  break;

	       /* second '-' of comment start */
	       case '-'|PM_COMMENT_START:
		  ++dash_count;
		  parse_mode=PM_COMMENT;
		  break;

	       /* '>' inside comment */
	       case '>'|PM_COMMENT:
		  if(dash_count>=2) {    /* after "--" (e.g "<!--comment-- -->") or "<--- comment --->" => probably meant as comment end */
		     if(broken) {    /* there were real errors in this comment already -> it's safer to quit here (needn't be correct) */
			parse_mode=PM_COMMENT_END2;
			recycle=1;
			break;
		     } else {    /* this comment was otherwise OK -> only warn, but treat strictly SGML-wise */
			DMSG(("\n"));
			*err_level=html_error(input, *err_level, SE_DISCOURAGED, "Suspicious comment syntax. (\"-->\" not terminating comment; too much '-' characters?)", NULL);
		     }
		  }
		  /* fallthrough */

	       /* normal comment char */
	       case 'a'|PM_COMMENT:
	       case '0'|PM_COMMENT:
	       case '~'|PM_COMMENT:
	       case '$'|PM_COMMENT:
	       case ';'|PM_COMMENT:
	       case '='|PM_COMMENT:
	       case '/'|PM_COMMENT:
	       case '"'|PM_COMMENT:
	       case '\''|PM_COMMENT:
	       case ' '|PM_COMMENT:
	       case '<'|PM_COMMENT:
	       case '&'|PM_COMMENT:
	       case '!'|PM_COMMENT:
	       case '['|PM_COMMENT:
	       case '?'|PM_COMMENT:
	       case '#'|PM_COMMENT:
		  dash_count=0;
		  break;    /* ignore */

	       /* first '-' of comment end */
	       case '-'|PM_COMMENT:
		  ++dash_count;
		  parse_mode=PM_COMMENT_END1;
		  break;

	       /* comment char after single '-' */
	       case 'a'|PM_COMMENT_END1:
	       case '0'|PM_COMMENT_END1:
	       case '~'|PM_COMMENT_END1:
	       case '$'|PM_COMMENT_END1:
	       case ';'|PM_COMMENT_END1:
	       case '='|PM_COMMENT_END1:
	       case '/'|PM_COMMENT_END1:
	       case '"'|PM_COMMENT_END1:
	       case '\''|PM_COMMENT_END1:
	       case ' '|PM_COMMENT_END1:
	       case '<'|PM_COMMENT_END1:
	       case '&'|PM_COMMENT_END1:
	       case '!'|PM_COMMENT_END1:
	       case '['|PM_COMMENT_END1:
	       case '?'|PM_COMMENT_END1:
	       case '#'|PM_COMMENT_END1:
		  dash_count=0;
		  parse_mode=PM_COMMENT;    /* back to comment mode */
		  break;

	       /* '>' after single '-' */
	       case '>'|PM_COMMENT_END1:
		  parse_mode=PM_COMMENT;    /* back to comment mode */
		  recycle=1;    /* may need to issue warning */
		  break;

	       /* second '-' of comment end */
	       case '-'|PM_COMMENT_END1:
		  ++dash_count;
		  parse_mode=PM_COMMENT_END2;
		  break;

#ifndef XHTML_ONLY
	       /* blank space between "--" and '>' (or another "--") */
	       case ' '|PM_COMMENT_END2:
		  *err_level=html_error(input, *err_level, SE_DISCOURAGED, "Suspicious comment syntax. (' ' after terminating \"--\")", NULL);
		  dash_count=0;
		  break;

	       /* another comment in same declaration */
	       case '-'|PM_COMMENT_END2:
		  *err_level=html_error(input, *err_level, SE_DISCOURAGED, "Suspicious comment syntax. (Too much '-' characters?)", NULL);
		  ++dash_count;
		  parse_mode=PM_COMMENT_RESTART;
		  break;

	       /* second '-' of comment restart */
	       case '-'|PM_COMMENT_RESTART:
		  ++dash_count;
		  parse_mode=PM_COMMENT;
		  break;

	       /* normal char after comment end */
	       case 'a'|PM_COMMENT_END2:
	       case '0'|PM_COMMENT_END2:
	       case '~'|PM_COMMENT_END2:
	       case '$'|PM_COMMENT_END2:
	       case ';'|PM_COMMENT_END2:
	       case '='|PM_COMMENT_END2:
	       case '/'|PM_COMMENT_END2:
	       case '"'|PM_COMMENT_END2:
	       case '\''|PM_COMMENT_END2:
	       case '<'|PM_COMMENT_END2:
	       case '&'|PM_COMMENT_END2:
	       case '!'|PM_COMMENT_END2:
	       case '['|PM_COMMENT_END2:
	       case '?'|PM_COMMENT_END2:
	       case '#'|PM_COMMENT_END2:

	       case 'a'|PM_COMMENT_RESTART:
	       case '0'|PM_COMMENT_RESTART:
	       case '~'|PM_COMMENT_RESTART:
	       case '$'|PM_COMMENT_RESTART:
	       case ';'|PM_COMMENT_RESTART:
	       case '='|PM_COMMENT_RESTART:
	       case '/'|PM_COMMENT_RESTART:
	       case '"'|PM_COMMENT_RESTART:
	       case '\''|PM_COMMENT_RESTART:
	       case '<'|PM_COMMENT_RESTART:
	       case '&'|PM_COMMENT_RESTART:
	       case '!'|PM_COMMENT_RESTART:
	       case '['|PM_COMMENT_RESTART:
	       case '?'|PM_COMMENT_RESTART:
	       case '#'|PM_COMMENT_RESTART:

		  *err_level=html_error(input, *err_level, SE_CRITICAL, "Broken comment. (Text after terminating \"--\".)", "Trying workaround.");
		  dash_count=0;
		  broken=1;
		  parse_mode=PM_COMMENT;    /* back to comment mode (ignore "--") */
		  break;

	       /* "->" after comment end (probably "--->" or so) */
	       case '>'|PM_COMMENT_RESTART:
		  *err_level=html_error(input, *err_level, SE_CRITICAL, "Broken comment. (Too much '-' characters.)", "Trying workaround.");
		  broken=1;
		  parse_mode=PM_COMMENT;
		  recycle=1;    /* may have to abort */
		  break;

	       /* "<!>" -> immediately end comment */
	       case '>'|PM_EXCLAM:
		  *err_level=html_error(input, *err_level, SE_DISCOURAGED, "Suspicious comment syntax. (Empty declaration.)", NULL);
		  /* fallthrough */
#endif

	       /* comment declaration end */
	       case '>'|PM_COMMENT_END2:
		  parse_mode=prev_mode_tag;
		  break;

#ifndef XHTML_ONLY
	       /* not really a comment */
	       case 'a'|PM_COMMENT_START:
	       case '0'|PM_COMMENT_START:
	       case '~'|PM_COMMENT_START:
	       case '$'|PM_COMMENT_START:
	       case ';'|PM_COMMENT_START:
	       case '='|PM_COMMENT_START:
	       case '/'|PM_COMMENT_START:
	       case '"'|PM_COMMENT_START:
	       case '\''|PM_COMMENT_START:
	       case '<'|PM_COMMENT_START:
	       case '>'|PM_COMMENT_START:
	       case '&'|PM_COMMENT_START:
	       case '!'|PM_COMMENT_START:
	       case '['|PM_COMMENT_START:
	       case '?'|PM_COMMENT_START:
	       case '#'|PM_COMMENT_START:
		  *err_level=html_error(input, *err_level, SE_DISCOURAGED, "Unescaped '<' character outside tag.", NULL);
		  buf_add_char('<'); buf_add_char('!'); buf_add_char('-');    /* store "<!-" literally */
		  parse_mode=prev_mode_tag;
		  recycle=1;
		  break;
#endif

	    }    /* comments */

	    /* DOCTYPE declarations */
	    {

	       /* normal char after "<!" => probably DOCTYPE declaration */
	       case 'a'|PM_EXCLAM:
		  parse_mode=PM_DOCTYPE;
		  /* fallthrough */

	       /* declaration char */
	       case 'a'|PM_DOCTYPE:
	       case '0'|PM_DOCTYPE:
	       case '~'|PM_DOCTYPE:
	       case '$'|PM_DOCTYPE:
	       case ';'|PM_DOCTYPE:
	       case '='|PM_DOCTYPE:
	       case '/'|PM_DOCTYPE:
	       case '"'|PM_DOCTYPE:
	       case '\''|PM_DOCTYPE:
	       case ' '|PM_DOCTYPE:
	       case '<'|PM_DOCTYPE:
	       case '&'|PM_DOCTYPE:
	       case '-'|PM_DOCTYPE:
	       case '!'|PM_DOCTYPE:
	       case '['|PM_DOCTYPE:
	       case '?'|PM_DOCTYPE:
	       case '#'|PM_DOCTYPE:
		  break;    /* ignore */

	       /* end of declaration */
	       case '>'|PM_DOCTYPE:
		  parse_mode=prev_mode_tag;    /* back to normal mode */
		  break;

	    }    /* DOCTYPE declarations */

	    /* CDATA sections */
	    {

	       /* first '[' of CDATA start */
	       case '['|PM_EXCLAM:
		  if(prev_mode_tag==PM_BLANK)    /* pending blank */
		     buf_add_char(' ');    /* add before CDATA */
		  parse_mode=PM_CDATA_START;
		  break;

	       /* normal char after first "<![" (should form "CDATA"...) */
	       case 'a'|PM_CDATA_START:
		  break;    /* ignore */

	       /* second '[' (after "<![CDATA") */
	       case '['|PM_CDATA_START:
		  parse_mode=PM_CDATA;
		  break;

	       /* normal CDATA char */
	       case 'a'|PM_CDATA:
	       case '0'|PM_CDATA:
	       case '~'|PM_CDATA:
	       case '$'|PM_CDATA:
	       case ';'|PM_CDATA:
	       case '='|PM_CDATA:
	       case '/'|PM_CDATA:
	       case '"'|PM_CDATA:
	       case '\''|PM_CDATA:
	       case ' '|PM_CDATA:
	       case '<'|PM_CDATA:
	       case '&'|PM_CDATA:
	       case '-'|PM_CDATA:
	       case '!'|PM_CDATA:
	       case '['|PM_CDATA:
	       case '?'|PM_CDATA:
	       case '#'|PM_CDATA:
		  buf_add_char(in);
		  break;

	       /* (possibly) CDATA end */
	       case '>'|PM_CDATA:
		  if(text_buf!=NULL && text_buf_len>=2 && text_buf[text_buf_len-2]==']' && text_buf[text_buf_len-1]==']') {    /* buffer contains at least two chars, the last ones being the two ']' of the CDATA end sequence "]]>" */
		     text_buf_len-=2;    /* remove the "]]" */
		     text_buf=realloc(text_buf, text_buf_len);    /* resize buffer */
		     if(text_buf==NULL) {
			fprintf(stderr, "memory allocation error while syntax parsing (in function parse_syntax)\n");
			exit(1);
		     }
		     parse_mode=PM_CONTENT;    /* now back to normal mode... */
		  } else    /* not CDATA end -> add the '>' as normal char */
		     buf_add_char(in);
		  break;

	    }    /* CDATA sections */

	    /* not really a declaration */
	    case '0'|PM_EXCLAM:
	    case '~'|PM_EXCLAM:
	    case '$'|PM_EXCLAM:
	    case ';'|PM_EXCLAM:
	    case '='|PM_EXCLAM:
	    case '/'|PM_EXCLAM:
	    case '"'|PM_EXCLAM:
	    case '\''|PM_EXCLAM:
	    case ' '|PM_EXCLAM:
	    case '<'|PM_EXCLAM:
	    case '&'|PM_EXCLAM:
	    case '!'|PM_EXCLAM:
	    case '?'|PM_EXCLAM:
	    case '#'|PM_EXCLAM:
	       *err_level=html_error(input, *err_level, SE_DISCOURAGED, "Unescaped '<' character outside tag.", NULL);
	       buf_add_char('<'); buf_add_char('!');    /* store "<!" literally */
	       parse_mode=prev_mode_tag;    /* back to normal mode (don't parse as tag) */
	       recycle=1;    /* process current char as normal text */
	       break;

	    /* processing instructions */
	    {

	       /* '?' starting processing instruction (after '<') */
	       case '?'|PM_TAG_START:
		  parse_mode=PM_INSTR;
		  break;

	       /* instruction char */
	       case 'a'|PM_INSTR:
	       case '0'|PM_INSTR:
	       case '~'|PM_INSTR:
	       case '$'|PM_INSTR:
	       case ';'|PM_INSTR:
	       case '='|PM_INSTR:
	       case '/'|PM_INSTR:
	       case '"'|PM_INSTR:
	       case '\''|PM_INSTR:
	       case ' '|PM_INSTR:
	       case '<'|PM_INSTR:
	       case '&'|PM_INSTR:
	       case '-'|PM_INSTR:
	       case '!'|PM_INSTR:
	       case '['|PM_INSTR:
	       case '>'|PM_INSTR:
	       case '#'|PM_INSTR:
		  break;    /* ignore */

	       /* '?' (possibly) ending instruction */
	       case '?'|PM_INSTR:
		  parse_mode=PM_INSTR_END;
		  break;

	       /* second '?' */
	       case '?'|PM_INSTR_END:
		  break;    /* ignore (the first one) */

	       /* normal char after '?' => not end */
	       case 'a'|PM_INSTR_END:
	       case '0'|PM_INSTR_END:
	       case '~'|PM_INSTR_END:
	       case '$'|PM_INSTR_END:
	       case ';'|PM_INSTR_END:
	       case '='|PM_INSTR_END:
	       case '/'|PM_INSTR_END:
	       case '"'|PM_INSTR_END:
	       case '\''|PM_INSTR_END:
	       case ' '|PM_INSTR_END:
	       case '<'|PM_INSTR_END:
	       case '&'|PM_INSTR_END:
	       case '-'|PM_INSTR_END:
	       case '!'|PM_INSTR_END:
	       case '['|PM_INSTR_END:
	       case '#'|PM_INSTR_END:
		  parse_mode=PM_INSTR;    /* still process instuction */
		  break;

	       /* '>' after '?' => really end */
	       case '>'|PM_INSTR_END:
		  parse_mode=prev_mode_tag;    /* back to normal mode */
		  break;

	    }    /* processing instuctions */

	    default:    /* no valid case => syntax error */
	       if((parse_mode&0xf000)==PM_TAG_START) {    /* any of the tag parsing modes */
		  *err_level=html_error(input, *err_level, SE_CRITICAL, "Invalid tag. (Unescaped '<' character?)", "Trying limitation of damage...");
		  parse_mode=prev_mode_tag;    /* bail out */
		  recycle=1;
	       } else {
		  if(isprint(in))
		     *err_level=html_error(input, *err_level, SE_CRITICAL, "Unexpected character.", "Ignoring.");
		  else
		     *err_level=html_error(input, *err_level, SE_WORKAROUND, "Illegal character.", "Ignoring.");
	       }
	 }    /* switch(dispatch) */
      } while(recycle);    /* additional dispatch pass after parsing mode change */
   }    /* for all chars in file (while !EOF) */

   /* EOF should not occur while parsing anything but normal text */
   if(!(parse_mode==PM_CONTENT || parse_mode==PM_BLANK)) {    /* not normal text */
      if(!input->user_break)
	 *err_level=html_error(input, *err_level, SE_CRITICAL, "Unexpected end of file. (Not in normal mode.)", "Ignoring.");
   }
#ifdef XHTML_ONLY
   /* in xhtml, all elements have to be closed */
   else if (cur_el->parent!=NULL)    /* inside some element (only global element has no parent) */
      if(!input->user_break)
	 html_error(input, 0, 0, "Unexpected end of file. (Unclosed elements.)", NULL);
#else    /* not XHTML_ONLY */
   else if(text_buf_len!=0) {    /* have some pending content */
      last_el=add_element();    /* -> create dummy tag */
      insert_buf(&last_el->content);    /* and save to it */
   }
#endif    /* not XHTML_ONLY */

   last_el->list_next=tree_top;

   if(!cfg.debug) html_error(NULL, 0, 0, NULL, NULL);    /* flush ignored errors */
   DMSG(("\n"));

   /* warn about syntax errors */
   if(*err_level) {    /* some error(s) occured */
      if((cfg.parser==VALID_HTML && *err_level<SE_WORKAROUND) || (cfg.parser==BROKEN_HTML && *err_level<SE_CRITICAL) || (cfg.parser==IGNORE_BROKEN && *err_level<SE_NODATA))    /* can be ignored -> clear flag */
	 *err_level=SE_NO;
      else {    /* don't ignore -> print warning */
	 printf("\n");

	 switch(*err_level) {
	    case SE_DISCOURAGED:
	       set_color_raw(COLOR_YELLOW|8);
	       printf("Warning: ");
	       reset_color_raw();
	       printf("The loaded page contains HTML constructs ");
	       set_color_raw(COLOR_WHITE|8);
	       printf("discouraged");
	       reset_color_raw();
	       printf(" by the standard, and ");
	       set_color_raw(COLOR_WHITE|8);
	       printf("not consistently handled");
	       reset_color_raw();
	       printf(" by existing browsers.\n\n");

	       printf("The page ");
	       set_color_raw(COLOR_WHITE|8);
	       printf("might be interpreted different");
	       reset_color_raw();
	       printf(" than in other browsers, or from what the author intended.\n\n");

	       printf("You might want to inform the page author. (See syntax_error.html or syntax_error.txt in the documentation directory for more info.)\n");
	       break;

	    case SE_UNIMPLEMENTED:
	       set_color_raw((COLOR_YELLOW|8)<<4);
	       printf("Warning: ");
	       reset_color_raw();
	       printf("The loaded page contains HTML constructs ");
	       set_color_raw(COLOR_WHITE|8);
	       printf("discouraged");
	       reset_color_raw();
	       printf(" by the standard, and ");
	       set_color_raw(COLOR_WHITE|8);
	       printf("not supported");
	       reset_color_raw();
	       printf(" by netrik or (any?) other browsers.\n\n");

	       printf("The page ");
	       set_color_raw(COLOR_WHITE|8);
	       printf("might be interpreted different");
	       reset_color_raw();
	       printf(" than in other browsers, or from what the author intended.\n\n");

	       printf("You might want to inform the page author. (See syntax_error.html or syntax_error.txt in the documentation directory for more info.)\n");
	       break;

	    case SE_WORKAROUND:
	       set_color_raw(COLOR_RED|8);
	       printf("Warning: ");
	       reset_color_raw();
	       printf("The loaded page contains HTML ");
	       set_color_raw(COLOR_WHITE|8);
	       printf("syntax errors");
	       reset_color_raw();
	       printf(".\n\n");

	       printf("I tried to use ");
	       set_color_raw(COLOR_WHITE|8);
	       printf("workarounds");
	       reset_color_raw();
	       printf(", but I ");
	       set_color_raw(COLOR_WHITE|8);
	       printf("can't ensure the page will not be interpreted different");
	       reset_color_raw();
	       printf(" than in other browsers, or from what the author intended.\n\n");

	       printf("You might want to inform the page author. (See syntax_error.html or syntax_error.txt in the documentation directory for more info.)\n");
	       break;

	    case SE_CRITICAL:
	       set_color_raw((COLOR_RED<<4)|(COLOR_WHITE|8));
	       printf("Warning: ");
	       reset_color_raw();
	       printf("The loaded page contains ");
	       set_color_raw(COLOR_WHITE|8);
	       printf("heavy HTML syntax errors");
	       reset_color_raw();
	       printf(".\n\n");

	       printf("I have ");
	       set_color_raw(COLOR_WHITE|8);
	       printf("no workarounds");
	       reset_color_raw();
	       printf(" for these, and the page will ");
	       set_color_raw(COLOR_WHITE|8);
	       printf("most probably look broken");
	       reset_color_raw();
	       printf("!\n\n");

	       printf("You might want to inform the page author. (See syntax_error.html or syntax_error.txt in the documentation directory for more info.)\n");
	       break;

	    case SE_NODATA:
	       set_color_raw(COLOR_RED|8);
	       printf("Error: ");
	       set_color_raw(COLOR_WHITE|8);
	       printf("No data read.\n");
	       reset_color_raw();
	       break;

	    case SE_NO:    /* shouldn't occur... */
	    case SE_BREAK:
	    case SE_FAIL:
	       break;
	 }    /* switch(err_level) */
	 fflush(stdout);
      }    /* don't ignore */
   }    /* syntax errors */

   if(input->type==RES_FAIL && *err_level!=SE_NODATA) {    /* file read error (but some data read) */
      *err_level=SE_FAIL;

      set_color_raw(COLOR_RED|8);
      printf("File not loaded completely.\n");
      reset_color_raw();
      fflush(stdout);
   }

   if(input->user_break) {
      *err_level=SE_BREAK;

      set_color_raw(COLOR_BLUE|8);
      printf("User break.\n");
      reset_color_raw();
      fflush(stdout);
   }

   return(tree_top);
}

/* unallocate the complete syntax tree;
 * traverses the whole tree (list), and frees every node */
void free_syntax(tree_top, elements_parsed)
struct Element	*tree_top;
int		elements_parsed;    /* element and attribute names are converted to enums (no strings to free) */
{
   struct Element	*cur_el;    /* element to be deleted now */
   struct Element	*next_el;    /* saved "list_next" of "cur_el" */
   int			cur_attr;

   /* for all elements in tree */
   next_el=tree_top;    /* start with first element */
   do {    /* until back at top */
      /* advance in list */
      cur_el=next_el;
      next_el=cur_el->list_next;    /* need to save "list_next" pointer, as won't be available after deleting current element */

      /* delete data */
      if(!elements_parsed)
	 free(cur_el->name.str);
      for(cur_attr=0; cur_attr<cur_el->attr_count; ++cur_attr) {
	 if(!elements_parsed)
	    free(cur_el->attr[cur_attr].name.str);
         if(!attr_table[cur_el->attr[cur_attr].name.type].numeric)
	    free(cur_el->attr[cur_attr].value.str);
      }
      free(cur_el->attr);
      free(cur_el->content);

      free(cur_el);    /* delete node */

   } while(next_el!=tree_top && next_el!=NULL);    /* for all elements (until back at top, or open list end (will occur if tree is unfinished)) */
}
