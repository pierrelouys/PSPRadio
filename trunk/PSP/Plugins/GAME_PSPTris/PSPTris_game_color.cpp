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
#include "PSPTris_game_color.h"
#include "PSPTris_menu.h"
#include "PSPTris_highscore.h"
#include "PSPTris_audio.h"

#include "danzeff.h"

static jsaTextureCache *tcache;

extern int	playfield[PLAYFIELD_MAX_X_SIZE+2*BRICK_SIZE][PLAYFIELD_MAX_Y_SIZE+BRICK_SIZE][LAYER_COUNT];

static const int	PLAYFIELD_X_SIZE	= 7;
static const int	PLAYFIELD_Y_SIZE 	= 8;

static const int	LEVEL_MAX = 10;
static const int	LEVEL_INCREASE = 60*60;
static u32			start_level = 1;
static s32			level = 1;
static char			level_text[] = "01";
static s32			level_counter = 0;

static u32			score = 0;
static char			score_text[] = "00000";
static u32			highscore_rank = 0;
static u32			bricks = 0;
static char			bricks_text[] = "00000";
static u32			timer_ticks = 0;
static u32			timer = 0;
static char			timer_text[] = "000";

static bool			game_over = false;
static u32			game_over_brightness = 0xFF;

static	u32			select_counter = 0;
static	u32			hint_counter = 0;

static float		cursor_x = 128 + (PLAYFIELD_X_SIZE / 2) * 16;
static float		cursor_y = 8 + (PLAYFIELD_Y_SIZE / 2) * 16;
static int			cursor_pf_x = PLAYFIELD_X_SIZE / 2;
static int			cursor_pf_y = PLAYFIELD_Y_SIZE / 2;
static float		speed_x = 0;
static float		speed_y = 0;

static u32			equal_colors = 0;

static SAMPLE		*remove_lines_sample = NULL;
static SAMPLE		*new_level_sample = NULL;

static void PSPTris_game_restart_time()
{
	timer = ((LEVEL_MAX+1) - level) * 2;
}

void PSPTris_game_init_color(char *cwd)
{
int		ret_value;
char	filename[MAXPATHLEN];

	sprintf(filename, "%s/Samples/lines.wav", cwd);
	ret_value = PSPTris_audio_load_sample(filename, &remove_lines_sample);
	if (ret_value != PSPTRIS_AUDIO_ERR_NONE)
		{
		printf("Couldn't load lines.wav\n");
		}
	sprintf(filename, "%s/Samples/new_level.wav", cwd);
	ret_value = PSPTris_audio_load_sample(filename, &new_level_sample);
	if (ret_value != PSPTRIS_AUDIO_ERR_NONE)
		{
		printf("Couldn't load new_level.wav\n");
		}

	for (int y = 0 ; y < PLAYFIELD_Y_SIZE ; y++)
		{
		for (int x = BRICK_SIZE ; x < PLAYFIELD_X_SIZE + BRICK_SIZE; x++)
			{
			playfield[x][y][LAYER_BRICKS] 		= (PSPTris_blockrand()%6)+8;
			playfield[x][y][LAYER_ATTRIBUTES] 	= FIELD_NONE;
			}
		}

	highscore_rank = 0;
	score = 0;
	sprintf(score_text, "%05d", score);
	bricks = 0;
	sprintf(bricks_text, "%05d", bricks);
	level_counter = 0;
	level = start_level;
	sprintf(level_text, "%02d", level);
	PSPTris_game_restart_time();
	timer_ticks = 0;
	sprintf(timer_text, "%03d", timer);

	equal_colors = 0;

	cursor_x = 128 + (PLAYFIELD_X_SIZE / 2) * 32;
	cursor_y = 8 + (PLAYFIELD_X_SIZE / 2) * 32;
	cursor_pf_x = PLAYFIELD_X_SIZE / 2;
	cursor_pf_y = PLAYFIELD_X_SIZE / 2;
	speed_x = 0;
	speed_y = 0;

	select_counter = 0;
	hint_counter = 0;

	game_over = false;
	game_over_brightness = 0xFF;

	sceKernelDcacheWritebackAll();
}

static void PSPTris_game_clear_markings()
{
	equal_colors = 0;
	for (int y = 0 ; y < PLAYFIELD_Y_SIZE ; y++)
		{
		for (int x = BRICK_SIZE ; x < PLAYFIELD_X_SIZE + BRICK_SIZE; x++)
			{
			playfield[x][y][LAYER_ATTRIBUTES] 	= FIELD_NONE;
			}
		}
}

