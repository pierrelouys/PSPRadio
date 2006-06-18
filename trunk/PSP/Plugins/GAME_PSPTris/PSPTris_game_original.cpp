/*
	PSPTris - The game - Original Game
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
#include "PSPTris_game_original.h"
#include "PSPTris_menu.h"
#include "PSPTris_highscore.h"
#include "PSPTris_audio.h"

#include "danzeff.h"

static jsaTextureCache *tcache;
static char *mCwd = NULL;

extern int	playfield[PLAYFIELD_MAX_X_SIZE+2*BRICK_SIZE][PLAYFIELD_MAX_Y_SIZE+BRICK_SIZE][LAYER_COUNT];


enum	{
		POINT_DOWN,
		POINT_DROP,
		POINT_LINE1,
		POINT_LINE2,
		POINT_LINE3,
		POINT_LINE4,
		};


static const brick	blue = {TEX_BRICK_BLUE,
							0,
							{0, 0},
							{{1,0},{1,1},{1,2},{1,3}},
							{{0,1},{1,1},{2,1},{3,1}},
							{{1,0},{1,1},{1,2},{1,3}},
							{{0,1},{1,1},{2,1},{3,1}},
							};

static const brick	green = {TEX_BRICK_GREEN,
							0,
							{0, 0},
							{{0,0},{0,1},{1,0},{1,1}},
							{{0,0},{0,1},{1,0},{1,1}},
							{{0,0},{0,1},{1,0},{1,1}},
							{{0,0},{0,1},{1,0},{1,1}},
							};

static const brick	lime = {TEX_BRICK_LIME,
							0,
							{0, 0},
							{{0,0},{0,1},{0,2},{1,2}},
							{{0,1},{1,1},{2,1},{2,0}},
							{{0,0},{1,0},{1,1},{1,2}},
							{{0,0},{0,1},{1,0},{2,0}},
							};

static const brick	orange = {TEX_BRICK_ORANGE,
							0,
							{0, 0},
							{{1,0},{1,1},{1,2},{0,2}},
							{{0,0},{1,0},{2,0},{2,1}},
							{{0,0},{1,0},{0,1},{0,2}},
							{{0,0},{0,1},{1,1},{2,1}},
							};

static const brick	red = {TEX_BRICK_RED,
							0,
							{0, 0},
							{{0,0},{0,1},{1,1},{1,2}},
							{{0,1},{1,0},{1,1},{2,0}},
							{{0,0},{0,1},{1,1},{1,2}},
							{{0,1},{1,0},{1,1},{2,0}},
							};

static const brick	pink = {TEX_BRICK_PINK,
							0,
							{0, 0},
							{{1,0},{0,1},{1,1},{0,2}},
							{{0,0},{1,0},{1,1},{2,1}},
							{{1,0},{0,1},{1,1},{0,2}},
							{{0,0},{1,0},{1,1},{2,1}},
							};

static const brick	yellow = {TEX_BRICK_YELLOW,
							0,
							{0, 0},
							{{1,0},{0,1},{1,1},{2,1}},
							{{1,0},{0,1},{1,1},{1,2}},
							{{0,1},{1,1},{1,2},{2,1}},
							{{1,0},{1,1},{2,1},{1,2}},
							};

static const brick	*bricks[] = {&blue, &green, &lime, &orange, &red, &pink, &yellow};


static const u32	DROP_SPEED = 2;

static brick		next_brick;
static brick		current_brick;

static int			time_out = 120;

static const int	LEVEL_MAX = 10;
static const int	LEVEL_INCREASE = 60*60;
static int			level_start = 1;
static int			level = 1;
static char			level_text[] = "01";
static const int	level_speed[] = {120, 90, 60, 50, 40, 30, 20, 15, 10, 5};
static int			level_counter = 0;

static u32			time_counter = 0;
static u32			tick_counter = 0;
static char			time_text[] = "00:00";

static u32			score = 0;
static char			score_text[] = "00000";
static u32			highscore_rank = 0;

static u32			line_counter = 0;
static char			line_text[] = "00000";

static bool			drop = false;
static u32			drop_counter = 0;

static bool			game_over = false;
static u8			game_over_brightness = 0xFF;

static SAMPLE		*new_level_sample = NULL;
static SAMPLE		*brick_drop_sample = NULL;
static SAMPLE		*remove_lines_sample = NULL;

#define	max(a,b)	((a > b) ? a : b)
#define	min(a,b)	((a < b) ? a : b)

static const int	PLAYFIELD_X_SIZE	= 10;
static const int	PLAYFIELD_Y_SIZE 	= 18;

#define				MIRROR_X(x)			(456 - (x))
#define 			NEXT_BRICK_X		(12)
#define 			NEXT_BRICK_Y		(116)
#define				PLAYFIELD_X			(56)
#define				PLAYFIELD_Y			(8)

static void PSPTris_render_playfield(u32 brightness)
{
	float cx = PLAYFIELD_X;
	float cy = PLAYFIELD_Y;

	for (int y = 0 ; y < PLAYFIELD_Y_SIZE ; y++)
		{
		for (int x = BRICK_SIZE ; x < PLAYFIELD_X_SIZE + BRICK_SIZE; x++)
			{
			if (playfield[x][y][LAYER_BRICKS] != 0)
				{
				PSPTris_render_brick(MIRROR_X(cy), cx, 16, 16, playfield[x][y][LAYER_BRICKS], brightness);
				}
			cx += 16;
			}
		cx = PLAYFIELD_X;
		cy += 16;
		}
}

static coord *PSPTris_game_get_shape(brick &orig_brick)
{
coord	*current = NULL;

	switch (orig_brick.current_shape)
		{
		case	0:
			current = (coord *) &orig_brick.shape1;
			break;
		case	1:
			current = (coord *) &orig_brick.shape2;
			break;
		case	2:
			current = (coord *) &orig_brick.shape3;
			break;
		case	3:
			current = (coord *) &orig_brick.shape4;
			break;
		}
	return current;
}

static void PSPTris_game_get_size(brick &orig_brick, int *x, int *y)
{
coord	*current;
int		max_x = 0;
int		min_x = 1000;
int		max_y = 0;
int		min_y = 1000;

	current = PSPTris_game_get_shape(orig_brick);

	for (int i = 0 ; i < BRICK_SIZE ; i++)
		{
		max_x = max(current[i].x, max_x); min_x = min(current[i].x, min_x);
		max_y = max(current[i].y, max_y); min_y = min(current[i].y, min_y);
		}
	*x = (max_x - min_x) + 1;
	*y = (max_y - min_y) + 1;
}

static bool PSPTris_game_collision(brick &orig_brick)
{
coord	*current;
int		x = orig_brick.current_pos.x;
int		y = orig_brick.current_pos.y;

	current = PSPTris_game_get_shape(orig_brick);

	for (int i = 0 ; i < BRICK_SIZE ; i++)
		{
		if (playfield[x + current[i].x + BRICK_SIZE][y + current[i].y][LAYER_BRICKS] != 0)
			{
			return true;
			}
		}
	return false;
}

static bool PSPTris_get_new_brick()
{
int	x, y;

	memcpy(&current_brick, &next_brick, sizeof(current_brick));
	PSPTris_game_get_size(current_brick, &x, &y);
	current_brick.current_pos.x = PLAYFIELD_X_SIZE / 2 - (BRICK_SIZE / 2 - x / 2);
	/* Get next brick */
	memcpy(&next_brick, bricks[PSPTris_blockrand()], sizeof(next_brick));
	return PSPTris_game_collision(current_brick);
}

