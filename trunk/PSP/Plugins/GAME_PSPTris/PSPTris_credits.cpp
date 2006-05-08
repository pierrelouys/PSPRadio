/*
	PSPTris - The game - Credits
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
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <math.h>
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspctrl.h>

#include "PSPTris.h"
#include "PSPTris_credits.h"
#include "PSPTris_intro.h"
#include "PSPTris_menu.h"
#include "jsaTextureCache.h"

static jsaTextureCache *tcache;

void PSPTris_credits_text(char *text, int x, int y)
{
	int	length;
	int	u, v;

	length = strlen(text);

	sceGuTexFunc(GU_TFX_BLEND,GU_TCC_RGBA);
	sceGuTexEnvColor(0xFF000000);
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
}

bool PSPTris_render_credits(u32 key_state, jsaTextureCache *mytcache)
{
bool	exit_credits = false;

	tcache = mytcache;

	/* Check for exit */
	if (key_state & PSP_CTRL_CROSS)
		{
		exit_credits = true;
		}
	else
		{
		sceGuScissor(128, 8, 128+14*16, 8 + 16*16);

		sceGuAlphaFunc(GU_GREATER,0x0,0xff);
		sceGuEnable(GU_ALPHA_TEST);
		sceGuBlendFunc( GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0 );
		sceGuEnable( GU_BLEND );

		PSPTris_intro_render_brick(false);
		PSPTris_credits_text("CODING", 		128 + 0 * 16 + 1 * 8,  42);
		PSPTris_credits_text("SANDBERG", 	128 + 5 * 16 + 1 * 8,  62);
		PSPTris_credits_text("GRAPHICS", 	128 + 0 * 16 + 1 * 8, 102);
		PSPTris_credits_text("SEMTEX199",	128 + 4 * 16 + 1 * 8, 122);
		PSPTris_credits_text("MUSIC", 		128 + 0 * 16 + 1 * 8, 162);
		PSPTris_credits_text("PROPHET",		128 + 6 * 16 + 1 * 8, 182);

		PSPTris_credits_text("GREETS : RAF",		128 + 1 * 16 + 0 * 8, 222);
		PSPTris_credits_text("HALFAST DANZEL",	128 + 0 * 16 + 0 * 8, 242);
		PSPTris_intro_render_brick(true);

		sceGuDisable(GU_BLEND);
		sceGuDisable(GU_ALPHA_TEST);

		sceGuScissor(0, 0, 480, 272);
		}

	return exit_credits;
}
