/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * winch.h -- declarations for winch.c
 *
 * (C) 2003 antrik
 */

void hold_winch(void);    /* put SIGWINCH on hold */
void enable_winch(void);    /* allow SIGWINCH delivery */
