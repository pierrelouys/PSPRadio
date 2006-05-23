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

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspdisplay.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include "PSPTris.h"
#include "PSPTris_menu.h"
#include "PSPTris_credits.h"
#include "PSPTris_game.h"
#include "PSPTris_audio.h"
#include "PSPTris_highscore.h"
#include "PSPTris_intro.h"
#include "jsaTextureCache.h"

#define SCR_WIDTH	(480)
#define SCR_HEIGHT	(272)
#define PIXEL_SIZE	(4)

static unsigned int __attribute__((aligned(16))) gu_list[65536];

static char *mCwd = NULL;
/* Texture handler */
static jsaTextureCache				*tcache = NULL;

static char font_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
							'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', ' ', '0', '1', '2', '3', '4',
							'5', '6', '7', '8', '9', ':', '.'};

#define	FONT_COUNT		(sizeof(font_table) / sizeof(char))

enum MENU_LIST {
	MENU_MAIN,
	MENU_GAMETYPE,
	MENU_LEVEL,
	MENU_INSTRUCTIONS,
	MENU_CREDITS,
	MENU_HIGHSCORE,
	};

enum MENU_ACTIONS {
	ACTION_START,
	ACTION_GAMETYPE,
	ACTION_LEVEL,
	ACTION_INSTRUCTIONS,
	ACTION_CREDITS,
	ACTION_HIGHSCORE,
	ACTION_EXIT,
	};

static int	active_menu = MENU_MAIN;
static bool render_credits = false;
static bool game_started = false;
static bool show_version = false;

static bool exit_plugin = false;

#define MENU_DELAY	250
#define GAME_DELAY	150

static u32	repeat_delay = MENU_DELAY;