void PSPTris_game_init_original(char *cwd)
{
int		ret_value;
char	filename[MAXPATHLEN];

	mCwd = cwd;

	sprintf(filename, "%s/Samples/brick_drop.wav", cwd);
	ret_value = PSPTris_audio_load_sample(filename, &brick_drop_sample);
	if (ret_value != PSPTRIS_AUDIO_ERR_NONE)
		{
		printf("Couldn't load brick_drop.wav\n");
		}

	sprintf(filename, "%s/Samples/new_level.wav", cwd);
	ret_value = PSPTris_audio_load_sample(filename, &new_level_sample);
	if (ret_value != PSPTRIS_AUDIO_ERR_NONE)
		{
		printf("Couldn't load new_level.wav\n");
		}

	sprintf(filename, "%s/Samples/lines.wav", cwd);
	ret_value = PSPTris_audio_load_sample(filename, &remove_lines_sample);
	if (ret_value != PSPTRIS_AUDIO_ERR_NONE)
		{
		printf("Couldn't load lines.wav\n");
		}

	for (int y = 0 ; y < PLAYFIELD_Y_SIZE ; y++)
		{
		/* fill static fields */
		for (int x = 0 ; x < BRICK_SIZE ; x++)
			{
			playfield[x][y][LAYER_BRICKS] 									= PSPTris_blockrand()+1;
			playfield[x][y][LAYER_ATTRIBUTES] 								= FIELD_NOREMOVE;
			playfield[x+BRICK_SIZE+PLAYFIELD_X_SIZE][y][LAYER_BRICKS] 		= PSPTris_blockrand()+1;
			playfield[x+BRICK_SIZE+PLAYFIELD_X_SIZE][y][LAYER_ATTRIBUTES] 	= FIELD_NOREMOVE;
			}

		for (int x = BRICK_SIZE ; x < PLAYFIELD_X_SIZE + BRICK_SIZE; x++)
			{
			playfield[x][y][LAYER_BRICKS] 		= 0;
			playfield[x][y][LAYER_ATTRIBUTES] 	= FIELD_NONE;
			}
		}

	for (int y = PLAYFIELD_Y_SIZE ; y < PLAYFIELD_Y_SIZE + BRICK_SIZE; y++)
		{
		/* fill static fields */
		for (int x = 0 ; x < PLAYFIELD_X_SIZE + 2*BRICK_SIZE ; x++)
			{
			playfield[x][y][LAYER_BRICKS] 		= PSPTris_blockrand()+1;
			playfield[x][y][LAYER_ATTRIBUTES] 	= FIELD_NOREMOVE;
			}
		}

	level = level_start;
	sprintf(level_text, "%02d", level);

	score = 0;
	sprintf(score_text, "%05d", score);

	line_counter = 0;
	sprintf(line_text, "%05d", line_counter);

	time_counter = 0;
	tick_counter = 0;
	sprintf(time_text, "%02d:%02d", 0, 0);

	/* Get initial next brick */
	memcpy(&next_brick, bricks[PSPTris_blockrand()], sizeof(next_brick));
	(void)PSPTris_get_new_brick();

	drop = false;
	drop_counter = 0;

	game_over = false;
	game_over_brightness = 0xFF;
	sceKernelDcacheWritebackAll();
}

