/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * main.c -- you guess it ;-) : Thats's the main program.
 *
 * (C) 2001, 2002, 2003 antrik
 *     2001, 2002 Patrice Neff
 *
 * handle startup (command line etc.); then load (and layout) first file (given
 * as argument), and either dump it and quit immediatly (with --dump), or
 * display it interactively and load new files (requested by user) in a loop
 */
#include <curses.h>
#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"

#include "cfg.h"
#include "cmdline.h"
#include "colors.h"    /* for load_color_map() */
#include "debug.h"
#include "forms.h"
#include "form-file.h"    /* for edit_textarea() */
#include "interrupt.h"
#include "links.h"
#include "layout.h"
#include "page.h"
#include "pager.h"
#include "readline.h"
#include "render.h"
#include "screen.h"
#include "search.h"
#include "url.h"

int main(int argc, char *argv[])
{
   unsigned		cfg_len;    /* config file size */
   int			argc_all;    /* total number of arguments in config file and command line */
   char			**argv_all;    /* array (vector) of pointers to all config file and command line arguments */
   char			*cfg_args;    /* array holding arguments from config file */

   int			opt_index;    /* position of first non-option parameter in (merged) command line */

   int			cur_page;    /* number of current page in page list */
   int			page_width;
   enum Pager_ret	pager_ret;    /* return value of pager (indicates if ended by 'q', by ':', or by following link) */
   enum Syntax_error	syntax_err;    /* parse_syntax() found syntax errors in the HTML page */

   setlocale(LC_ALL, "");    /* enable (default) locale settings */

   /* read config file */
   {
      char	*home;    /* user's home directory */

      home=getenv("HOME");
      if(home!=NULL) {
	 struct stat	stats;    /* config file stats */
	 char		cfg_name[strlen(home)+sizeof("/.netrikrc")];
	 snprintf(cfg_name, sizeof(cfg_name), "%s/.netrikrc", home);

	 if(stat(cfg_name, &stats)==0) {    /* file exists, and stat succeeded */
	    cfg_len=(int)stats.st_size;
	 } else {    /* can't stat */
	    if(errno==ENOENT) {    /* OK, just doesn't exist */
	       cfg_len=0;
	    } else {
	       fprintf(stderr, "error while checking for config file (%s):", cfg_name); perror("");
	       exit(1);
	    }
	 }    /* can't stat */

	 if(cfg_len) {    /* have a (nonempty) config file -> merge options from file with command line options */
	    FILE	*cfg_file;

	    char	*dest_ptr;    /* current position in cfg_args */
	    char	*arg_start;    /* where currently scanned argument began */

	    cfg_file=fopen(cfg_name, "r");
	    if(cfg_file==NULL) {
	       fprintf(stderr, "error: can't open config file (%s):", cfg_name); perror("");
	       exit(1);
	    }

	    cfg_args=malloc(sizeof(char[cfg_len]));
	    if(cfg_args==NULL) {
	       fprintf(stderr, "memory allocation error while reading config file\n");
	       exit(1);
	    }

	    if(fread(cfg_args, sizeof(char), cfg_len, cfg_file)!=cfg_len) {    /* read whole file at once */
	       fprintf(stderr, "error reading config file:"); perror("");
	       exit(1);
	    }

	    if(cfg_args[cfg_len-1]!='\n') {
	       fprintf(stderr, "syntax error in config file: last line not terminated with newline character\n");
	       exit(1);
	    }

	    /* store config file arguments in argument list */
	    for(argc_all=0, argv_all=NULL, dest_ptr=arg_start=cfg_args; dest_ptr<cfg_args+cfg_len; ++dest_ptr) {    /* scan whole config file data */
	       if(*dest_ptr=='\n') {    /* line end */
		  *dest_ptr='\0';    /* -> end argument here */

		  /* store pointer to argument */
		  argv_all=realloc(argv_all, sizeof(char *[++argc_all+argc]));    /* also reserve the space for the command line arguments already */
		  if(argv_all==NULL) {
		     fprintf(stderr, "memory allocation error while reading config file\n");
		     exit(1);
		  }
		  argv_all[argc_all]=arg_start;    /* store (beginning with 1, not 0, as argument 0 is normally own name!) */

		  arg_start=dest_ptr+1;    /* next argument starts after the '\n' */
	       }    /* line end */
	    }    /* for all chars in config file */

	    /* append command line args (starting with 1st, not 0th!) after config file args */
	    memcpy(&argv_all[argc_all+1], argv+1, sizeof(char *[argc-1]));
	    argc_all+=argc;
	 } else {    /* no config file -> just use command line arguments */
	    argv_all=argv;
	    argc_all=argc;
	 }
      } else {    /* no $HOME */
	 fprintf(stderr, "error: can't find your home directory (while looking for config file)\n");
	 exit(1);
      }

#ifdef DEBUG
      if(cfg.debug) {
	 int	arg;

	 debug_printf("arguments:");
	 for(arg=1; arg<argc_all; ++arg)
	    debug_printf(" %s", argv_all[arg]);
	 debug_printf("\n");
      }
#endif
   }

/* Patrice --> */
   /* process arguments */
   opt_index=config_cmdln(argc_all, argv_all);
   if(opt_index==argc_all) {    /* no non-option arguments */
      fprintf(stderr,"usage: %1$s html-file\n   or: %1$s -          (read from stdin)\n", argv[0]);
      exit(3);
   }
/* <-- Patrice */

   if(cfg.force_colors < 0)    /* not explicitely set -> default depends on color scheme */
      cfg.force_colors=!cfg.bright_background;    /* force colors if dark color scheme to be used */

   load_color_map();

   if(cfg.term_width)
      page_width=init_curses();
   else {
      init_curses();
      page_width=80;
   }

   init_int();

   cur_page=load_page(NULL, argv_all[argc_all-1], NULL, NULL, page_width, &syntax_err);    /* load URL given in last command line argument */

#ifdef DEBUG
   if(cfg_len) {    /* have read config file -> need to clean up */
      DMSG(("\nfreeing memory used by argument list...\n"));
      free(cfg_args);
      free(argv_all);
   }
#endif

   if(!cfg.dump) {
      start_curses();
      endwin();    /* return to scroll mode for now, as we might want print something still before actually starting pager */

      if(syntax_err
#ifdef DEBUG
         || cfg.debug
#endif
      ) {
	 syntax_err=SE_NO;
	 fprintf(stderr, "\nHit some key to start viewer.\n");
	 cbreak(); getchar(); endwin();    /* wait for *single* keypress (endwin() resets normal scroll mode terminal settings) */
      }

      do {    /* until quit */
	 int	pager_wait=0;    /* wait for keypress before returning to pager */

         pager_ret=display(page_list.page[cur_page]);    /* start pager with current page (returns when 'q' pressed, when command line mode entered with ':', or when link followed) */

	 switch(pager_ret) {
	    /* pager ended by ':' */
	    case RET_COMMAND: {
	       char	*command;    /* user input */

	       command=read_line(":");
	       if(command!=NULL && *command) {    /* nonempty (any command entered) */
		  add_history(command);
	       
		  if(strncmp(command, "e ", 2)==0 || strncmp(command, "E ", 2)==0) {    /* load file */
		     const char	*url=command+2;    /* name begins after "e " */

		     char	*blank_pos;    /* first blank in url */

		     /* remove trailing blank(s) */
		     blank_pos=strchr(url, ' ');    /* find first blank */
		     if(blank_pos!=NULL)    /* anything found */
			*blank_pos=0;    /* end string at first blank */

		     DMSG(("\nfreeing memory used by old page...\n"));
		     free_layout(page_list.page[cur_page]->layout);    /* get rid of old document */

		     if(command[0]=='e' && page_list.page[cur_page]->url->proto.type!=PT_INTERNAL)    /* load relative URL */
			cur_page=load_page(page_list.page[cur_page]->url, url, NULL, NULL, page_width, &syntax_err);    /* load new page (use current page URL as base) */
		     else    /* load absolute URL */
			cur_page=load_page(NULL, url, NULL, NULL, page_width, &syntax_err);    /* load new page using absoulute URL (no base) */
		  } else {    /* command not "e ..." */
		     printf("unknown command\n");
		     pager_wait=1;
		  }    /* command not "e " */

#ifdef DEBUG
		  if(cfg.debug)
		     pager_wait=1;
#endif
	       }    /* command nonempty */
	       free(command);
	       break;
	    }    /* RET_COMMAND */

	    /* pager ended by requesting text search */
	    case RET_SEARCH: {
	       struct Page	*page=page_list.page[cur_page];

	       char		*string;

	       struct Item	*start_item;
	       int		start_pos;

	       string=read_line("/");
	       if(string==NULL) {    /* aborted (^D) */
		  search.type=SEARCH_NO;
		  break;
	       }

	       if(*string) {    /* nonempty -> store as new search string */
		  add_history(string);
		  if(search.string!=NULL) {
		     DMSG(("freeing old search string...\n"));
		     free(search.string);
		  }
		  search.string=string;
	       }

	       if(search.string==NULL) {    /* nothing entered, and there was no previous search string either */
		  set_color_raw(COLOR_RED|8);
		  printf("No search string.\n");
		  reset_color_raw();
		  pager_wait=1;

		  search.type=SEARCH_NO;
		  break;
	       }

	       /* find start position for search (text item containing cursor and position inside the text) */
	       DMSG(("looking for search start position (cursor position: %d, %d)\n   ", page->cursor_x, page->cursor_y));
	       {
		  int	cur_item;

		  start_item=NULL;

		  for(cur_item=0; cur_item < page->layout->page_map[page->cursor_y].count; ++cur_item) {    /* all items in cursor line */
		     const struct Item	*item=page->layout->page_map[page->cursor_y].item[cur_item];

		     DMSG(("."));
		     if(item->type==ITEM_TEXT && item->x_start <= page->cursor_x && item->x_end > page->cursor_x) {    /* text block, and cursor is inside */
			int	line_offset;    /* position of cursor relative to start of text in this line */

			line_offset=page->cursor_x - line_pos(item, page->cursor_y);
			if(line_offset<0) {    /* cursor before start of text in this line */
			   DMSG(("cursor before text line -> moving\n   "));
			   line_offset=-1;    /* -> assume it stands just before first char in line, so search will begin at line start */
			}

			if(line_offset < line_end(item, page->cursor_y) - line_start(item, page->cursor_y)) {    /* text in this line doesn't end before cursor position -> begin search here */
			   DMSG(("cursor in text line\n"));
			   start_item=(struct Item *)item;
			   start_pos=line_start(item, page->cursor_y) + line_offset + 1;    /* begin search after cursor position */
			   break;
			} else if(item->y_end > page->cursor_y) {    /* cursor after end of line, but not last line in text block -> begin search with next line */
			   DMSG(("cursor after current text line, but in text block\n"));
			   start_item=(struct Item *)item;
			   start_pos=line_end(item, page->cursor_y);    /* begin search at beginning of next line */
			   break;
			}
		     }    /* cursor in text block */
		  }    /* all items in line */
		  if(start_item==NULL) {    /* nothing found (cursor not inside text block) -> find first text block starting after cursor (or first on page if nothing after cursor) */
		     struct Item	*item;

		     DMSG(("cursor not in text block\nsearching for first text block after cursor\n   "));
		     for(item=page->layout->item_tree->parent; item!=NULL; item=item->list_next) {    /* all items (in page order) */
			DMSG(("."));
			if(item->type==ITEM_TEXT) {
			   if(item->y_start > page->cursor_y) {    /* starts after cursor pos -> begin search at this one */
			      DMSG(("found a text block after cursor"));
			      start_item=item;
			      start_pos=0;
			      break;    /* don't search further */
			   }
			}    /* text item */
		     }    /* for all items */
		     DMSG(("\n"));
		  }    /* cursor not in text block */
	       }    /* find start position */

	       /* perform search */
	       {
		  struct Item	*item;
		  char		*found_pos=NULL;

#ifdef DEBUG
		  if(start_item!=NULL) {
		     DMSG(("start item:\n\n%s\n\n", start_item->data.string->text));
		     DMSG(("start pos: %d\n", start_pos));
		  } else
		     DMSG(("at page end\n"));
#endif

		  DMSG(("starting search "));
		  for(item=start_item; item!=NULL; (item=item->list_next), start_pos=0) {    /* all items beginning with "start_item" (don't use "start_pos" except in first!) */
		     DMSG(("."));
		     if(item->type==ITEM_TEXT) {
			DMSG(("\""));
			found_pos=strstr(item->data.string->text + start_pos, search.string);
			if(found_pos!=NULL)
			   break;
		     }
		  }

		  if(found_pos==NULL) {    /* nothing found yet -> wrap search */
		     struct Item	*end_item;

		     DMSG(("\n"));
		     set_color_raw(COLOR_BLUE|8);
		     printf("Search wrapped.\n");
		     reset_color_raw();
		     fflush(stdout);
		     pager_wait=1;

		     if(start_item!=NULL)    /* have a start item -> search up to start item (inclusive!) */
			end_item=start_item->list_next;
		     else    /* no start item -> search to page end (last item) */
			end_item=NULL;

		     for(item=page->layout->item_tree->parent; item!=end_item; item=item->list_next) {    /* all items until back at start */
			DMSG(("."));
			if(item->type==ITEM_TEXT) {
			   DMSG(("\""));
			   found_pos=strstr(item->data.string->text, search.string);
			   if(found_pos!=NULL)
			      break;
			}
		     }
		  }    /* wrap search */

		  if(found_pos!=NULL) {    /* something found -> set cursor to match position */
		     const int	string_offset=found_pos - item->data.string->text;    /* position of match relative to string start */

		     int	line;

		     DMSG(("found\n"));
		     DMSG(("item:\n\n%s\n\n", item->data.string->text));
		     DMSG(("position: %d\n", string_offset));

		     DMSG(("determining new cursor position "));
		     /* find line containing match */
		     for(line=item->y_start; line<item->y_end; ++line) {    /* all lines in text block */
			DMSG(("."));
			if(line_end(item, line) > string_offset)    /* first line ending after match pos -> found */
			   break;
		     }
#ifdef DEBUG
		     if(line==item->y_end) {
			fprintf(stderr, "internal error: failed to find line containing match position\n");
			exit(100);
		     }
#endif

		     page->cursor_y=line;
		     page->cursor_x=line_pos(item, line) + string_offset-line_start(item, line);
		     DMSG(("\nposition: %d, %d\n", page->cursor_x, page->cursor_y));
		  } else {    /* nothing found */
		     set_color_raw(COLOR_RED|8);
		     printf("No match.\n");
		     reset_color_raw();
		     pager_wait=1;

		     search.type=SEARCH_NO;
		  }
	       }    /* perform search */

#ifdef DEBUG
	       if(cfg.debug)
		  pager_wait=1;
#endif
	       break;
	    }    /* RET_SEARCH */

	    /* pager ended by following link */
	    case RET_LINK: {
	       const struct Page	*old_page=page_list.page[cur_page];
	       struct Link		*link=get_link(old_page->layout, old_page->active_link);    /* link data in item tree */

	       switch(link->form) {    /* dispatch on link/form type */
		  case FORM_NO: {    /* normal link (not form control) */
		     char	url[strlen(link->value.data)+1];
		     strcpy(url, link->value.data);    /* save link URL before killing old page */

		     if(url[0]!='#') {    /* not local anchor -> load new document */
			DMSG(("\nfreeing memory used by old page...\n"));
			free_layout(old_page->layout);    /* get rid of old document */
			cur_page=load_page(old_page->url, url, NULL, NULL, page_width, &syntax_err);    /* load link (use current page URL as base) */
		     } else    /* local anchor -> keep document, just jump to anchor */
			cur_page=load_page(old_page->url, url, NULL, old_page, page_width, &syntax_err);    /* load link (use current page URL as base) */
#ifdef DEBUG
		     if(cfg.debug)
			pager_wait=1;
#endif
		     break;
		  }    /* FORM_NO */

		  case FORM_TEXT:
		  case FORM_PASS:
		  case FORM_FILE: {
		     char	*new_value;

		     new_value=read_line(link->form==FORM_FILE ? "file:" : "value:");
		     if(new_value!=NULL) {    /* not aborted -> store new value */
			free(link->value.data);
			link->value.data=new_value; link->value.size=strlen(new_value);
			update_form(old_page->layout, old_page->active_link);
		     }

		     if(link->form==FORM_FILE && *new_value) {    /* file to upload -> check */
			if(access(new_value, R_OK)==-1) {
			   set_color_raw(COLOR_RED|8);
			   printf("\nWarning: Can't access file \"%s\".\n", new_value);
			   reset_color_raw();
			   pager_wait=1;
			}
		     }

		     break;
		  }    /* FORM_TEXT/FORM_PASS/FORM_FILE */

		  case FORM_HIDDEN:
		     break;    /* shouldn't occur... */

		  case FORM_CHECKBOX:
	          case FORM_MULTIOPTION:
		     link->enabled=1-link->enabled;
		     update_form(old_page->layout, old_page->active_link);
		     break;

		  case FORM_RADIO:
		  case FORM_OPTION: {
		     const struct Item	*form_item=get_form_item(old_page->layout, old_page->active_link);

		     struct Form_handle	handle=form_start(form_item, 0);    /* handle to iterate through all form controls (unfiltered) */
		     struct Link	*sibling;    /* other controls in this form */

		     /* disable any other radio buttons/options with same name */
		     if(link->name!=NULL)    /* belongs to some group at all... */
			while((sibling=form_next(&handle)) != NULL) {    /* for all form controls */
			   if(sibling->name!=NULL && !strcmp(sibling->name, link->name)) {    /* in same group -> disable */
			      if(sibling->enabled) {
				 sibling->enabled=0;
				 set_form(handle.cur_item->data.string, sibling);    /* beware -- crude hack! */
			      }
			   }
			}

		     link->enabled=1;
		     update_form(old_page->layout, old_page->active_link);

		     break;
		  }

		  case FORM_TEXTAREA: {
		     char	*err=edit_textarea(&link->value, link->name);
		     if(err==NULL)    /* no error */
			update_form(old_page->layout, old_page->active_link);
		     else {
			set_color_raw(COLOR_RED|8);
			printf("\nEditing textarea failed: %s\n", err);
			reset_color_raw();
			pager_wait=1;
		     }
		     break;
		  }

/* Patrice, antrik --> */
	          case FORM_SUBMIT: {		     
		     const struct Item	*form_item=get_form_item(old_page->layout, old_page->active_link);

		     if(form_item==NULL) {    /* orphaned button (no parent form) */
			set_color_raw(COLOR_RED|8);
			printf("\nError: Submit button outside form\n");
			reset_color_raw();
			pager_wait=1;
			break;
		     }

		     if(form_item->data.form->url!=NULL) {    /* "action" given */
			link->enabled=1;    /* make the used submit button successful */

			cur_page=load_page(old_page->url, form_item->data.form->url, form_item, NULL, page_width, &syntax_err);    /* load form "action" URL (use current page URL as base); submit form data with HTTP request */
			DMSG(("\nfreeing memory used by old page...\n"));
			free_layout(old_page->layout);
#ifdef DEBUG
			if(cfg.debug)
			   pager_wait=1;
#endif
		     } else {    /* no "action" */
			set_color_raw(COLOR_RED|8);
			printf("\nError: Form gives no URL to submit to\n");
			reset_color_raw();
			pager_wait=1;
		     }
		     break;
		  }    /* FORM_SUBMIT */
/* <-- Patrice, antrik */
		     
	       }    /* switch link type */

	       break;
	    }    /* RET_LINK */

	    /* "show link URL" requested */
	    case RET_LINK_URL: {
	       const struct Page	*page=page_list.page[cur_page];
	       const struct Link	*link=get_link(page->layout, page->active_link);    /* link data in item tree */

	       char			*url;

	       if(link->form) {
		  const struct Item	*form_item=get_form_item(page->layout, page->active_link);
		  url=form_item->data.form->url;
	       } else
		  url=link->value.data;

	       printf("\n%s URL:\n", link->form?"submit":"link");
	       set_color_raw(COLOR_WHITE|8);
	       printf("%s\n", url);
	       reset_color_raw();
	       pager_wait=1;
	       break;
	    }    /* RET_LINK_URL */

	    /* show absolute link target URL */
	    case RET_ABSOLUTE_URL: {
	       const struct Page	*page=page_list.page[cur_page];
	       const struct Link	*link=get_link(page->layout, page->active_link);    /* link data in item tree */

	       char			*url;
	       struct Url		*target_url;    /* effective URL of link target */

	       if(link->form) {
		  const struct Item	*form_item=get_form_item(page->layout, page->active_link);
		  url=form_item->data.form->url;
	       } else
		  url=link->value.data;

	       target_url=merge_urls(page->url, url, NULL);    /* (temporarily) construct target URL merged from current URL and link URL */

	       if(target_url->proto.type!=PT_INTERNAL) {    /* could extract absolute URL */
		  printf("\nabsolute link target URL:\n");
		  set_color_raw(COLOR_WHITE|8);
		  printf("%s\n", target_url->full_url);
		  reset_color_raw();
	       } else {
		  set_color_raw(COLOR_RED|8);
		  printf("\ncan't get target URL\n");
		  reset_color_raw();
	       }

	       free_url(target_url);    /* don't keep */

	       pager_wait=1;
	       break;
	    }    /* RET_ABSOLUTE_URL */

	    /* "show current page URL" requested */
	    case RET_URL: {
	       const struct Page	*page=page_list.page[cur_page];
	       const char		*url_str=page->url->full_url;

	       if(page->url->proto.type!=PT_INTERNAL) {    /* has URL */
		  printf("\ncurrent page URL:\n");
		  set_color_raw(COLOR_WHITE|8);
		  printf("%s\n", url_str);
		  reset_color_raw();
	       } else    /* internal */
		  printf("\npage is from stdin and has no URL\n");
	       pager_wait=1;
	       break;
	    }    /* RET_LINK_URL */

	    /* pager ended by going back or forward in URL history */
	    case RET_HISTORY: {
	       struct Page	*ref=page_list.page[cur_page];    /* old page descriptor, if new page is from same URL (can reuse layout data) */
	       int		first_page, last_page;    /* range of page list entries to check for local */
	       int		page;    /* currently checked page list entry */

	       if(page_list.pos!=cur_page) {    /* not reloading same page -> check if can reuse layout data */
		  DMSG(("\nchecking whether we stay in same document...\n"));
		  if(page_list.pos>cur_page) {
		     first_page=cur_page;
		     last_page=page_list.pos;
		  } else {
		     first_page=page_list.pos;
		     last_page=cur_page;
		  }
		  for(page=first_page+1; page<=last_page; ++page) {    /* test all pages between old an new one for being created by jumping to local anchors */
		     if(!page_list.page[page]->url->local) {    /* not local -> can't reuse layout data */
			ref=NULL;
			break;
		     }
		  }
	       } else    /* reloading same page -> never reuse */
		  ref=NULL;

	       if(ref==NULL) {    /* layout data won't be reused */
		  DMSG(("freeing memory used by old page...\n"));
		  free_layout(page_list.page[cur_page]->layout);
	       }

	       DMSG(("loading new page:\n"));
	       cur_page=load_page(page_list.page[page_list.pos]->url, NULL, NULL, ref, page_width, &syntax_err);    /* reload page from history (using already split url) */

#ifdef DEBUG
	       if(cfg.debug)
		  pager_wait=1;
#endif
	       break;
	    }    /* RET_HISTORY */

	    /* pager ended by SIGWINCH */
	    case RET_WINCH:
	       resize(page_list.page[cur_page]->layout, COLS);
	       break;

	    case RET_QUIT:
	       break;
	    case RET_NO:    /* suppress warning */
	       break;
	 }    /* switch pager_ret */

	 if(syntax_err) {
	    syntax_err=SE_NO;
	    pager_wait=1;
	 }

	 if(pager_wait) {
	    printf("\nHit some key to return to pager.\n"); fflush(stdout);
	    cbreak(); getchar(); endwin();    /* wait for *single* keypress (endwin() resets normal scroll mode terminal settings) */
	 }

      } while(pager_ret!=RET_QUIT);    /* until pager ended by 'q' */
   } else {    /* cfg.dump */
      DMSG(("layouted page:\n"));
      dump(page_list.page[cur_page]->layout);
   }

#ifdef DEBUG
   DMSG(("\nfreeing memory used by page...\n"));
   free_layout(page_list.page[cur_page]->layout);

   DMSG(("freeing memory used by page list...\n"));
   free_page_list();

   DMSG(("freeing memory use by search string...\n"));
   free(search.string);
#endif

   return(0);
}
