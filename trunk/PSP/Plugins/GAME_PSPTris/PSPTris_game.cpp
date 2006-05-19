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

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <pspgu.h>
#include <pspgum.h>
#include <psprtc.h>
#include <pspctrl.h>
#include <pspkernel.h>

#include "PSPTris.h"
#include "PSPTris_game.h"
#include "PSPTris_game_classic.h"
#include "PSPTris_game_color.h"
#include "PSPTris_menu.h"
#include "PSPTris_highscore.h"
#include "PSPTris_audio.h"

#include "danzeff.h"

static jsaTextureCache *tcache;

int	playfield[PLAYFIELD_MAX_X_SIZE+2*BRICK_SIZE][PLAYFIELD_MAX_Y_SIZE+BRICK_SIZE][LAYER_COUNT];

static int			gametype = GAMETYPE_CLASSIC;

static moving_brick	*dynamic_brick_list = NULL;
static moving_brick	*last_in_list = NULL;


/* From POSIX  1003.1-2003 (modified) */

static unsigned long next = 114343;

int PSPTris_blockrand(void)
{
	next = next * 1103515245 + 12345;
	return((unsigned)(next/65536) % BRICK_COUNT);
}

static float myrandf(void)
{
	next = next * 1103515245 + 12345;
	return((unsigned)(next/65536) % 1024);
}

void PSPTris_game_start_music(char *cwd, char *name)
{
#if !defined(DYNAMIC_BUILD)
	char path[1024];

	/* Start playing menu module */
	sprintf(path, "%s%s", cwd, name);
	printf("Loading : %s\n", path);
	PSPTris_audio_play_module(path);
#endif /* !defined(DYNAMIC_BUILD) */
}

void PSPTris_game_stop_music()
{
#if !defined(DYNAMIC_BUILD)
	PSPTris_audio_stop_module();
#endif /* !defined(DYNAMIC_BUILD) */
}

#if !defined(DYNAMIC_BUILD)
int PSPTris_get_ingame_mods(char *cwd, char *prefix, bool play, int number)
{
	int dfd = 0;
	int modules = 0;
	SceIoDirent direntry;
	char path[1024];

	sprintf(path, "%s/Music/", cwd);

	dfd = sceIoDopen(path);

	if (dfd >= 0)
	{
		memset(&direntry, 0, sizeof(SceIoDirent));
		while(sceIoDread(dfd, &direntry) > 0)
		{
			if((direntry.d_stat.st_attr & FIO_SO_IFREG))
			{
				if (strcmp(direntry.d_name, ".") == 0)
					continue;
				else if (strcmp(direntry.d_name, "..") == 0)
					continue;

				if (strncmp(direntry.d_name, prefix, strlen(prefix)) == 0)
				{
					if (play)
						{
						if (modules == number)
							{
							PSPTris_game_start_music(path, direntry.d_name);
							return 0;
							}
						}
					else
						{
						}
					modules++;
				}
			}
		}
		sceIoDclose(dfd);
	}
	else
	{
		printf("Mod scanner: Unable to open '%s' Directory! (Error=0x%x)\n", path, dfd);
	}

	return modules;
}
#endif /*!defined(DYNAMIC_BUILD)*/

void PSPTris_game_init(char *cwd)
{
u64		ticks;
#if !defined(DYNAMIC_BUILD)
int		nbr_modules;
#endif /*!defined(DYNAMIC_BUILD)*/

	/* Seed the rand generator */
	(void)sceRtcGetCurrentTick(&ticks);
	next = (unsigned long) ticks;

#if !defined(DYNAMIC_BUILD)
	/* Search for ingame modules */
	nbr_modules = PSPTris_get_ingame_mods(cwd, "ingame_", false, 0);
	/* Select random ingame module */
	nbr_modules = (int)myrandf() % nbr_modules;
	/* Play ingame module */
	PSPTris_get_ingame_mods(cwd, "ingame_", true, nbr_modules);
#endif /*!defined(DYNAMIC_BUILD)*/

	if (gametype == GAMETYPE_CLASSIC)
		{
		PSPTris_game_init_classic(cwd);
		}
	else
		{
		PSPTris_game_init_color(cwd);
		}

	sceKernelDcacheWritebackAll();
}

