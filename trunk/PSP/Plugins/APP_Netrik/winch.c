/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/

/*
 * winch.c -- SIGWINCH handling
 *
 * (C) 2003 antrik
 *
 * This file contains functions for setting method of processing (immediate
 * delivery or hold) the winch signal (window change), raised by the terminal
 * when the terminal size changes. (e.g. xterm resize)
 */

#include <signal.h>
#include <stdlib.h>

#include "winch.h"

/* block signals, so they will neither be deliverd immediately nor discared;
 * instead, they will arrive as soon as enable_winch() is called */
void hold_winch(void)
{
   static sigset_t	winch_mask;    /* signal mask with only SIGWINCH set */

   sigemptyset(&winch_mask);
   sigaddset(&winch_mask, SIGWINCH);

   sigprocmask(SIG_BLOCK, &winch_mask, NULL);
}

/* release SIGWINCH (allows signals now, but also releases pending signals that
 * arrived during the hold period) */
void enable_winch(void)
{
   static sigset_t	winch_mask;    /* signal mask with only SIGWINCH set */

   sigemptyset(&winch_mask);
   sigaddset(&winch_mask, SIGWINCH);

   sigprocmask(SIG_UNBLOCK, &winch_mask, NULL);
}