static unsigned int menu_main_selection = 0;
static char *menu_main_table[] = {"START", "GAME TYPE", "LEVEL", "INSTRUCTIONS", "CREDITS", "HIGHSCORE", "EXIT"};
static u8 opacity_main_table[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

#define	MENU_MAIN_ITEMS		(sizeof(menu_main_table) / sizeof(char *))

static unsigned int menu_gt_selection = 0;
static char *menu_gt_table[] = {"CLASSIC", "COLOR", "ORIGINAL"};
static u8 opacity_gt_table[] = {0xFF, 0xFF, 0xFF};

#define	MENU_GT_ITEMS		(sizeof(menu_gt_table) / sizeof(char *))

static unsigned int menu_lvl_selection = 0;
static char *menu_lvl_table[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9"};
static u8 opacity_lvl_table[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

#define	MENU_LVL_ITEMS		(sizeof(menu_lvl_table) / sizeof(char *))

static char *menu_hs_table[] = {"10000  PSP", "09000  PSP", "08000  PSP", "07000  PSP", "06000  PSP", "05000  PSP", "04000  PSP", "03000  PSP", "02000  PSP", "01000  PSP"};
static u8 opacity_hs_table[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

#define	MENU_HS_ITEMS		(sizeof(menu_hs_table) / sizeof(char *))

typedef struct textline
	{
	int		fontsize;
	char	*text;
	};

static textline instructions[] = {	{ 12, " "},
									{ 12, "USE UP AND DOWN TO"},
									{ 12, "NAVIGATE IN THE"},
									{ 12, "MENU AND CROSS TO"},
									{ 12, "SELECT AND EXIT"},
									{ 12, "SUBMENUS."},
									{ 12, " "},
									{ 12, "..INGAME KEYS.."},
									{ 12, " "},
									{ 12, "CLASSIC:"},
									{ 12, "USE LEFT/LTRIGGER"},
									{ 12, "TO MOVE LEFT AND"},
									{ 12, "RIGHT/RTRIGGER TO"},
									{ 12, "MOVE RIGHT."},
									{ 12, "DOWN MOVES DOWN"},
									{ 12, "AND UP WILL DROP"},
									{ 12, "THE BRICK."},
									{ 12, "USE CROSS FOR"},
									{ 12, "ROTATION."},
									{ 12, " "},
									{ 12, "COLOR:"},
									{ 12, "USE ANALOG PAD"},
									{ 12, "TO MOVE. CROSS FOR"},
									{ 12, "SELECTING AND"},
									{ 12, "TRIANGLE FOR AUTO"},
									{ 12, "SELECTION (WILL"},
									{ 12, "ONLY BE POSSIBLE"},
									{ 12, "EVERY 5 SECONDS"},
									{ 12, "AND IT WILL COST"},
									{ 12, "YOU 500  POINTS)."},
									{ 12, " "},
									{ 12, "INSTRUCTIONS FOR"},
									{ 12, "ENTERING YOUR NAME"},
									{ 12, "ON THE HIGHSCORE"},
									{ 12, "AND HOW THE POINTS"},
									{ 12, "ARE GIVEN CAN BE"},
									{ 12, "READ IN README.TXT"},
									};

#define	INSTRUCTION_LINES		(sizeof(instructions) / sizeof(textline))
#define	SCREEN_LINES			15

/* pointer for background image and for the framebuffer */
static u8		*backimage = NULL;
static void		*framebuffer;


static jsaTextureFile __attribute__((aligned(16))) texture_list[] =
	{
	{TEX_FONT_01,		GU_PSM_8888, 256, 64, true, FT_PNG, "menufont_01.png"},
	{TEX_BRICK_BLUE,	GU_PSM_8888, 16,  16, true, FT_PNG, "brick_blue.png"},
	{TEX_BRICK_GREEN,	GU_PSM_8888, 16,  16, true, FT_PNG, "brick_green.png"},
	{TEX_BRICK_LIME,	GU_PSM_8888, 16,  16, true, FT_PNG, "brick_lime.png"},
	{TEX_BRICK_ORANGE,	GU_PSM_8888, 16,  16, true, FT_PNG, "brick_orange.png"},
	{TEX_BRICK_RED,		GU_PSM_8888, 16,  16, true, FT_PNG, "brick_red.png"},
	{TEX_BRICK_PINK,	GU_PSM_8888, 16,  16, true, FT_PNG, "brick_pink.png"},
	{TEX_BRICK_YELLOW,	GU_PSM_8888, 16,  16, true, FT_PNG, "brick_yellow.png"},
	{TEX_BALL_BLUE,		GU_PSM_8888, 32,  32, true, FT_PNG, "ball_blue.png"},
	{TEX_BALL_GREEN,	GU_PSM_8888, 32,  32, true, FT_PNG, "ball_green.png"},
	{TEX_BALL_GREY,		GU_PSM_8888, 32,  32, true, FT_PNG, "ball_grey.png"},
	{TEX_BALL_PINK,		GU_PSM_8888, 32,  32, true, FT_PNG, "ball_pink.png"},
	{TEX_BALL_RED,		GU_PSM_8888, 32,  32, true, FT_PNG, "ball_red.png"},
	{TEX_BALL_YELLOW,	GU_PSM_8888, 32,  32, true, FT_PNG, "ball_yellow.png"},
	{TEX_BALL_FRAME,	GU_PSM_8888, 32,  32, true, FT_PNG, "ball_frame.png"},
	{TEX_BALL_SELECT,	GU_PSM_8888, 32,  32, true, FT_PNG, "ball_select.png"},
	};

#define	TEXTURE_COUNT		(sizeof(texture_list) / sizeof(jsaTextureFile))

void PSPTris_load_background(char *name)
{
	if (backimage)
		{
		free(backimage);
		}
	backimage = (u8 *) memalign(16, SCR_WIDTH * SCR_HEIGHT * PIXEL_SIZE);

	if (backimage == NULL)
		{
		printf("Memory allocation error for background image..(%s)\n", name);
		return;
		}

	if (tcache->jsaTCacheLoadPngImage((const char *)name, (u32 *)backimage) == -1)
		{
		printf("Failed loading background image..\n");
		free(backimage);
		backimage = NULL;
		return;
		}
}

void PSPTris_render_background(void)
{
	static bool skip_first = true;

	if (skip_first)
		{
		skip_first = false;
		}
	else
		{
		sceGuCopyImage(GU_PSM_8888, 0, 0, 480, 272, 480, backimage, 0, 0, 512, (void*)(0x04000000+(u32)framebuffer));
		sceGuTexSync();
		}
}

int PSPTris_calc_start_x(int chars)
{
	return (128 + (((14-chars)*FONT_X_SIZE) / 2));
}

void PSPTris_get_texture_coords(char letter, int *u, int *v)
{
bool	found = false;
int		char_number = 0;

	for (unsigned int i = 0 ; i < FONT_COUNT; i++)
		{
		if (font_table[i] == letter)
			{
			found = true;
			break;
			}
		char_number++;
		}

	if (found == false)
		{
		/* call this function again to get the coords for a space */
		PSPTris_get_texture_coords(' ', u, v);
		}
	else
		{
		*u = (char_number % FONT_X_SIZE) * FONT_X_SIZE;
		*v = (char_number / FONT_X_COUNT) * FONT_Y_SIZE;
		}
}

void PSPTris_render_letter(float x, float y, float u, float v, u32 opacity, float size)
{
	u32		color = opacity | 0xFFFFFF;

	struct NCVertex* c_vertices = (struct NCVertex*)sceGuGetMemory(2 * sizeof(struct NCVertex));

	c_vertices[0].u 		= u;
	c_vertices[0].v 		= v;
	c_vertices[0].x 		= x;
	c_vertices[0].y 		= y;
	c_vertices[0].z 		= 0;
	c_vertices[0].color 	= color;

	c_vertices[1].u 		= u + FONT_X_SIZE;
	c_vertices[1].v 		= v + FONT_Y_SIZE;
	c_vertices[1].x 		= x + size;
	c_vertices[1].y 		= y + size;
	c_vertices[1].z 		= 0;
	c_vertices[1].color 	= color;

	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,c_vertices);
}

void PSPTris_render_menu_item(char *text, int x, int y, u32 opacity, int size)
{
	int	u, v;
	int length;

	length = strlen(text);

	for (int i = 0 ; i < length ; i++)
		{
		PSPTris_get_texture_coords(text[i], &u, &v);
		PSPTris_render_letter(x, y, u, v, opacity, size);
		x += size;
		}
}

unsigned int PSPTris_menu_get_count(void)
{
	switch (active_menu)
		{
		case	MENU_MAIN:
			return MENU_MAIN_ITEMS;
			break;
		case	MENU_GAMETYPE:
			return MENU_GT_ITEMS;
			break;
		case	MENU_LEVEL:
			return MENU_LVL_ITEMS;
			break;
		case	MENU_HIGHSCORE:
			return MENU_HS_ITEMS;
			break;
		}
	return 0;
}

void PSPTris_menu_text_gu_on()
	{
	sceGuEnable(GU_TEXTURE_2D);
	sceGuDisable(GU_DEPTH_TEST);

	sceGuAlphaFunc( GU_GREATER, 0, 0xff );
	sceGuEnable( GU_ALPHA_TEST );

	sceGuTexFunc(GU_TFX_BLEND,GU_TCC_RGBA);
	sceGuTexEnvColor(0xFF000000);

	sceGuBlendFunc( GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0 );
	sceGuEnable( GU_BLEND );

	sceGuTexWrap(GU_REPEAT, GU_REPEAT);
	sceGuTexFilter(GU_LINEAR, GU_LINEAR);

	// setup texture
	(void)tcache->jsaTCacheSetTexture(TEX_FONT_01);
	sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);
	}

void PSPTris_menu_text_gu_off()
	{
	sceGuDisable(GU_BLEND);
	sceGuDisable(GU_ALPHA_TEST);
	sceGuDisable(GU_TEXTURE_2D);
	sceGuEnable(GU_DEPTH_TEST);
	}

void PSPTris_render_menu(void)
{
unsigned int	count = PSPTris_menu_get_count();
u32				opacity;
char			*text;
int				y = (8+4*16);

	if ((active_menu == MENU_LEVEL) || (active_menu == MENU_HIGHSCORE))
		{
		y = (8+2*16);
		}

	PSPTris_menu_text_gu_on();

	for (unsigned int i = 0 ; i < count ; i++)
		{

		switch (active_menu)
			{
			case	MENU_MAIN:
				if (i == menu_main_selection)
					{
					opacity_main_table[i] = 0xFF;
					}
				else
					{
					opacity_main_table[i] = 0x80;
					}
				opacity = opacity_main_table[i] << 24;
				text = menu_main_table[i];
				break;
			case	MENU_GAMETYPE:
				if (i == menu_gt_selection)
					{
					opacity_gt_table[i] = 0xFF;
					}
				else
					{
					opacity_gt_table[i] = 0x80;
					}
				opacity = opacity_gt_table[i] << 24;
				text = menu_gt_table[i];
				break;
			case	MENU_LEVEL:
				if (i == menu_lvl_selection)
					{
					opacity_lvl_table[i] = 0xFF;
					}
				else
					{
					opacity_lvl_table[i] = 0x80;
					}
				opacity = opacity_lvl_table[i] << 24;
				text = menu_lvl_table[i];
				break;
			case	MENU_HIGHSCORE:
				if (i == 0)
					{
					opacity_hs_table[i] = 0xFF;
					}
				else
					{
					opacity_hs_table[i] = 0x80;
					}
				opacity = opacity_hs_table[i] << 24;
				text = menu_hs_table[i];
				break;
			default:
				opacity = 0;
				text = 0;
				break;
			}

		PSPTris_render_menu_item(text, PSPTris_calc_start_x(strlen(text)), y, opacity, FONT_X_SIZE);
		y += FONT_Y_SIZE;
		}

	PSPTris_menu_text_gu_off();
}

void PSPTris_update_highscore_table(void)
{
highscore_str *highscore;

	for (int i = 0 ; i < 10 ; i++)
		{
		highscore =	PSPTris_highscore_get(i+1, menu_gt_selection);
		sprintf(menu_hs_table[i], "%05d  %s", highscore->score, highscore->name);
		}
}

void PSPTris_start_music()
{
#if !defined(DYNAMIC_BUILD)
	char path[1024];

	/* Start playing menu module */
	sprintf(path, "%s/Music/menu.mod", mCwd);
	PSPTris_audio_play_module(path);
#endif /* !defined(DYNAMIC_BUILD) */
}

void PSPTris_stop_music()
{
#if !defined(DYNAMIC_BUILD)
	PSPTris_audio_stop_module();
#endif /* !defined(DYNAMIC_BUILD) */
}

void PSPTris_menu_init(char *cwd)
{
	char path[1024];

	mCwd = cwd;
#if !defined(DYNAMIC_BUILD)
	/* Start playing menu module */
	PSPTris_start_music();
#endif /* !defined(DYNAMIC_BUILD) */

	/* Create a texture cache object */
	tcache = new jsaTextureCache();

	sprintf(path, "%s/Textures", cwd);
	tcache->jsaTCacheLoadTextureSet(path, texture_list, TEXTURE_COUNT);

	backimage = NULL;
	framebuffer = NULL;

	PSPTris_highscore_init(cwd);
	PSPTris_update_highscore_table();

	sprintf(path, "%s/Textures/psptris_background_01.png", cwd);
	PSPTris_load_background(path);
	sceKernelDcacheWritebackAll();
}

void PSPTris_menu_destroy(void)
{
	PSPTris_stop_music();

	if (backimage)
		{
		free(backimage);
		}
	if (tcache)
		{
		delete tcache;
		}
}

void PSPTris_handle_menu_main(u32 key_state)
	{

	switch(key_state)
		{
		case	PSP_CTRL_CROSS:
			if (menu_main_selection == ACTION_START)
				{
				/* The original game needs to use another background */
				if (menu_gt_selection == GAMETYPE_ORIGINAL)
					{
					char	path[1024];
					sprintf(path, "%s/Textures/psptris_background_02.png", mCwd);
					PSPTris_load_background(path);
					}
				PSPTris_stop_music();
				PSPTris_game_init(mCwd);
				game_started = true;
				repeat_delay = GAME_DELAY;
				}
			else if (menu_main_selection == ACTION_GAMETYPE)
				{
				active_menu = MENU_GAMETYPE;
				}
			else if (menu_main_selection == ACTION_LEVEL)
				{
				active_menu = MENU_LEVEL;
				}
			else if (menu_main_selection == ACTION_INSTRUCTIONS)
				{
				active_menu = MENU_INSTRUCTIONS;
				}
			else if (menu_main_selection == ACTION_CREDITS)
				{
				PSPTris_setup_bricks();
				render_credits = true;
				active_menu = MENU_CREDITS;
				}
			else if (menu_main_selection == ACTION_HIGHSCORE)
				{
				PSPTris_update_highscore_table();
				active_menu = MENU_HIGHSCORE;
				}
			else if (menu_main_selection == ACTION_EXIT)
				{
				exit_plugin = true;
				}
			break;
		case	PSP_CTRL_UP:
			if (menu_main_selection > 0)
				{
				menu_main_selection--;
				}
			break;
		case	PSP_CTRL_DOWN:
			if (menu_main_selection < MENU_MAIN_ITEMS-1)
				{
				menu_main_selection++;
				}
			break;
		case	PSP_CTRL_SELECT:
			if (show_version)
				{
				show_version = false;
				}
			else
				{
				show_version = true;
				}
			break;
		default:
			break;
		}
	}

void PSPTris_handle_menu_gametype(u32 key_state)
	{
	switch(key_state)
		{
		case	PSP_CTRL_CROSS:
			PSPTris_game_type(menu_gt_selection);
			active_menu = MENU_MAIN;
			break;
		case	PSP_CTRL_UP:
			if (menu_gt_selection > 0)
				{
				menu_gt_selection--;
				}
			break;
		case	PSP_CTRL_DOWN:
			if (menu_gt_selection < MENU_GT_ITEMS-1)
				{
				menu_gt_selection++;
				}
			break;
		default:
			break;
		}
	}

void PSPTris_handle_menu_level(u32 key_state)
	{
	switch(key_state)
		{
		case	PSP_CTRL_CROSS:
			PSPTris_game_start_level(menu_lvl_selection + 1);
			active_menu = MENU_MAIN;
			break;
		case	PSP_CTRL_UP:
			if (menu_lvl_selection > 0)
				{
				menu_lvl_selection--;
				}
			break;
		case	PSP_CTRL_DOWN:
			if (menu_lvl_selection < MENU_LVL_ITEMS-1)
				{
				menu_lvl_selection++;
				}
			break;
		default:
			break;
		}
	}

void PSPTris_handle_menu_instructions(u32 key_state)
	{
	static u32 current_line = 0;

	switch(key_state)
		{
		case	PSP_CTRL_CROSS:
			current_line = 0;
			active_menu = MENU_MAIN;
			break;
		case	PSP_CTRL_UP:
			if (current_line > 0)
				{
				current_line--;
				}
			break;
		case	PSP_CTRL_DOWN:
			if (current_line < (INSTRUCTION_LINES - SCREEN_LINES))
				{
				current_line++;
				}
			break;
		default:
			break;
		}

	PSPTris_menu_text_gu_on();
	PSPTris_render_menu_item("INSTRUCTIONS", 144, 16, 0xFFFFFFFF, 16);
	PSPTris_render_menu_item("USE UP AND DOWN TO SCROLL", 144, 36, 0xFFFFFFFF, 8);

	int		y = 48;

	for (u32 i = current_line ; i < (current_line + SCREEN_LINES) ; i++)
		{
		PSPTris_render_menu_item(instructions[i].text, 130, y, 0xFFFFFFFF, instructions[i].fontsize);
		y += instructions[i].fontsize + 2;
		}

	PSPTris_menu_text_gu_off();

	}

void PSPTris_handle_menu_highscore(u32 key_state)
	{
	switch(key_state)
		{
		case	PSP_CTRL_CROSS:
			active_menu = MENU_MAIN;
			break;
		default:
			break;
		}
	}

void PSPTris_handle_menu_credits(u32 key_state)
	{
	switch(key_state)
		{
		default:
			break;
		}
	}

void PSPTris_handle_key(u32 key_state)
{

	switch(active_menu)
		{
		case	MENU_MAIN:
			PSPTris_handle_menu_main(key_state);
			break;
		case	MENU_GAMETYPE:
			PSPTris_handle_menu_gametype(key_state);
			break;
		case	MENU_LEVEL:
			PSPTris_handle_menu_level(key_state);
			break;
		case	MENU_INSTRUCTIONS:
			PSPTris_handle_menu_instructions(key_state);
			break;
		case	MENU_CREDITS:
			PSPTris_handle_menu_credits(key_state);
			break;
		case	MENU_HIGHSCORE:
			PSPTris_handle_menu_highscore(key_state);
			break;
		default:
			break;
		}
}

bool PSPTris_menu(u32 key_state, u32 *key_delay)
{
 	sceGuStart(GU_DIRECT,::gu_list);

	sceGuClearColor(0xFF1009B2);
	sceGuClearDepth(0);
	sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);

	PSPTris_render_background();

	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadIdentity();
	sceGumPerspective(75.0f,16.0f/9.0f,0.5f,1000.0f);

	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();

	if (render_credits)
		{
		if (PSPTris_render_credits(key_state, tcache))
			{
			active_menu = MENU_MAIN;
			render_credits = false;
			}
		}
	else if (game_started)
		{
		if (PSPTris_game_render(key_state, tcache))
			{
			/* Reload the menu background if needed */
			if (menu_gt_selection == GAMETYPE_ORIGINAL)
				{
				char	path[1024];
				sprintf(path, "%s/Textures/psptris_background_01.png", mCwd);
				PSPTris_load_background(path);
				}
			PSPTris_start_music();
			game_started = false;
			repeat_delay = MENU_DELAY;
			}
		}
	else
		{
		PSPTris_handle_key(key_state);
		PSPTris_render_menu();
		if (show_version)
			{
			PSPTris_menu_text_gu_on();
			PSPTris_render_menu_item(APP_NAME, 144, 250, 0xFFFFFFFF, 12);
			PSPTris_menu_text_gu_off();
			}
		}

	sceGuFinish();
	sceGuSync(0,0);

	framebuffer = sceGuSwapBuffers();
	sceDisplayWaitVblankStart();

	*key_delay = repeat_delay;

	return exit_plugin;
}