void PSPTris_game_add_moving_brick(int x, int y, int texture_id, float size)
{
moving_brick	*new_brick;

	new_brick = (moving_brick *) malloc(sizeof(moving_brick));

	new_brick->next 		= NULL;
	new_brick->size			= size;
	new_brick->x			= x;
	new_brick->y			= y;
	new_brick->z			= 0;
	new_brick->vx			= (myrandf() / 256.0f) - 2.0f;
	new_brick->vy			= (myrandf() / 256.0f) - 2.0f;
	new_brick->vz			= (myrandf() / 256.0f) - 2.0f;
	new_brick->g			= 0.05;
	new_brick->texture_id	= texture_id;
	new_brick->opacity		= 0xFF;

	if (dynamic_brick_list == NULL)
		{
		dynamic_brick_list = new_brick;
		}
	else
		{
		last_in_list->next = new_brick;
		}
	last_in_list = new_brick;
}

void PSPTris_shift_column(int column, int rows)
{
	/* Shift from bottom and up */
	for (int y = rows - 1; y >= 0 ; y--)
		{
		if (playfield[column][y][LAYER_ATTRIBUTES] && FIELD_REMOVE)
			{
			/* Shift down rows above this one */
			for (int shift_y = y ; shift_y > 0 ; shift_y--)
				{
				playfield[column][shift_y][LAYER_ATTRIBUTES]	= playfield[column][shift_y-1][LAYER_ATTRIBUTES];
				playfield[column][shift_y][LAYER_BRICKS]		= playfield[column][shift_y-1][LAYER_BRICKS];
				}
			/* Insert empty brick at the top */
			playfield[column][0][LAYER_ATTRIBUTES]	= FIELD_NONE;
			playfield[column][0][LAYER_BRICKS]		= 0;
			/* Repeat for this row (recursive) */
			PSPTris_shift_column(column, rows);
			}
		}
}

void PSPTris_remove_rows(int columns, int rows)
{
	for (int x = BRICK_SIZE ; x < columns + BRICK_SIZE; x++)
		{
		PSPTris_shift_column(x, rows);
		}
}

void PSPTris_render_text(char *text, int x, int y)
{
	int	length;
	int	u, v;

	length = strlen(text);

	sceGuEnable(GU_TEXTURE_2D);
	sceGuAlphaFunc( GU_GREATER, 0, 0xff );
	sceGuEnable( GU_ALPHA_TEST );
	sceGuTexFunc(GU_TFX_BLEND,GU_TCC_RGBA);
	sceGuTexEnvColor(0xFF000000);
	sceGuBlendFunc( GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0 );
	sceGuEnable( GU_BLEND );
	(void)tcache->jsaTCacheSetTexture(TEX_FONT_01);
	sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);

	for (int i = 0 ; i < length ; i++)
		{
		PSPTris_get_texture_coords(text[i], &u, &v);

		struct NCVertex* c_vertices = (struct NCVertex*)sceGuGetMemory(2 * sizeof(struct NCVertex));

		c_vertices[0].u 		= u;
		c_vertices[0].v 		= v;
		c_vertices[0].x 		= x;
		c_vertices[0].y 		= y;
		c_vertices[0].z 		= 0;
		c_vertices[0].color 	= 0xFFFFFFFF;

		c_vertices[1].u 		= u + FONT_X_SIZE;
		c_vertices[1].v 		= v + FONT_Y_SIZE;
		c_vertices[1].x 		= x + FONT_X_SIZE;
		c_vertices[1].y 		= y + FONT_Y_SIZE;
		c_vertices[1].z 		= 0;
		c_vertices[1].color 	= 0xFFFFFFFF;

		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,c_vertices);

		x += FONT_X_SIZE;
		}

	sceGuDisable( GU_BLEND );
	sceGuDisable( GU_ALPHA_TEST );
	sceGuDisable(GU_TEXTURE_2D);
}

void PSPTris_render_brick(float x, float y, float size, float tex_size, int texture_id, u32 brightness)
{
	struct NCVertex* c_vertices = (struct NCVertex*)sceGuGetMemory(2 * sizeof(struct NCVertex));

	c_vertices[0].u 	= 0;
	c_vertices[0].v 	= 0;
	c_vertices[0].x 	= x;
	c_vertices[0].y 	= y;
	c_vertices[0].z 	= 0;
	c_vertices[0].color = brightness;

	c_vertices[1].u 	= tex_size;
	c_vertices[1].v 	= tex_size;
	c_vertices[1].x 	= x + size;
	c_vertices[1].y 	= y + size;
	c_vertices[1].z 	= 0;
	c_vertices[1].color = brightness;
	sceGuEnable(GU_TEXTURE_2D);
	(void)tcache->jsaTCacheSetTexture(texture_id);
	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,c_vertices);
	sceGuDisable(GU_TEXTURE_2D);
}

