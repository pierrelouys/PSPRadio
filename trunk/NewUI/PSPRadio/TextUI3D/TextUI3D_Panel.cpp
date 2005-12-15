/* 
	PSPRadio / Music streaming client for the PSP. (Initial Release: Sept. 2005)
	PSPRadio Copyright (C) 2005 Rafael Cabezas a.k.a. Raf
	TextUI3D Copyright (C) 2005 Jesper Sandberg & Raf

	
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
#include <list>
#include <PSPApp.h>
#include <PSPSound.h>
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <malloc.h>
#include <math.h>
#include <time.h>
#include <iniparser.h>
#include <Tools.h>
#include <stdarg.h>
#include <Logging.h>

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <psprtc.h>
#include <psppower.h>

#include "TextUI3D.h"

#define PANEL_X			32
#define PANEL_Y			32

#define	PANELWIDTH		280
#define PANELHEIGHT		170

#define FRAMEWIDTH		31
#define FRAMEHEIGHT		31

#define	FRAME_X_1	PANEL_X
#define	FRAME_X_2	(FRAME_X_1+FRAMEWIDTH)

#define	FRAME_X_3	(FRAME_X_2+1)
#define	FRAME_X_4	(FRAME_X_3+PANELWIDTH)

#define	FRAME_X_5	(FRAME_X_4+1)
#define	FRAME_X_6	(FRAME_X_5+FRAMEWIDTH)

#define	FRAME_Y_1	PANEL_Y
#define	FRAME_Y_2	(FRAME_Y_1+FRAMEHEIGHT)

#define	FRAME_Y_3	(FRAME_Y_2+1)
#define	FRAME_Y_4	(FRAME_Y_3+PANELHEIGHT)

#define	FRAME_Y_5	(FRAME_Y_4+1)
#define	FRAME_Y_6	(FRAME_Y_5+FRAMEHEIGHT)


void CTextUI3D::RenderFrame1(void)
{
	struct Vertex* c_vertices;

	sceGuEnable(GU_TEXTURE_2D);

	sceGuAlphaFunc( GU_GREATER, 0, 0xff );
	sceGuEnable( GU_ALPHA_TEST );

	sceGuTexFunc(GU_TFX_BLEND,GU_TCC_RGBA);
	sceGuTexEnvColor(0xFF000000);

	sceGuDepthFunc(GU_ALWAYS);

	sceGuBlendFunc( GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0 );
	sceGuEnable( GU_BLEND );

	sceGuTexWrap(GU_REPEAT, GU_REPEAT);

	/* Upper left corner */
	(void)tcache.jsaTCacheSetTexture(TEX_CORNER);
	c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	c_vertices[0].u = 0; c_vertices[0].v = 0;
	c_vertices[0].x = FRAME_X_1; c_vertices[0].y = FRAME_Y_1; c_vertices[0].z = 0;
	c_vertices[0].color = 0xFF000000;
	c_vertices[1].u = (FRAME_X_2-FRAME_X_1); c_vertices[1].v = (FRAME_Y_2-FRAME_Y_1);
	c_vertices[1].x = FRAME_X_2; c_vertices[1].y = FRAME_Y_2; c_vertices[1].z = 0;
	c_vertices[1].color = 0xFF000000;
	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, c_vertices);

	/* Upper frame */
	(void)tcache.jsaTCacheSetTexture(TEX_HORIZONTAL);
	c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	c_vertices[0].u = 0; c_vertices[0].v = 0;
	c_vertices[0].x = FRAME_X_3; c_vertices[0].y = FRAME_Y_1; c_vertices[0].z = 0;
	c_vertices[0].color = 0xFF000000;
	c_vertices[1].u = (FRAME_X_4-FRAME_X_3); c_vertices[1].v = (FRAME_Y_2-FRAME_Y_1);
	c_vertices[1].x = FRAME_X_4; c_vertices[1].y = FRAME_Y_2; c_vertices[1].z = 0;
	c_vertices[1].color = 0xFF000000;
	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, c_vertices);

	/* Upper right corner */
	(void)tcache.jsaTCacheSetTexture(TEX_CORNER);
	c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	c_vertices[0].u = (FRAME_X_6-FRAME_X_5); c_vertices[0].v = 0;
	c_vertices[0].x = FRAME_X_5; c_vertices[0].y = FRAME_Y_1; c_vertices[0].z = 0;
	c_vertices[0].color = 0xFF000000;
	c_vertices[1].u = 0; c_vertices[1].v = (FRAME_Y_2-FRAME_Y_1);
	c_vertices[1].x = FRAME_X_6; c_vertices[1].y = FRAME_Y_2; c_vertices[1].z = 0;
	c_vertices[1].color = 0xFF000000;
	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, c_vertices);

	/* Left frame */
	(void)tcache.jsaTCacheSetTexture(TEX_VERTICAL);
	c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	c_vertices[0].u = 0; c_vertices[0].v = 0;
	c_vertices[0].x = FRAME_X_1; c_vertices[0].y = FRAME_Y_3; c_vertices[0].z = 0;
	c_vertices[0].color = 0xFF000000;
	c_vertices[1].u = (FRAME_X_2-FRAME_X_1); c_vertices[1].v = (FRAME_Y_4-FRAME_Y_3);
	c_vertices[1].x = FRAME_X_2; c_vertices[1].y = FRAME_Y_4; c_vertices[1].z = 0;
	c_vertices[1].color = 0xFF000000;
	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, c_vertices);

	/* Fill frame */
	(void)tcache.jsaTCacheSetTexture(TEX_FILL);
	c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	c_vertices[0].u = 0; c_vertices[0].v = 0;
	c_vertices[0].x = FRAME_X_3; c_vertices[0].y = FRAME_Y_3; c_vertices[0].z = 0;
	c_vertices[0].color = 0xFF000000;
	c_vertices[1].u = (FRAME_X_4-FRAME_X_3); c_vertices[1].v = (FRAME_Y_4-FRAME_Y_3);
	c_vertices[1].x = FRAME_X_4; c_vertices[1].y = FRAME_Y_4; c_vertices[1].z = 0;
	c_vertices[1].color = 0xFF000000;
	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, c_vertices);

	/* Right frame */
	(void)tcache.jsaTCacheSetTexture(TEX_VERTICAL);
	c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	c_vertices[0].u = (FRAME_X_6-FRAME_X_5); c_vertices[0].v = 0;
	c_vertices[0].x = FRAME_X_5; c_vertices[0].y = FRAME_Y_3; c_vertices[0].z = 0;
	c_vertices[0].color = 0xFF000000;
	c_vertices[1].u = 0; c_vertices[1].v = (FRAME_Y_4-FRAME_Y_3);
	c_vertices[1].x = FRAME_X_6; c_vertices[1].y = FRAME_Y_4; c_vertices[1].z = 0;
	c_vertices[1].color = 0xFF000000;
	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, c_vertices);

	/* Lower left corner */
	(void)tcache.jsaTCacheSetTexture(TEX_CORNER);
	c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	c_vertices[0].u = 0; c_vertices[0].v = (FRAME_Y_6-FRAME_Y_5);
	c_vertices[0].x = FRAME_X_1; c_vertices[0].y = FRAME_Y_5; c_vertices[0].z = 0;
	c_vertices[0].color = 0xFF000000;
	c_vertices[1].u = (FRAME_X_2-FRAME_X_1); c_vertices[1].v = 0;
	c_vertices[1].x = FRAME_X_2; c_vertices[1].y = FRAME_Y_6; c_vertices[1].z = 0;
	c_vertices[1].color = 0xFF000000;
	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, c_vertices);

	/* Lower frame */
	(void)tcache.jsaTCacheSetTexture(TEX_HORIZONTAL);
	c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	c_vertices[0].u = 0; c_vertices[0].v = (FRAME_Y_6-FRAME_Y_5);
	c_vertices[0].x = FRAME_X_3; c_vertices[0].y = FRAME_Y_5; c_vertices[0].z = 0;
	c_vertices[0].color = 0xFF000000;
	c_vertices[1].u = (FRAME_X_4-FRAME_X_3); c_vertices[1].v = 0;
	c_vertices[1].x = FRAME_X_4; c_vertices[1].y = FRAME_Y_6; c_vertices[1].z = 0;
	c_vertices[1].color = 0xFF000000;
	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, c_vertices);

	/* Lower right corner */
	(void)tcache.jsaTCacheSetTexture(TEX_CORNER);
	c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	c_vertices[0].u = (FRAME_X_6-FRAME_X_5); c_vertices[0].v = (FRAME_Y_6-FRAME_Y_5);
	c_vertices[0].x = FRAME_X_5; c_vertices[0].y = FRAME_Y_5; c_vertices[0].z = 0;
	c_vertices[0].color = 0xFF000000;
	c_vertices[1].u = 0; c_vertices[1].v = 0;
	c_vertices[1].x = FRAME_X_6; c_vertices[1].y = FRAME_Y_6; c_vertices[1].z = 0;
	c_vertices[1].color = 0xFF000000;
	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, c_vertices);

	sceGuDisable(GU_BLEND);
	sceGuDisable(GU_ALPHA_TEST);
	sceGuDisable(GU_TEXTURE_2D);
	sceGuDepthFunc(GU_GEQUAL);
}

