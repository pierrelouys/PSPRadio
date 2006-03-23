/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * cfg.h -- function declaration for cfg.c and structure definition
 *             for storing configuration values
 *
 * (C) 2001 Patrice Neff
 */

#include "cfg.h"

/* read command line and write values into the struct */
int config_cmdln(int argc, char *argv[]);