static void PSPTris_game_rotate_brick_left(brick &orig_brick)
{
	orig_brick.current_shape++;
	orig_brick.current_shape = orig_brick.current_shape % 4;
}

static void PSPTris_game_rotate_brick_right(brick &orig_brick)
{
	if (orig_brick.current_shape == 0)
		{
		orig_brick.current_shape = 3;
		}
	else
		{
		orig_brick.current_shape--;
		}
}

static void PSPTris_game_update_score(int event)
{
	switch(event)
		{
		case POINT_DOWN:
			score += 1;
			break;
		case POINT_DROP:
			score += 5;
			break;
		case POINT_LINE1:
			score += 10 * level;
			break;
		case POINT_LINE2:
			score += 40 * level;
			break;
		case POINT_LINE3:
			score += 80 * level;
			break;
		case POINT_LINE4:
			score += 120 * level;
			break;
		}
	sprintf(score_text, "%05d", score);
}

static void PSPTris_scan_rows(void)
{
bool 	fullline;
u32		linecount = 0;

	for (int y = 0 ; y < PLAYFIELD_Y_SIZE ; y++)
		{
		fullline = true;
		for (int x = BRICK_SIZE ; x < PLAYFIELD_X_SIZE + BRICK_SIZE; x++)
			{
			if (!playfield[x][y][LAYER_BRICKS])
				{
				/* Empty field -> Skip this row */
				fullline = false;
				break;
				}
			}
		if (fullline)
			{
			linecount++;
			for (int x = BRICK_SIZE ; x < PLAYFIELD_X_SIZE + BRICK_SIZE; x++)
				{
				playfield[x][y][LAYER_ATTRIBUTES] = FIELD_REMOVE;
				/* Add field to list of moving bricks */
				PSPTris_game_add_moving_brick(128 + (x-BRICK_SIZE) * 16, 8 + (y * 16), playfield[x][y][LAYER_BRICKS], 16);
				}
			}
		}

	switch (linecount)
		{
		case	1:
			PSPTris_audio_play_sample(remove_lines_sample);
			PSPTris_game_update_score(POINT_LINE1);
			break;
		case	2:
			PSPTris_audio_play_sample(remove_lines_sample);
			PSPTris_game_update_score(POINT_LINE2);
			break;
		case	3:
			PSPTris_audio_play_sample(remove_lines_sample);
			PSPTris_game_update_score(POINT_LINE3);
			break;
		case	4:
			PSPTris_audio_play_sample(remove_lines_sample);
			PSPTris_game_update_score(POINT_LINE4);
			break;
		}

	line_counter += linecount;
	sprintf(line_text, "%05d", line_counter);
}

