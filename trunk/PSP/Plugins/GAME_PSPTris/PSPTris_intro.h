/*
	PSPTris - The game - Intro sequence
	Copyright (C) 2006  Jesper Sandberg

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef _PSPTRIS_INTRO_H_
#define _PSPTRIS_INTRO_H_

void PSPTris_intro();
void PSPTris_intro_destroy(void);
void PSPTris_intro_init(char *cwd);

/* These are used on the credits screen also */
void PSPTris_setup_bricks();
void PSPTris_intro_render_brick(bool front);

#endif /* _PSPTRIS_INTRO_H_ */