static void PSPTris_game_update_cursor()
{
SceCtrlData 		new_data;

	sceCtrlReadBufferPositive(&new_data, 1);

	/* Don't move in the 'dead' zone */
	if ((new_data.Lx > 128+24) || (new_data.Lx < 128-24))
		{
		speed_x = ((float)(new_data.Lx - 128) / 255.0f) * 4;
		}
	else
		{
		speed_x = 0;
		}

	if ((new_data.Ly > 128+24) || (new_data.Ly < 128-24))
		{
		speed_y = ((float)(new_data.Ly - 128) / 255.0f) * 4;
		}
	else
		{
		speed_y= 0;
		}

	cursor_x += speed_x;
	if (cursor_x < 128)
		{
		cursor_x = 128;
		}
	else if (cursor_x > 128 + (PLAYFIELD_X_SIZE-1) * 32)
		{
		cursor_x = 128 + (PLAYFIELD_X_SIZE-1) * 32;
		}
	cursor_pf_x = (int)((cursor_x - 128) / 32);

	cursor_y += speed_y;
	if (cursor_y < 8)
		{
		cursor_y = 8;
		}
	else if (cursor_y > 8 + (PLAYFIELD_Y_SIZE-1) * 32)
		{
		cursor_y = 8 + (PLAYFIELD_Y_SIZE-1) * 32;
		}
	cursor_pf_y = (int)((cursor_y - 8) / 32);

	sceGuTexEnvColor(0xFF000000);
	sceGuBlendFunc( GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0 );
	sceGuEnable( GU_BLEND );
	sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);
	PSPTris_render_brick(128 + cursor_pf_x * 32, 8 + cursor_pf_y * 32, 32, 32, TEX_BALL_SELECT, 0x40FFFFFF);
	sceGuDisable( GU_BLEND );
}


static void PSPTris_game_mark_equal(int x, int y)
{
int		color	= playfield[x + BRICK_SIZE][y][LAYER_BRICKS];
bool	left 	= false;
bool	right 	= false;
bool	above 	= false;
bool	below 	= false;

	/* Mark this field as done */
	playfield[x + BRICK_SIZE][y][LAYER_ATTRIBUTES] = FIELD_REMOVE;
	equal_colors++;

	/* Check 4 neighbourgh cells */
	if (x > 0)
		{
		if ((playfield[x - 1 + BRICK_SIZE][y][LAYER_BRICKS] == color) &&
			(playfield[x - 1 + BRICK_SIZE][y][LAYER_ATTRIBUTES] != FIELD_REMOVE))
			{
			/* This field should have it's neighbourgh cells examined */
			playfield[x - 1 + BRICK_SIZE][y][LAYER_ATTRIBUTES] = FIELD_NEW;
			left = true;
			}
		}
	if (x < PLAYFIELD_X_SIZE - 1)
		{
		if ((playfield[x + 1 + BRICK_SIZE][y][LAYER_BRICKS] == color) &&
			(playfield[x + 1 + BRICK_SIZE][y][LAYER_ATTRIBUTES] != FIELD_REMOVE))
			{
			/* This field should have it's neighbourgh cells examined */
			playfield[x + 1 + BRICK_SIZE][y][LAYER_ATTRIBUTES] = FIELD_NEW;
			right = true;
			}
		}
	if (y > 0)
		{
		if ((playfield[x + BRICK_SIZE][y - 1][LAYER_BRICKS] == color) &&
			(playfield[x + BRICK_SIZE][y - 1][LAYER_ATTRIBUTES] != FIELD_REMOVE))
			{
			/* This field should have it's neighbourgh cells examined */
			playfield[x + BRICK_SIZE][y - 1][LAYER_ATTRIBUTES] = FIELD_NEW;
			above = true;
			}
		}
	if (y < PLAYFIELD_Y_SIZE - 1)
		{
		if ((playfield[x + BRICK_SIZE][y + 1][LAYER_BRICKS] == color) &&
			(playfield[x + BRICK_SIZE][y + 1][LAYER_ATTRIBUTES] != FIELD_REMOVE))
			{
			/* This field should have it's neighbourgh cells examined */
			playfield[x + BRICK_SIZE][y + 1][LAYER_ATTRIBUTES] = FIELD_NEW;
			below = true;
			}
		}

	/* Recursive calls to this function for all the fields which had the same color */
	if (left)
		{
		PSPTris_game_mark_equal(x - 1, y);
		}
	if (right)
		{
		PSPTris_game_mark_equal(x + 1, y);
		}
	if (above)
		{
		PSPTris_game_mark_equal(x, y - 1);
		}
	if (below)
		{
		PSPTris_game_mark_equal(x, y + 1);
		}
}

