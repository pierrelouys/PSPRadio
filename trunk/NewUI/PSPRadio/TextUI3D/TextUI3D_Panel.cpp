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
/*
#include <psprtc.h>
#include <psppower.h>
*/

#include "TextUI3D.h"
#include "TextUI3D_Panel.h"


#define PANEL_X			32
#define PANEL_Y			32

#define	PANELWIDTH		280
#define PANELHEIGHT		170

#define FRAMEWIDTH		31
#define FRAMEHEIGHT		31

#define	FRAME_X_1	PANEL_X
#define	FRAME_X_2	(FRAME_X_1+FRAMEWIDTH)

#define	FRAME_X_3	(FRAME_X_2)
#define	FRAME_X_4	(FRAME_X_3+PANELWIDTH)

#define	FRAME_X_5	(FRAME_X_4)
#define	FRAME_X_6	(FRAME_X_5+FRAMEWIDTH)

#define	FRAME_Y_1	PANEL_Y
#define	FRAME_Y_2	(FRAME_Y_1+FRAMEHEIGHT)

#define	FRAME_Y_3	(FRAME_Y_2)
#define	FRAME_Y_4	(FRAME_Y_3+PANELHEIGHT)

#define	FRAME_Y_5	(FRAME_Y_4)
#define	FRAME_Y_6	(FRAME_Y_5+FRAMEHEIGHT)

CTextUI3D_Panel::CTextUI3D_Panel()
	{
	m_xpos 			= 0;
	m_ypos 			= 0;
	m_zpos 			= 0;

	m_width			= 0;
	m_height		= 0;

	/* Build the vertex array with initial parameters */
	m_vertex_array = (Vertex *) memalign(16, 18 * sizeof(Vertex));
	UpdateVertexArray();
	}

CTextUI3D_Panel::~CTextUI3D_Panel()
	{
	free(m_vertex_array);
	}

void CTextUI3D_Panel::SetPosition(int x, int y, int z = 0)
	{
	m_xpos = x;
	m_ypos = y;
	m_zpos = z;

	/* Update the vertex array, since the parameters has changed */
	UpdateVertexArray();
	}

void CTextUI3D_Panel::SetSize(int width, int height)
	{
	m_width		= width;
	m_height	= height;

	/* Update the vertex array, since the parameters has changed */
	UpdateVertexArray();
	}

void CTextUI3D_Panel::SetFrameTexture(FrameTextures &textures)
	{
	m_frametextures = textures;

	/* Update the vertex array, since the parameters has changed */
	UpdateVertexArray();
	}