static void PSPTris_render_next_brick()
{
	coord	*shape;
	float 	cx = NEXT_BRICK_X;
	float 	cy = NEXT_BRICK_Y;
	int		x;
	int		y;

	shape = PSPTris_game_get_shape(next_brick);
	PSPTris_game_get_size(next_brick, &x, &y);

	/* hack for the blue brick :-) */
	if (next_brick.texture_id == TEX_BRICK_BLUE)
		{
		cx = NEXT_BRICK_X + ((32 - x * 8) / 2) - 8;
		cy = NEXT_BRICK_Y + ((32 - y * 8) / 2);
		}
	else
		{
		cx = NEXT_BRICK_X + ((32 - x * 8) / 2);
		cy = NEXT_BRICK_Y + ((32 - y * 8) / 2);
		}

	for (int i = 0 ; i < BRICK_SIZE ; i++)
		{
		PSPTris_render_brick(MIRROR_X(cy + shape[i].y * 8), cx + shape[i].x * 8, 8, 16, next_brick.texture_id, 0xFFFFFFFF);
		}
}



static void PSPTris_render_current_brick()
{
	coord	*shape;
	float cx = PLAYFIELD_X + 16 * current_brick.current_pos.x;
	float cy = PLAYFIELD_Y + 16 * current_brick.current_pos.y;

	shape = PSPTris_game_get_shape(current_brick);

	for (int i = 0 ; i < BRICK_SIZE ; i++)
		{
		PSPTris_render_brick(MIRROR_X(cy + shape[i].y * 16), cx + shape[i].x * 16, 16, 16, current_brick.texture_id, 0xFFFFFFFF);
		}
}

static void PSPTris_store_brick(void)
{
	coord	*shape;
	int x = current_brick.current_pos.x;
	int y = current_brick.current_pos.y;

	shape = PSPTris_game_get_shape(current_brick);

	for (int i = 0 ; i < BRICK_SIZE ; i++)
		{
		playfield[x + shape[i].x + BRICK_SIZE][y + shape[i].y][LAYER_BRICKS] = current_brick.texture_id;
		}
}

void PSPTris_game_start_level_original(int level)
{
	level_start = level;
}

static void PSPTris_update_time()
{
	tick_counter++;

	if (tick_counter == 60)
		{
		tick_counter = 0;
		time_counter++;
		sprintf(time_text, "%02d:%02d", time_counter / 60, time_counter % 60);
		}
}

void PSPTris_game_stop_original()
{
	PSPTris_audio_free_sample(brick_drop_sample);
	PSPTris_audio_free_sample(new_level_sample);
	PSPTris_audio_free_sample(remove_lines_sample);
	danzeff_free();
}

