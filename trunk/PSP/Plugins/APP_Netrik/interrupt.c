/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * interrupt.c -- SIGINT handling
 *
 * (C) 2002 antrik
 *
 * This file contains functions for handling the interrupt signal (^C),
 * which allows aborting a file/HTTP loading operation.
 */
#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>

#include "interrupt.h"

static void int_handler(int sig);    /* signal handler for SIGINT (^C) */

jmp_buf			label_int;    /* return position after SIGINT received */

static sigset_t		int_mask;    /* signal mask with only SIGINT set */
static sigset_t		empty_mask;    /* mask with no signals */
static struct sigaction	int_action;    /* SIGINT handling */

/* prepare constants and data structures for SIGINT handling
 * (must be called before first usage of the other _int functions) */
void init_int(void)
{
   sigemptyset(&empty_mask);

   sigemptyset(&int_mask);
   sigaddset(&int_mask, SIGINT);

   int_action.sa_mask=empty_mask;    /* don't block any other signals during handler execution */
   int_action.sa_flags=0;
}

/* SIGINT handler;
 * simply jumps to the position last stored in "label_int" with setjmp() */
static void int_handler(sig)
int	sig;
{
   sig=0;    /* avoid warning about unused "sig"... */

   longjmp(label_int, 1);
}

/* block signals, so they will neither be deliverd immediately nor discared;
 * instead, they will arrive as soon as enable_int() is called */
void hold_int(void)
{
   sigprocmask(SIG_BLOCK, &int_mask, NULL);
}

/* release SIGINT (allows breaks now, but also releases pending signals that
 * arrived during the hold period) */
void enable_int(void)
{
   int_action.sa_handler=int_handler;
   sigaction(SIGINT, &int_action, NULL);
   sigprocmask(SIG_UNBLOCK, &int_mask, NULL);
}

/* completely ignore all SIGINT until next call of hold_int() or enable_int()
 * (don't put on hold) */
void disable_int(void)
{
   int_action.sa_handler=SIG_IGN;
   sigaction(SIGINT, &int_action, NULL);
   sigprocmask(SIG_UNBLOCK, &int_mask, NULL);    /* release signal, so interrupts won't be put on hold and activated on next enable_int() */
}