void CTextUI3D_Panel::UpdateVertexArray()
	{
	float	xpos_array[] = 	{
							m_xpos,										m_xpos + m_frametextures.width,
							m_xpos + m_frametextures.width,				m_xpos + m_frametextures.width + m_width,
							m_xpos + m_frametextures.width + m_width,	m_xpos + 2 * m_frametextures.width + m_width
							};

	float	ypos_array[] = 	{
							m_ypos,										m_ypos + m_frametextures.height,
							m_ypos + m_frametextures.height, 			m_ypos + m_frametextures.height + m_height,
							m_ypos + m_frametextures.height + m_height,	m_ypos + 2 * m_frametextures.height + m_height
							};

	/* Set color and zpos for all vertices */
	for (int i = 0 ; i < 18 ; i++)
		{
		m_vertex_array[i].color = 0xFF000000;
		m_vertex_array[i].z		= m_zpos;

		/* Every second vertex have (u,v) set to (0,0) */
		if (!(i % 2))
			{
			m_vertex_array[i].u = 0;
			m_vertex_array[i].v = 0;
			}
		}

	/* Set x and y positions and texture coords */
	for (int y = 0 ; y < 3 ; y++)
		{
		for (int x = 0 ; x < 3 ; x++)
			{
			m_vertex_array[y * 6 + x + 0].x = xpos_array[x * 2 + 0];
			m_vertex_array[y * 6 + x + 1].x = xpos_array[x * 2 + 1];

			m_vertex_array[y * 6 + x + 0].y = ypos_array[x * 2 + 0];
			m_vertex_array[y * 6 + x + 1].y = ypos_array[x * 2 + 1];
			
			/* Texture coords */
			m_vertex_array[y * 6 + x + 1].u = m_vertex_array[y * 6 + x + 1].x - m_vertex_array[y * 6 + x + 0].x;
			m_vertex_array[y * 6 + x + 1].v = m_vertex_array[y * 6 + x + 1].y - m_vertex_array[y * 6 + x + 0].y;
			}
		}
	}

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

	// Upper left corner
	(void)tcache.jsaTCacheSetTexture(TEX_CORNER_UL);
	c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	c_vertices[0].u = 0; c_vertices[0].v = 0;
	c_vertices[0].x = FRAME_X_1; c_vertices[0].y = FRAME_Y_1; c_vertices[0].z = 0;
	c_vertices[0].color = 0xFF000000;
	c_vertices[1].u = (FRAME_X_2-FRAME_X_1); c_vertices[1].v = (FRAME_Y_2-FRAME_Y_1);
	c_vertices[1].x = FRAME_X_2; c_vertices[1].y = FRAME_Y_2; c_vertices[1].z = 0;
	c_vertices[1].color = 0xFF000000;
	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, c_vertices);

	// Upper frame
	(void)tcache.jsaTCacheSetTexture(TEX_FRAME_T);
	c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	c_vertices[0].u = 0; c_vertices[0].v = 0;
	c_vertices[0].x = FRAME_X_3; c_vertices[0].y = FRAME_Y_1; c_vertices[0].z = 0;
	c_vertices[0].color = 0xFF000000;
	c_vertices[1].u = (FRAME_X_4-FRAME_X_3); c_vertices[1].v = (FRAME_Y_2-FRAME_Y_1);
	c_vertices[1].x = FRAME_X_4; c_vertices[1].y = FRAME_Y_2; c_vertices[1].z = 0;
	c_vertices[1].color = 0xFF000000;
	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, c_vertices);

	// Upper right corner
	(void)tcache.jsaTCacheSetTexture(TEX_CORNER_UR);
	c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	c_vertices[0].u = 0; c_vertices[0].v = 0;
	c_vertices[0].x = FRAME_X_5; c_vertices[0].y = FRAME_Y_1; c_vertices[0].z = 0;
	c_vertices[0].color = 0xFF000000;
	c_vertices[1].u = (FRAME_X_6-FRAME_X_5); c_vertices[1].v = (FRAME_Y_2-FRAME_Y_1);
	c_vertices[1].x = FRAME_X_6; c_vertices[1].y = FRAME_Y_2; c_vertices[1].z = 0;
	c_vertices[1].color = 0xFF000000;
	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, c_vertices);

	// Left frame
	(void)tcache.jsaTCacheSetTexture(TEX_FRAME_L);
	c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	c_vertices[0].u = 0; c_vertices[0].v = 0;
	c_vertices[0].x = FRAME_X_1; c_vertices[0].y = FRAME_Y_3; c_vertices[0].z = 0;
	c_vertices[0].color = 0xFF000000;
	c_vertices[1].u = (FRAME_X_2-FRAME_X_1); c_vertices[1].v = (FRAME_Y_4-FRAME_Y_3);
	c_vertices[1].x = FRAME_X_2; c_vertices[1].y = FRAME_Y_4; c_vertices[1].z = 0;
	c_vertices[1].color = 0xFF000000;
	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, c_vertices);

	// Fill frame
	(void)tcache.jsaTCacheSetTexture(TEX_FILL);
	c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	c_vertices[0].u = 0; c_vertices[0].v = 0;
	c_vertices[0].x = FRAME_X_3; c_vertices[0].y = FRAME_Y_3; c_vertices[0].z = 0;
	c_vertices[0].color = 0xFF000000;
	c_vertices[1].u = (FRAME_X_4-FRAME_X_3); c_vertices[1].v = (FRAME_Y_4-FRAME_Y_3);
	c_vertices[1].x = FRAME_X_4; c_vertices[1].y = FRAME_Y_4; c_vertices[1].z = 0;
	c_vertices[1].color = 0xFF000000;
	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, c_vertices);

	// Right frame
	(void)tcache.jsaTCacheSetTexture(TEX_FRAME_R);
	c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	c_vertices[0].u = 0; c_vertices[0].v = 0;
	c_vertices[0].x = FRAME_X_5; c_vertices[0].y = FRAME_Y_3; c_vertices[0].z = 0;
	c_vertices[0].color = 0xFF000000;
	c_vertices[1].u = (FRAME_X_6-FRAME_X_5); c_vertices[1].v = (FRAME_Y_4-FRAME_Y_3);
	c_vertices[1].x = FRAME_X_6; c_vertices[1].y = FRAME_Y_4; c_vertices[1].z = 0;
	c_vertices[1].color = 0xFF000000;
	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, c_vertices);

	// Lower left corner
	(void)tcache.jsaTCacheSetTexture(TEX_CORNER_LL);
	c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	c_vertices[0].u = 0; c_vertices[0].v = 0;
	c_vertices[0].x = FRAME_X_1; c_vertices[0].y = FRAME_Y_5; c_vertices[0].z = 0;
	c_vertices[0].color = 0xFF000000;
	c_vertices[1].u = (FRAME_X_2-FRAME_X_1); c_vertices[1].v = (FRAME_Y_6-FRAME_Y_5);
	c_vertices[1].x = FRAME_X_2; c_vertices[1].y = FRAME_Y_6; c_vertices[1].z = 0;
	c_vertices[1].color = 0xFF000000;
	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, c_vertices);

	// Lower frame
	(void)tcache.jsaTCacheSetTexture(TEX_FRAME_B);
	c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	c_vertices[0].u = 0; c_vertices[0].v = 0;
	c_vertices[0].x = FRAME_X_3; c_vertices[0].y = FRAME_Y_5; c_vertices[0].z = 0;
	c_vertices[0].color = 0xFF000000;
	c_vertices[1].u = (FRAME_X_4-FRAME_X_3); c_vertices[1].v = (FRAME_Y_6-FRAME_Y_5);
	c_vertices[1].x = FRAME_X_4; c_vertices[1].y = FRAME_Y_6; c_vertices[1].z = 0;
	c_vertices[1].color = 0xFF000000;
	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, c_vertices);

	// Lower right corner
	(void)tcache.jsaTCacheSetTexture(TEX_CORNER_LR);
	c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	c_vertices[0].u = 0; c_vertices[0].v = 0;
	c_vertices[0].x = FRAME_X_5; c_vertices[0].y = FRAME_Y_5; c_vertices[0].z = 0;
	c_vertices[0].color = 0xFF000000;
	c_vertices[1].u = (FRAME_X_6-FRAME_X_5); c_vertices[1].v = (FRAME_Y_6-FRAME_Y_5);
	c_vertices[1].x = FRAME_X_6; c_vertices[1].y = FRAME_Y_6; c_vertices[1].z = 0;
	c_vertices[1].color = 0xFF000000;
	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, c_vertices);

	sceGuDisable(GU_BLEND);
	sceGuDisable(GU_ALPHA_TEST);
	sceGuDisable(GU_TEXTURE_2D);
	sceGuDepthFunc(GU_GEQUAL);
}
