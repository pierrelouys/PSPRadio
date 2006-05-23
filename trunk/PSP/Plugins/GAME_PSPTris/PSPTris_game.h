/*
	PSPTris - The game - Game
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

#ifndef _PSPTRIS_GAME_H_
#define _PSPTRIS_GAME_H_

#include "jsaTextureCache.h"

enum 	{
		GAMETYPE_CLASSIC,
		GAMETYPE_COLOR,
		GAMETYPE_ORIGINAL,
		};

enum	{
		LAYER_BRICKS,
		LAYER_ATTRIBUTES,
		LAYER_COUNT,
		};

const int	BRICK_SIZE			= 4;
const int	BRICK_COUNT			= 7;

const int	FIELD_NONE		=	0x00000000;
const int	FIELD_NOREMOVE	=	0x00000001;
const int	FIELD_REMOVE	=	0x00000002;
const int	FIELD_NEW		=	0x00000003;

static const int	PLAYFIELD_MAX_X_SIZE	= 14;
static const int	PLAYFIELD_MAX_Y_SIZE 	= 16;

typedef struct
{
	int 	x, y;
} coord;

typedef struct _moving_brick
{
	_moving_brick	*next;
	float			x, y, z;
	float			vx, vy, vz;
	float			g;
	float			size;
	int				texture_id;
	u8				opacity;
} moving_brick;

typedef struct
{
	int 	texture_id;
	int		current_shape;
	coord	current_pos;
	coord	shape1[4];
	coord	shape2[4];
	coord	shape3[4];
	coord	shape4[4];
} brick;

/* Prototypes */
void PSPTris_game_init(char *cwd);
void PSPTris_game_stop();
bool PSPTris_game_render(u32 key_state, jsaTextureCache *tcache);
void PSPTris_game_start_level(int level);
void PSPTris_game_type(int gametype);

int PSPTris_blockrand(void);
void PSPTris_game_add_moving_brick(int x, int y, int texture_id, float size);
void PSPTris_remove_rows(int columns, int rows);
void PSPTris_render_text(char *text, int x, int y);
void PSPTris_render_brick(float x, float y, float size, float tex_size, int texture_id, u32 brightness);
void PSPTris_game_render_moving_brick();

#endif /* _PSPTRIS_GAME_H_ */
