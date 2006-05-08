/*
	PSPTris - The game - Color Game
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

#ifndef _PSPTRIS_GAME_COLOR_H_
#define _PSPTRIS_GAME_COLOR_H_

#include "jsaTextureCache.h"

/* Prototypes */
void PSPTris_game_init_color(char *cwd);
bool PSPTris_game_render_color(u32 key_state, jsaTextureCache *tcache);
void PSPTris_game_start_level_color(int level);

#endif /* _PSPTRIS_GAME_COLOR_H_ */
