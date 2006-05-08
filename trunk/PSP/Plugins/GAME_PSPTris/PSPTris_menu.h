/*
	PSPTris - The game - Game menu
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

#ifndef _PSPTRIS_MENU_H_
#define _PSPTRIS_MENU_H_

enum TEX_NAMES
	{
	TEX_FONT_01,
	TEX_BRICK_BLUE,
	TEX_BRICK_GREEN,
	TEX_BRICK_LIME,
	TEX_BRICK_ORANGE,
	TEX_BRICK_RED,
	TEX_BRICK_PINK,
	TEX_BRICK_YELLOW,
	TEX_BALL_BLUE,
	TEX_BALL_GREEN,
	TEX_BALL_GREY,
	TEX_BALL_PINK,
	TEX_BALL_RED,
	TEX_BALL_YELLOW,
	TEX_BALL_FRAME,
	TEX_BALL_SELECT,
	} ;

#define FONT_X_SIZE		(16)
#define FONT_Y_SIZE		(16)
#define FONT_X_COUNT	(256/FONT_X_SIZE)

/* Prototypes */
void PSPTris_menu_init(char *cwd);
void PSPTris_menu_destroy(void);
void PSPTris_menu(u32 key_state, u32 *key_delay);
void PSPTris_get_texture_coords(char letter, int *u, int *v);

#endif /* PSPTRIS_MENU_H_ */
