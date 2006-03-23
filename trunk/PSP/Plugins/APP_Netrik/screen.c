/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * screen.c -- all screen handling functions (curses wrappers).
 *
 * (C) 2001, 2002, 2003 antrik
 *
 * Presently the full screen functions aren't used by the main program...
 */
#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <term.h>

#include "cfg.h"
#include "screen.h"
#include "winch.h"

/* init curses full screen mode;
 * includes initializing color mode and color pairs */
void start_curses(void)
{
   initscr();    /* init curses */

   /* set options */
   if((cfg.color && start_color()==ERR)
      || cbreak()==ERR    /* unbuffered input */
      || noecho()==ERR    /* no key echo */
      || intrflush(stdscr, FALSE)==ERR    /* don't flush output on signals */
      || keypad(stdscr, TRUE)==ERR    /* translate function key sequences */
   ) {
      endwin();
      fprintf(stderr, "error initializing curses in fullscreen mode\n");
      exit(1);
   }
   nonl();    /* don't mess with cr on keyboard input */
   idlok(stdscr, TRUE);    /* allow hardware scrolling */

   /* prepare color pairs */
   if(cfg.color) {
      int	pair;    /* currently processed color pair */

      for(pair=1; pair<64; ++pair) {    /* 8x8 color pairs (all combinations of fg and bg color); pair 0 is hard-wired (to white on black instead of black on black) */
	 int	fg, bg;

	 fg=(pair+7)%8; bg=pair/8;    /* fg colorspace rotated, so white on black will be mapped to pair 0 */
	 if(!cfg.force_colors) {
	    if(use_default_colors()!=ERR) {    /* can use default colors */
	       if(bg==COLOR_BLACK)
		  bg=-1;    /* use terminal default */
	       if(fg==COLOR_WHITE)
		  fg=-1;
	    }
	 }
	 if(init_pair(pair, fg, bg)==ERR) {    /* set foreground and background of pair */
	    endwin();
	    fprintf(stderr, "error initializing curses in fullscreen mode (can't set color pair %d)\n", pair);
	    exit(1);
	 }
      }

      if(cfg.force_colors)
	 bkgdset(COLOR_PAIR(0));
   }
}

/* set screen attributes (color, bold)
 * (standard colors for white foreground/black background are handled in init) */
void set_color(color)
int	color;
{
   int	attrs=A_NORMAL;

   int	fg=(color+1)&0x07;    /* foreground color */
   int	bg=(color&0x70)>>4;    /* background */

   if(!cfg.color)
      return;

   /* prepare attributes */
   if(color&0x80) {    /* bright background */
      if(cfg.inverse_bold) {    /* can obtain bright background only with reverse video */
	 attrs|=A_BOLD|A_REVERSE;    /* set bright background (reversed bright foreground) */
	 fg^=bg; bg^=fg; fg^=bg;    /* swap */
	 bg=(bg-1)&7; fg=(fg+1)&7;    /* undo color rotation for new background (old forground) and apply to new forground instead */
      } else    /* can set bright background directly */
	 attrs|=A_BLINK;
   }
   if(color&0x08)    /* bright foreground */
      attrs|=A_BOLD;

   attrs|=COLOR_PAIR(bg<<3|fg);

   attrset(attrs);
}

/* leave curses full screen mode (possibly temporarily) */
void end_fullscreen(void)
{
   /* free bottom line (scroll one line) */
   move(0,0);
   deleteln();
   refresh();

   endwin();

   enable_winch();    /* allow SIGWINCH outside fullscreen mode, so it can be handled by readline() (curses will adapt when restarting fullscreen) */
}


/* terminal control sequences */
static char	*setaf;    /* foreground color */
static char	*setab;    /* background color */

/* init curses (not full screen) and read some control sequence strings
 * return screen width */
int init_curses(void)
{
   int	width;

   if(setupterm(NULL, 1, NULL)==ERR) {    /* init */
      fprintf(stderr, "Error initializing curses in raw mode.\n");
      exit(1);
   }

   width=tigetnum("cols");
   if(width<=0) {
      fprintf(stderr, "Error initializing curses in raw mode. (Can't get screen size.)\n");
      exit(1);
   }

   if(cfg.color) {
      setaf=tigetstr("setaf");    /* set foreground color */
      setab=tigetstr("setab");    /* background color */
      if(setaf==NULL || setab==NULL) {
	 cfg.color=0;
	 fprintf(stderr, "Can't get color capabilities -- starting in monochrome mode.\n\nIf this is what you want, use --monochrome option to suppress this warning. Otherwise, please check your $TERM environment variable. (See README for details.)\n\nPress <return> to continue.\n");
	 getchar();
      }

      /* determine whether terminal needs workaround to display bright background colors */
      if(cfg.inverse_bold < 0) {    /* not forced by command line option -> guess */
	 char	*term;

	 term=getenv("TERM");
	 if(term==NULL)
	    term="";

	 cfg.inverse_bold=(strstr(term, "xterm")!=NULL);    /* xterm needs it */
      }
   }

   return width;
}

/* set screen attributes (color, bold) in raw mode
 * color white is not set; standard attributes are kept instead;
 * likewise for black background */
void set_color_raw(color)
int	color;
{
   chtype	attrs=A_NORMAL;

   int	fg=color&0x07;    /* foreground color */
   int	bg=(color&0x70)>>4;    /* background */

   if(!cfg.color)
      return;

   /* prepare attributes */
   if(color&0x80) {    /* bright background */
      if(cfg.inverse_bold) {    /* can obtain bright background only with reverse video */
	 attrs|=A_BOLD|A_REVERSE;    /* set bright background (reversed bright foreground) */
	 fg^=bg; bg^=fg; fg^=bg;    /* swap */
      } else    /* can set bright background directly */
	 attrs|=A_BLINK;
   }
   if(color&0x08)    /* bright foreground */
      attrs|=A_BOLD;

   /* set */
   vidattr(attrs);
   if(bg!=COLOR_BLACK)    /* not black (or force_colors) -> set background */
      putp(tparm(setab, bg));
   if(fg!=COLOR_WHITE)    /* not white (or force_colors) -> set color */
      putp(tparm(setaf, fg));
}

/* reset screen attributes to default */
void reset_color_raw(void)
{
   if(!cfg.color)
      return;

   vidattr(A_BOLD); vidattr(A_NORMAL);    /* force reset (curses doesn't know about color changes in raw mode...) */
}
