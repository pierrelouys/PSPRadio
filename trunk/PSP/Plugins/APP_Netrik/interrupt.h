/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * interrupt.h -- declarations for interrupt.c
 *
 * (C) 2002 antrik
 */
#include <setjmp.h>

extern jmp_buf	label_int;    /* return position after SIGINT received */

void init_int(void);    /* prepare constants for _int functions */

void hold_int(void);    /* put SIGINT on hold */
void enable_int(void);    /* allow SIGINT delivery */
void disable_int(void);    /* ignore SIGINT */