void CTextUI3D::RenderFrame2(void)
{
	sceGuEnable(GU_TEXTURE_2D);

	sceGuAlphaFunc( GU_GREATER, 0, 0xff );
	sceGuEnable( GU_ALPHA_TEST );

	sceGuTexFunc(GU_TFX_BLEND,GU_TCC_RGBA);
	sceGuTexEnvColor(0xFF000000);

	sceGuDepthFunc(GU_ALWAYS);

	sceGuBlendFunc( GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0 );
	sceGuEnable( GU_BLEND );

	sceGuTexWrap(GU_REPEAT, GU_REPEAT);

	struct Vertex* c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	c_vertices[0].u = 0; c_vertices[0].v = 0;
	c_vertices[0].x = PANEL_X; c_vertices[0].y = PANEL_Y; c_vertices[0].z = 0;
	c_vertices[0].color = 0xFF000000;
	c_vertices[1].u = 31; c_vertices[1].v = 31;
	c_vertices[1].x = PANEL_X + PANELWIDTH; c_vertices[1].y = PANEL_Y + PANELHEIGHT; c_vertices[1].z = 0;
	c_vertices[1].color = 0xFF000000;
	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,c_vertices);

	sceGuDisable(GU_BLEND);
	sceGuDisable(GU_TEXTURE_2D);

	struct Vertex* l_vertices = (struct Vertex*)sceGuGetMemory(5 * sizeof(struct Vertex));
	l_vertices[0].x = PANEL_X;	l_vertices[0].y = PANEL_Y; 	l_vertices[0].color = 0xFFFFFFFF;
	l_vertices[1].x = PANEL_X + PANELWIDTH;	l_vertices[1].y = PANEL_Y; 	l_vertices[1].color = 0xFFFFFFFF;
	l_vertices[2].x = PANEL_X + PANELWIDTH;	l_vertices[2].y = PANEL_Y + PANELHEIGHT; 	l_vertices[2].color = 0xFFFFFFFF;
	l_vertices[3].x = PANEL_X;	l_vertices[3].y = PANEL_Y + PANELHEIGHT; 	l_vertices[3].color = 0xFFFFFFFF;
	l_vertices[4].x = PANEL_X;	l_vertices[4].y = PANEL_Y; 	l_vertices[4].color = 0xFFFFFFFF;

	sceGuDrawArray(GU_LINE_STRIP,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,5,0,l_vertices);

	sceGuDisable(GU_BLEND);
	sceGuDisable(GU_ALPHA_TEST);
	sceGuDisable(GU_TEXTURE_2D);
	sceGuDepthFunc(GU_GEQUAL);
}