void PSPTris_game_remove_moving_brick(moving_brick *finished_brick)
{
moving_brick	*temp = dynamic_brick_list;
moving_brick	*previous = NULL;

	while (temp != NULL)
		{
		if (finished_brick == temp)
			{
			/* Remove element from list, taking first element into account */
			if (previous != NULL)
				{
				previous->next = temp->next;
				}
			else
				{
				dynamic_brick_list = temp->next;
				}
			/* If we remove the last element, then update last pointer */
			if (temp->next == NULL)
				{
				last_in_list = previous;
				}
			free(temp);
			break;
			}
		previous = temp;
		temp = temp->next;
		}
}

void PSPTris_game_update_moving_brick()
{
moving_brick	*temp = dynamic_brick_list;

	/* Remove bricks outside the screen */
	while (temp != NULL)
		{
		if ((temp->x < -16) || (temp->x > 480) || (temp->y < -16) || (temp->y > 272))
			{
			PSPTris_game_remove_moving_brick(temp);
			/* TODO : Make a proper list to avoid this redundancy */
			temp = dynamic_brick_list;
			if (temp == NULL)
				{
				break;
				}
			}
		temp = temp->next;
		}

	temp = dynamic_brick_list;

	while (temp != NULL)
		{
		/* add velocity to position */
		temp->x += temp->vx;
		temp->y += temp->vy;
		temp->z += temp->vz;
		/* add gravity to y position */
		temp->vy += temp->g;

		/* Update color / opacity */
		if (temp->opacity > 0x00)
			{
			temp->opacity--;
			}

		temp = temp->next;
		}
}

void PSPTris_game_render_moving_brick()
{
moving_brick	*temp = dynamic_brick_list;

	sceGuTexFunc(GU_TFX_BLEND,GU_TCC_RGBA);
	sceGuTexEnvColor(0xFF000000);
	sceGuBlendFunc( GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0 );
	sceGuEnable( GU_BLEND );
	sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);
	while (temp != NULL)
		{
		PSPTris_render_brick(temp->x, temp->y, temp->size, temp->size, temp->texture_id, (temp->opacity << 24) | 0x00FFFFFF);
		temp = temp->next;
		}
	sceGuDisable( GU_BLEND );
	/* Update positions */
	PSPTris_game_update_moving_brick();
}

void PSPTris_game_start_level(int level)
{
	if (gametype == GAMETYPE_CLASSIC)
		{
		PSPTris_game_start_level_classic(level);
		}
	else
		{
		PSPTris_game_start_level_color(level);
		}
}

void PSPTris_game_type(int type)
{
	gametype = type;
}

void PSPTris_game_stop()
{
	PSPTris_game_stop_music();
	if (gametype == GAMETYPE_CLASSIC)
		{
		PSPTris_game_stop_classic();
		}
	else
		{
		PSPTris_game_stop_color();
		}
}

bool PSPTris_game_render(u32 key_state, jsaTextureCache *mytcache)
{
bool	exit_game = false;
static	bool game_pause = false;

	tcache = mytcache;

	/* Check for pause exit */
	if (key_state & PSP_CTRL_START)
		{
		game_pause = true;
		MikMod_DisableOutput();
		}

	if (!game_pause)
		{
		if (gametype == GAMETYPE_CLASSIC)
			{
			exit_game = PSPTris_game_render_classic(key_state, mytcache);
			}
		else
			{
			exit_game = PSPTris_game_render_color(key_state, mytcache);
			}
		}
	else
		{
		/* Check for pause exit */
		if (key_state & PSP_CTRL_CROSS)
			{
			game_pause = false;
			MikMod_EnableOutput();
			}
		/* Check for exit to menu */
		if (key_state & PSP_CTRL_CIRCLE)
			{
			game_pause = false;
			exit_game = true;
			PSPTris_game_stop();
			}
		PSPTris_render_text("PAUSE",	128 + 4 * 16 + 1 * 8,  82);
		PSPTris_render_text("X TO CONTINUE",	128 + 0 * 16 + 1 * 8,  142);
		PSPTris_render_text("O TO EXIT",	128 + 2 * 16 + 1 * 8,  182);
		}
	return exit_game;
}