bool PSPTris_game_render_original(u32 key_state, jsaTextureCache *mytcache)
{
bool	exit_game = false;

	tcache = mytcache;
	sceGuDisable(GU_DEPTH_TEST);
	sceGuEnable(GU_TEXTURE_2D);

	if (!game_over)
		{
		PSPTris_render_next_brick();

		level_counter++;
		if (level_counter == LEVEL_INCREASE)
			{
			level_counter = 0;
			if (level < LEVEL_MAX)
				{
				PSPTris_audio_play_sample(new_level_sample);
				level++;
				sprintf(level_text, "%02d", level);
				}
			}

		/* Check for drop */
		if ((key_state & PSP_CTRL_RIGHT) && !drop)
			{
			drop = true;
			PSPTris_game_update_score(POINT_DROP);
			}

		if (!drop)
			{
			/* Check for time-out -> Force brick down */
			if (time_out == 0)
				{
				key_state |= PSP_CTRL_LEFT;
				time_out = level_speed[level-1];
				}
			else
				{
				time_out--;
				}

			/* Check for rotation left */
			if ((key_state & PSP_CTRL_LTRIGGER) || (key_state & PSP_CTRL_CROSS))
				{
				/* Rotate current brick */
				PSPTris_game_rotate_brick_left(current_brick);
				/* Check for collision */
				if (PSPTris_game_collision(current_brick))
					{
					/* On collision -> back to original state */
					PSPTris_game_rotate_brick_right(current_brick);
					}
				}

			/* Check for rotation right */
			if (key_state & PSP_CTRL_RTRIGGER)
				{
				/* Rotate current brick */
				PSPTris_game_rotate_brick_right(current_brick);
				/* Check for collision */
				if (PSPTris_game_collision(current_brick))
					{
					/* On collision -> back to original state */
					PSPTris_game_rotate_brick_left(current_brick);
					}
				}

			/* Check for movement -> RIGHT */
			if (key_state & PSP_CTRL_DOWN)
				{
				if (current_brick.current_pos.x < PLAYFIELD_X_SIZE)
					{
					current_brick.current_pos.x++;
					/* Check for collision */
					if (PSPTris_game_collision(current_brick))
						{
						current_brick.current_pos.x--;
						}
					}
				}
			/* Check for movement -> LEFT */
			if (key_state & PSP_CTRL_UP)
				{
				if (current_brick.current_pos.x > -BRICK_SIZE)
					{
					current_brick.current_pos.x--;
					/* Check for collision */
					if (PSPTris_game_collision(current_brick))
						{
						current_brick.current_pos.x++;
						}
					}
				}
			}
		else
			{
			drop_counter++;

			if (drop_counter == DROP_SPEED)
				{
				drop_counter = 0;
				key_state = PSP_CTRL_LEFT;
				}
			}

		/* Check for movement -> DOWN */
		if (key_state & PSP_CTRL_LEFT)
			{
			if (current_brick.current_pos.y < PLAYFIELD_Y_SIZE)
				{
				current_brick.current_pos.y++;
				/* Check for collision */
				if (PSPTris_game_collision(current_brick))
					{
					PSPTris_audio_play_sample(brick_drop_sample);
					drop = false;
					current_brick.current_pos.y--;
					/* Move brick to playfield */
					PSPTris_store_brick();
					/* Scan rows */
					PSPTris_scan_rows();
					/* Remove rows */
					PSPTris_remove_rows(PLAYFIELD_X_SIZE, PLAYFIELD_Y_SIZE);
					/* Get new brick */
					PSPTris_get_new_brick();

					/* Check for collision -> GAME OVER */
					if (PSPTris_game_collision(current_brick))
						{
						game_over = true;
						PSPTris_game_stop_music();
						PSPTris_game_start_music(mCwd, "/Music/endgame.mod");
						highscore_rank = PSPTris_highscore_check(score, GAMETYPE_ORIGINAL);
						}
					}
				else
					{
					PSPTris_game_update_score(POINT_DOWN);
					}
				}
			}
		PSPTris_render_playfield(0xFFFFFFFF);

		PSPTris_render_text_vertical(score_text, 100,  18);
		PSPTris_render_text_vertical(level_text,  44,  42);
		PSPTris_render_text_vertical(line_text,	 100, 172);
		PSPTris_update_time();
		PSPTris_render_text_vertical(time_text,   44, 172);

		if (!game_over)
			{
			PSPTris_render_current_brick();
			PSPTris_game_render_moving_brick(true);
			}
		}
	/* Game over */
	else
		{
		u32				brightness;

		if (game_over_brightness > 0x60)
			{
			game_over_brightness--;
			}
		brightness = (game_over_brightness << 24) | (game_over_brightness << 16) | (game_over_brightness << 8) | game_over_brightness;

		PSPTris_render_playfield(brightness);

		PSPTris_render_text_vertical(score_text, 100,  18);
		PSPTris_render_text_vertical(level_text,  44,  42);
		PSPTris_render_text_vertical(line_text,	 100, 172);
		PSPTris_render_text_vertical(time_text,   44, 172);

		/* If we made it to the highscore rank, then get input */
		if (highscore_rank > 0)
			{
			static bool first_time = true;
			static bool name_entered = false;
			static highscore_str	highscore;
			SceCtrlData pad_data;

			PSPTris_render_text_vertical("WELL DONE",		420, 56 + 0 * 16 + 8);
			PSPTris_render_text_vertical("YOU MADE",		380, 56 + 1 * 16 + 0);
			PSPTris_render_text_vertical("IT TO THE",		360, 56 + 0 * 16 + 8);
			PSPTris_render_text_vertical("HIGHSCORE",		340, 56 + 0 * 16 + 8);

			if (first_time)
				{
				highscore.score = score;
				highscore.rank  = highscore_rank;
				strcpy(highscore.name, "AAA");
				first_time = false;
				/* Initialize OSK */
				danzeff_load();
				danzeff_moveTo(128, 62);
				danzeff_set_orientation(DANZEFF_ORIENT_RIGHT);
				}

			if (!name_entered)
				{
				unsigned int	entered_char;
				static int		cur_char = 0;

				sceCtrlReadBufferPositive(&pad_data, 1);
				entered_char = danzeff_readInput(pad_data);
				PSPTris_render_text_vertical(highscore.name, 300, 56 + 3 * 16 + 8);
				danzeff_render();

				/* Get input from OSK and convert to uppercase */
				if (isalnum(entered_char))
					{
					entered_char = toupper(entered_char);
					highscore.name[cur_char] = (char) entered_char;
					}

				if ((entered_char == DANZEFF_RIGHT) || (isalnum(entered_char)))
					{
					if (cur_char < 2)
						{
						cur_char++;
						}
					}
				if (entered_char == DANZEFF_LEFT)
					{
					if (cur_char > 0)
						{
						cur_char--;
						}
					}

				if (entered_char == '\n')
					{
					first_time = true;
					name_entered = false;
					cur_char = 0;
					PSPTris_highscore_store(&highscore, GAMETYPE_ORIGINAL);
					danzeff_free();
					exit_game = true;
					}
				}
			}
		/* We didn't make it to the highscore rank */
		else
			{
			PSPTris_render_text_vertical("SORRY",			360, 56 + 2 * 16 + 8);
			PSPTris_render_text_vertical("YOU NEED",		320, 56 + 1 * 16 + 0);
			PSPTris_render_text_vertical("MORE",			300, 56 + 3 * 16 + 0);
			PSPTris_render_text_vertical("PRACTICE",		280, 56 + 1 * 16 + 0);
			PSPTris_render_text_vertical("TO",				260, 56 + 4 * 16 + 0);
			PSPTris_render_text_vertical("REACH THE",		240, 56 + 0 * 16 + 8);
			PSPTris_render_text_vertical("HIGHSCORE",		220, 56 + 0 * 16 + 8);

			/* Check for exit */
			if (key_state & PSP_CTRL_CROSS)
				{
				exit_game = true;
				}
			}
		}
	sceGuEnable(GU_DEPTH_TEST);

	if (exit_game)
		{
		PSPTris_game_stop_original();
		}

	return exit_game;
}