static void PSPTris_game_remove_markings()
{
	for (int y = 0 ; y < PLAYFIELD_Y_SIZE ; y++)
		{
		for (int x = BRICK_SIZE ; x < PLAYFIELD_X_SIZE + BRICK_SIZE; x++)
			{
			if (playfield[x][y][LAYER_ATTRIBUTES] == FIELD_REMOVE)
				{
				PSPTris_game_add_moving_brick(128 + (x-BRICK_SIZE)*32, 8 + y*32, playfield[x][y][LAYER_BRICKS], 32);
				}
			}
		}
}

static void PSPTris_game_fill_empty()
{
	for (int y = 0 ; y < PLAYFIELD_Y_SIZE ; y++)
		{
		for (int x = BRICK_SIZE ; x < PLAYFIELD_X_SIZE + BRICK_SIZE; x++)
			{
			if (playfield[x][y][LAYER_BRICKS] == 0)
				{
				playfield[x][y][LAYER_BRICKS] 		= (PSPTris_blockrand()%6)+8;
				playfield[x][y][LAYER_ATTRIBUTES] 	= FIELD_NONE;
				}
			}
		}
}

static bool PSPTris_game_detect_gameover()
{
	for (int y = 0 ; y < PLAYFIELD_Y_SIZE ; y++)
		{
		for (int x = BRICK_SIZE ; x < PLAYFIELD_X_SIZE + BRICK_SIZE; x++)
			{
			equal_colors = 0;
			PSPTris_game_mark_equal(x-BRICK_SIZE, y);
			if (equal_colors > 1)
				{
				return false;
				}
			}
		}
	return true;
}

static void PSPTris_game_update_score(int count)
{
	for (int i = 1 ; i <= count ; i++)
		{
		score += 10 * i;
		}
	sprintf(score_text, "%05d", score);
}

static void PSPTris_game_use_hint()
{
	for (int y = PLAYFIELD_Y_SIZE - 1 ; y >= 0 ; y--)
		{
		for (int x = PLAYFIELD_X_SIZE + BRICK_SIZE - 1 ; x >= BRICK_SIZE; x--)
			{
			equal_colors = 0;
			PSPTris_game_mark_equal(x-BRICK_SIZE, y);
			/* If we found a hint, then move the cursor and reduce the score */
			if ((equal_colors > 1) && (score >= 500))
				{
				cursor_pf_x = x - BRICK_SIZE;
				cursor_pf_y = y;
				cursor_x = (float)((cursor_pf_x * 32) + 128);
				cursor_y = (float)((cursor_pf_y * 32) +   8);
				score -= 500;
				PSPTris_game_update_score(0);
				return;
				}
			}
		}
}

static void PSPTris_game_render_connected()
{
	sceGuTexEnvColor(0xFF000000);
	sceGuBlendFunc( GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0 );
	sceGuEnable( GU_BLEND );
	sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);
	for (int y = 0 ; y < PLAYFIELD_Y_SIZE ; y++)
		{
		for (int x = BRICK_SIZE ; x < PLAYFIELD_X_SIZE + BRICK_SIZE; x++)
			{
			if (playfield[x][y][LAYER_ATTRIBUTES] == FIELD_REMOVE)
				{
				PSPTris_render_brick(128 + (x-BRICK_SIZE) * 32, 8 + y * 32, 32, 32, TEX_BALL_FRAME, 0xFFFFFFFF);
				}
			}
		}
	sceGuDisable( GU_BLEND );
}

static void PSPTris_render_playfield(u32 brightness)
{
	float cx = 128;
	float cy = 8;

	sceGuAlphaFunc( GU_GREATER, 0, 0xff );
	sceGuEnable( GU_ALPHA_TEST );
	sceGuTexEnvColor(0xFF000000);
	sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);

	for (int y = 0 ; y < PLAYFIELD_Y_SIZE ; y++)
		{
		for (int x = BRICK_SIZE ; x < PLAYFIELD_X_SIZE + BRICK_SIZE; x++)
			{
			if (playfield[x][y][LAYER_BRICKS] != 0)
				{
				PSPTris_render_brick(cx, cy, 32, 32, playfield[x][y][LAYER_BRICKS], brightness);
				}
			cx += 32;
			}
		cx = 128;
		cy += 32;
		}
	sceGuDisable( GU_ALPHA_TEST );
}

void PSPTris_game_start_level_color(int level)
{
	start_level = level;
}

static void PSPTris_game_update_timer()
{
	timer_ticks++;

	if (timer_ticks == 60)
		{
		timer_ticks = 0;
		timer--;
		sprintf(timer_text, "%03d", timer);
		}
}

void PSPTris_game_stop_color()
{
	PSPTris_audio_free_sample(new_level_sample);
	PSPTris_audio_free_sample(remove_lines_sample);
	danzeff_free();
}

bool PSPTris_game_render_color(u32 key_state, jsaTextureCache *mytcache)
{
bool	exit_game = false;

	tcache = mytcache;
	sceGuDisable(GU_DEPTH_TEST);
	sceGuEnable(GU_TEXTURE_2D);

	PSPTris_render_text("SCORE", 	374,   2);
	PSPTris_render_text(score_text, 	374,  22);
	PSPTris_render_text("LEVEL", 	374,  42);
	PSPTris_render_text(level_text, 	398,  62);
	PSPTris_render_text("BRICKS", 	366,  82);
	PSPTris_render_text(bricks_text, 374,  102);
	PSPTris_render_text("TIMER",		374,  122);
	PSPTris_render_text(timer_text,390,  142);

	if (select_counter > 0)
		{
		select_counter--;
		}
	if (hint_counter > 0)
		{
		hint_counter--;
		}

	PSPTris_game_clear_markings();
	if(!game_over && PSPTris_game_detect_gameover())
		{
		game_over = true;
		highscore_rank = PSPTris_highscore_check(score, GAMETYPE_COLOR);
		}

	if (!game_over)
		{
		PSPTris_render_playfield(0xFFFFFFFF);
		PSPTris_game_update_cursor();
		PSPTris_game_render_moving_brick();

		PSPTris_game_clear_markings();
		PSPTris_game_mark_equal(cursor_pf_x, cursor_pf_y);
		PSPTris_game_render_connected();

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

		PSPTris_game_update_timer();
		if (timer == 0)
			{
			game_over = true;
			highscore_rank = PSPTris_highscore_check(score, GAMETYPE_COLOR);
			}
		else
			{
			if ((key_state & PSP_CTRL_CROSS) && (select_counter == 0))
				{
				if (equal_colors > 1)
					{
					/* Delay for 1/2 second after a press */
					select_counter = 30;
					bricks += equal_colors;
					sprintf(bricks_text, "%05d", bricks);
					PSPTris_audio_play_sample(remove_lines_sample);
					PSPTris_game_remove_markings();
					PSPTris_game_update_score(equal_colors);
					PSPTris_remove_rows(PLAYFIELD_X_SIZE, PLAYFIELD_Y_SIZE);
					PSPTris_game_fill_empty();
					PSPTris_game_restart_time();
					}
				}
			if ((key_state & PSP_CTRL_TRIANGLE) && (hint_counter == 0))
				{
				/* Delay for 5 second after a press */
				hint_counter = 5*60;
				PSPTris_game_clear_markings();
				PSPTris_game_use_hint();
				}
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

		/* If we made it to the highscore rank, then get input */
		if (highscore_rank > 0)
			{
			static bool first_time = true;
			static bool name_entered = false;
			static highscore_str	highscore;
			SceCtrlData pad_data;

			PSPTris_render_text("WELL DONE",			128 + 2 * 16 + 8,   22);
			PSPTris_render_text("YOU MADE IT TO",	128 + 0 * 16 + 0,   42);
			PSPTris_render_text("THE HIGHSCORE",		128 + 0 * 16 + 8,   62);

			if (first_time)
				{
				highscore.score = score;
				highscore.rank  = highscore_rank;
				strcpy(highscore.name, "AAA");
				first_time = false;
				/* Initialize OSK */
				danzeff_load(DANZEFF_RENDER_SCEGU);
				danzeff_moveTo(165, 82);
				}

			if (!name_entered)
				{
				unsigned int	entered_char;
				static int		cur_char = 0;

				sceCtrlReadBufferPositive(&pad_data, 1);
				entered_char = danzeff_readInput(pad_data);
				PSPTris_render_text(highscore.name, 128 + 5 * 16 + 8, 242);
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
					PSPTris_highscore_store(&highscore, GAMETYPE_COLOR);
					danzeff_free();
					exit_game = true;
					}
				}
			}
		/* We didn't make it to the highscore rank */
		else
			{
			PSPTris_render_text("SORRY",			128 + 4 * 16 + 8,   82);
			PSPTris_render_text("YOU NEED MORE",	128 + 0 * 16 + 8,  122);
			PSPTris_render_text("PRACTICE TO",	128 + 1 * 16 + 8,  142);
			PSPTris_render_text("REACH THE",		128 + 2 * 16 + 8,  162);
			PSPTris_render_text("HIGHSCORE",		128 + 2 * 16 + 8,  182);

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
		PSPTris_game_stop_color();
		}

	return exit_game;
}
