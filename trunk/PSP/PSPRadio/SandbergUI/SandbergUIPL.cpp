/* 
	PSPRadio / Music streaming client for the PSP. (Initial Release: Sept. 2005)
	PSPRadio Copyright (C) 2005 Rafael Cabezas a.k.a. Raf
	SandbergUI Copyright (C) 2005 Jesper Sandberg

	
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
#include <stdarg.h>

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>

#include <pspgu.h>
#include <pspgum.h>

#include "SandbergUI.h"


#define PL_TEXT_LENGTH		80
#define PL_TEXT_SCALE_X		2.0f
#define PL_TEXT_SCALE_Z		2.0f

#define NUMBER_OF_CHARS		(2*26+1+10+12)

static CSandbergUI::char_map __attribute__((aligned(16))) char_mapping[NUMBER_OF_CHARS] =
{
	{' ', (float)( 0*16)/256, (float)( 3*16)/64, (float)( 1*16)/256, (float)( 4*16)/64},

	{'a', (float)( 0*16)/256, (float)( 0*16)/64, (float)( 1*16)/256, (float)( 1*16)/64},
	{'b', (float)( 1*16)/256, (float)( 0*16)/64, (float)( 2*16)/256, (float)( 1*16)/64},
	{'c', (float)( 2*16)/256, (float)( 0*16)/64, (float)( 3*16)/256, (float)( 1*16)/64},
	{'d', (float)( 3*16)/256, (float)( 0*16)/64, (float)( 4*16)/256, (float)( 1*16)/64},
	{'e', (float)( 4*16)/256, (float)( 0*16)/64, (float)( 5*16)/256, (float)( 1*16)/64},
	{'f', (float)( 5*16)/256, (float)( 0*16)/64, (float)( 6*16)/256, (float)( 1*16)/64},
	{'g', (float)( 6*16)/256, (float)( 0*16)/64, (float)( 7*16)/256, (float)( 1*16)/64},
	{'h', (float)( 7*16)/256, (float)( 0*16)/64, (float)( 8*16)/256, (float)( 1*16)/64},
	{'i', (float)( 8*16)/256, (float)( 0*16)/64, (float)( 9*16)/256, (float)( 1*16)/64},
	{'j', (float)( 9*16)/256, (float)( 0*16)/64, (float)(10*16)/256, (float)( 1*16)/64},
	{'k', (float)(10*16)/256, (float)( 0*16)/64, (float)(11*16)/256, (float)( 1*16)/64},
	{'l', (float)(11*16)/256, (float)( 0*16)/64, (float)(12*16)/256, (float)( 1*16)/64},
	{'m', (float)(12*16)/256, (float)( 0*16)/64, (float)(13*16)/256, (float)( 1*16)/64},
	{'n', (float)(13*16)/256, (float)( 0*16)/64, (float)(14*16)/256, (float)( 1*16)/64},
	{'o', (float)(14*16)/256, (float)( 0*16)/64, (float)(15*16)/256, (float)( 1*16)/64},
	{'p', (float)(15*16)/256, (float)( 0*16)/64, (float)(16*16)/256, (float)( 1*16)/64},

	{'q', (float)( 0*16)/256, (float)( 1*16)/64, (float)( 1*16)/256, (float)( 2*16)/64},
	{'r', (float)( 1*16)/256, (float)( 1*16)/64, (float)( 2*16)/256, (float)( 2*16)/64},
	{'s', (float)( 2*16)/256, (float)( 1*16)/64, (float)( 3*16)/256, (float)( 2*16)/64},
	{'t', (float)( 3*16)/256, (float)( 1*16)/64, (float)( 4*16)/256, (float)( 2*16)/64},
	{'u', (float)( 4*16)/256, (float)( 1*16)/64, (float)( 5*16)/256, (float)( 2*16)/64},
	{'v', (float)( 5*16)/256, (float)( 1*16)/64, (float)( 6*16)/256, (float)( 2*16)/64},
	{'w', (float)( 6*16)/256, (float)( 1*16)/64, (float)( 7*16)/256, (float)( 2*16)/64},
	{'x', (float)( 7*16)/256, (float)( 1*16)/64, (float)( 8*16)/256, (float)( 2*16)/64},
	{'y', (float)( 8*16)/256, (float)( 1*16)/64, (float)( 9*16)/256, (float)( 2*16)/64},
	{'z', (float)( 9*16)/256, (float)( 1*16)/64, (float)(10*16)/256, (float)( 2*16)/64},

	{'A', (float)( 0*16)/256, (float)( 0*16)/64, (float)( 1*16)/256, (float)( 1*16)/64},
	{'B', (float)( 1*16)/256, (float)( 0*16)/64, (float)( 2*16)/256, (float)( 1*16)/64},
	{'C', (float)( 2*16)/256, (float)( 0*16)/64, (float)( 3*16)/256, (float)( 1*16)/64},
	{'D', (float)( 3*16)/256, (float)( 0*16)/64, (float)( 4*16)/256, (float)( 1*16)/64},
	{'E', (float)( 4*16)/256, (float)( 0*16)/64, (float)( 5*16)/256, (float)( 1*16)/64},
	{'F', (float)( 5*16)/256, (float)( 0*16)/64, (float)( 6*16)/256, (float)( 1*16)/64},
	{'G', (float)( 6*16)/256, (float)( 0*16)/64, (float)( 7*16)/256, (float)( 1*16)/64},
	{'H', (float)( 7*16)/256, (float)( 0*16)/64, (float)( 8*16)/256, (float)( 1*16)/64},
	{'I', (float)( 8*16)/256, (float)( 0*16)/64, (float)( 9*16)/256, (float)( 1*16)/64},
	{'J', (float)( 9*16)/256, (float)( 0*16)/64, (float)(10*16)/256, (float)( 1*16)/64},
	{'K', (float)(10*16)/256, (float)( 0*16)/64, (float)(11*16)/256, (float)( 1*16)/64},
	{'L', (float)(11*16)/256, (float)( 0*16)/64, (float)(12*16)/256, (float)( 1*16)/64},
	{'M', (float)(12*16)/256, (float)( 0*16)/64, (float)(13*16)/256, (float)( 1*16)/64},
	{'N', (float)(13*16)/256, (float)( 0*16)/64, (float)(14*16)/256, (float)( 1*16)/64},
	{'O', (float)(14*16)/256, (float)( 0*16)/64, (float)(15*16)/256, (float)( 1*16)/64},
	{'P', (float)(15*16)/256, (float)( 0*16)/64, (float)(16*16)/256, (float)( 1*16)/64},

	{'Q', (float)( 0*16)/256, (float)( 1*16)/64, (float)( 1*16)/256, (float)( 2*16)/64},
	{'R', (float)( 1*16)/256, (float)( 1*16)/64, (float)( 2*16)/256, (float)( 2*16)/64},
	{'S', (float)( 2*16)/256, (float)( 1*16)/64, (float)( 3*16)/256, (float)( 2*16)/64},
	{'T', (float)( 3*16)/256, (float)( 1*16)/64, (float)( 4*16)/256, (float)( 2*16)/64},
	{'U', (float)( 4*16)/256, (float)( 1*16)/64, (float)( 5*16)/256, (float)( 2*16)/64},
	{'V', (float)( 5*16)/256, (float)( 1*16)/64, (float)( 6*16)/256, (float)( 2*16)/64},
	{'W', (float)( 6*16)/256, (float)( 1*16)/64, (float)( 7*16)/256, (float)( 2*16)/64},
	{'X', (float)( 7*16)/256, (float)( 1*16)/64, (float)( 8*16)/256, (float)( 2*16)/64},
	{'Y', (float)( 8*16)/256, (float)( 1*16)/64, (float)( 9*16)/256, (float)( 2*16)/64},
	{'Z', (float)( 9*16)/256, (float)( 1*16)/64, (float)(10*16)/256, (float)( 2*16)/64},
	{'0', (float)(10*16)/256, (float)( 1*16)/64, (float)(11*16)/256, (float)( 2*16)/64},
	{'1', (float)(11*16)/256, (float)( 1*16)/64, (float)(12*16)/256, (float)( 2*16)/64},
	{'2', (float)(12*16)/256, (float)( 1*16)/64, (float)(13*16)/256, (float)( 2*16)/64},
	{'3', (float)(13*16)/256, (float)( 1*16)/64, (float)(14*16)/256, (float)( 2*16)/64},
	{'4', (float)(14*16)/256, (float)( 1*16)/64, (float)(15*16)/256, (float)( 2*16)/64},
	{'5', (float)(15*16)/256, (float)( 1*16)/64, (float)(16*16)/256, (float)( 2*16)/64},

	{'6', (float)( 0*16)/256, (float)( 2*16)/64, (float)( 1*16)/256, (float)( 3*16)/64},
	{'7', (float)( 1*16)/256, (float)( 2*16)/64, (float)( 2*16)/256, (float)( 3*16)/64},
	{'8', (float)( 2*16)/256, (float)( 2*16)/64, (float)( 3*16)/256, (float)( 3*16)/64},
	{'9', (float)( 3*16)/256, (float)( 2*16)/64, (float)( 4*16)/256, (float)( 3*16)/64},
	{':', (float)( 4*16)/256, (float)( 2*16)/64, (float)( 5*16)/256, (float)( 3*16)/64},
	{'/', (float)( 5*16)/256, (float)( 2*16)/64, (float)( 6*16)/256, (float)( 3*16)/64},
	{'-', (float)( 6*16)/256, (float)( 2*16)/64, (float)( 7*16)/256, (float)( 3*16)/64},
	{'+', (float)( 7*16)/256, (float)( 2*16)/64, (float)( 8*16)/256, (float)( 3*16)/64},
	{'(', (float)( 8*16)/256, (float)( 2*16)/64, (float)( 9*16)/256, (float)( 3*16)/64},
	{')', (float)( 9*16)/256, (float)( 2*16)/64, (float)(10*16)/256, (float)( 3*16)/64},
	{'[', (float)(10*16)/256, (float)( 2*16)/64, (float)(11*16)/256, (float)( 3*16)/64},
	{']', (float)(11*16)/256, (float)( 2*16)/64, (float)(12*16)/256, (float)( 3*16)/64},
	{'*', (float)(12*16)/256, (float)( 2*16)/64, (float)(13*16)/256, (float)( 3*16)/64},
	{'.', (float)(13*16)/256, (float)( 2*16)/64, (float)(14*16)/256, (float)( 3*16)/64},
	{'^', (float)(14*16)/256, (float)( 2*16)/64, (float)(15*16)/256, (float)( 3*16)/64},
	{'"', (float)(15*16)/256, (float)( 2*16)/64, (float)(16*16)/256, (float)( 3*16)/64},
};


static struct CSandbergUI::NCVertex __attribute__((aligned(16))) pl_name_vertices[2*3*(PL_TEXT_LENGTH+2)];

static struct CSandbergUI::NCVertex __attribute__((aligned(16))) pl_entry_vertices[2*3*(PL_TEXT_LENGTH+2)];

void CSandbergUI::InitPL(void)
{
	// Generate the objects which is used for the PL name and the PL entry
	for (int i = 0, index = 0 ; i < PL_TEXT_LENGTH+2; i++)
	{
		index += 2 * 3;

		// vertex 0
		::pl_name_vertices[index + 0].u 	= 0.0f;
		::pl_name_vertices[index + 0].v 	= 0.25f;
		::pl_name_vertices[index + 0].color 	= 0x00ffffff; /**/
		::pl_name_vertices[index + 0].x 	= sinf((((float)i*360)/PL_TEXT_LENGTH)*(M_PI/180))*PL_TEXT_SCALE_X; /**/
		::pl_name_vertices[index + 0].y 	= -0.20f; /**/
		::pl_name_vertices[index + 0].z 	= cosf((((float)i*360)/PL_TEXT_LENGTH)*(M_PI/180))*PL_TEXT_SCALE_Z; /**/
		::pl_name_vertices[index + 0].nx 	= ::pl_name_vertices[index + 0].x;
		::pl_name_vertices[index + 0].ny 	= ::pl_name_vertices[index + 0].y;
		::pl_name_vertices[index + 0].nz 	= -::pl_name_vertices[index + 0].z;

		// vertex 1
		::pl_name_vertices[index + 1].u 	= 0.0f;
		::pl_name_vertices[index + 1].v 	= 0.0f;
		::pl_name_vertices[index + 1].color 	= 0x00ffffff; /**/
		::pl_name_vertices[index + 1].x 	= ::pl_name_vertices[index + 0].x; /**/
		::pl_name_vertices[index + 1].y	 	= 0.20f; /**/
		::pl_name_vertices[index + 1].z 	= ::pl_name_vertices[index + 0].z; /**/
		::pl_name_vertices[index + 1].nx 	= ::pl_name_vertices[index + 1].x;
		::pl_name_vertices[index + 1].ny 	= ::pl_name_vertices[index + 1].y;
		::pl_name_vertices[index + 1].nz 	= -::pl_name_vertices[index + 1].z;

		// vertex 2
		::pl_name_vertices[index + 2].u 	= 0.0625f; 
		::pl_name_vertices[index + 2].v 	= 0.0f; 
		::pl_name_vertices[index + 2].color 	= 0x00ffffff;  /**/
		::pl_name_vertices[index + 2].x 	= sinf((((float)(i+1)*360)/PL_TEXT_LENGTH)*(M_PI/180))*PL_TEXT_SCALE_X; /**/
		::pl_name_vertices[index + 2].y 	= 0.20f; /**/
		::pl_name_vertices[index + 2].z 	= cosf((((float)(i+1)*360)/PL_TEXT_LENGTH)*(M_PI/180))*PL_TEXT_SCALE_Z; /**/
		::pl_name_vertices[index + 2].nx 	= ::pl_name_vertices[index + 2].x;
		::pl_name_vertices[index + 2].ny 	= ::pl_name_vertices[index + 2].y;
		::pl_name_vertices[index + 2].nz 	= -::pl_name_vertices[index + 2].z;

		// vertex 0
		::pl_name_vertices[index + 3].u 	= ::pl_name_vertices[index + 0].u; 
		::pl_name_vertices[index + 3].v 	= ::pl_name_vertices[index + 0].v; 
		::pl_name_vertices[index + 3].color 	= ::pl_name_vertices[index + 0].color; 
		::pl_name_vertices[index + 3].x 	= ::pl_name_vertices[index + 0].x;
		::pl_name_vertices[index + 3].y 	= ::pl_name_vertices[index + 0].y; /**/
		::pl_name_vertices[index + 3].z 	= ::pl_name_vertices[index + 0].z;
		::pl_name_vertices[index + 3].nx 	= ::pl_name_vertices[index + 3].x;
		::pl_name_vertices[index + 3].ny 	= ::pl_name_vertices[index + 3].y;
		::pl_name_vertices[index + 3].nz 	= -::pl_name_vertices[index + 3].z;

		// vertex 2
		::pl_name_vertices[index + 4].u 	= ::pl_name_vertices[index + 2].u;
		::pl_name_vertices[index + 4].v 	= ::pl_name_vertices[index + 2].v;
		::pl_name_vertices[index + 4].color 	= ::pl_name_vertices[index + 2].color;
		::pl_name_vertices[index + 4].x 	= ::pl_name_vertices[index + 2].x;
		::pl_name_vertices[index + 4].y 	= ::pl_name_vertices[index + 2].y; /**/
		::pl_name_vertices[index + 4].z 	= ::pl_name_vertices[index + 2].z;
		::pl_name_vertices[index + 4].nx 	= ::pl_name_vertices[index + 4].x;
		::pl_name_vertices[index + 4].ny 	= ::pl_name_vertices[index + 4].y;
		::pl_name_vertices[index + 4].nz 	= -::pl_name_vertices[index + 4].z;

		// vertex 3
		::pl_name_vertices[index + 5].u 	= 0.0625f; 
		::pl_name_vertices[index + 5].v 	= 0.25f; 
		::pl_name_vertices[index + 5].color 	= 0x00ffffff;  /**/
		::pl_name_vertices[index + 5].x 	= ::pl_name_vertices[index + 2].x;
		::pl_name_vertices[index + 5].y 	= -0.20f; /**/
		::pl_name_vertices[index + 5].z 	= ::pl_name_vertices[index + 2].z;
		::pl_name_vertices[index + 5].nx 	= ::pl_name_vertices[index + 5].x;
		::pl_name_vertices[index + 5].ny 	= ::pl_name_vertices[index + 5].y;
		::pl_name_vertices[index + 5].nz 	= -::pl_name_vertices[index + 5].z;
	}

	memcpy(::pl_entry_vertices, ::pl_name_vertices, sizeof(pl_name_vertices));

	// flush cache before we use the object
	sceKernelDcacheWritebackAll();
}

void CSandbergUI::RenderPL(void)
{
	static int rotate = 0;

	RenderScroller(pl_entry, ::pl_entry_vertices, rotate,  2.0f);
	RenderScroller(pl_name,  ::pl_name_vertices,  rotate, -2.0f);

	RenderMetaDataFrame();
	RenderOptions(RENDER_METADATA);

	rotate++;
}

void CSandbergUI::RenderScroller(char *text, struct NCVertex *vertices, int rotate, float x_offset)
{
	sceGuEnable(GU_TEXTURE_2D);

	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();
	{
		ScePspFVector3 pos = { x_offset, -1, -4.0f };
		ScePspFVector3 rot = { 0, -rotate * 0.98f * (M_PI/180.0f), 0};
		sceGumRotateXYZ(&rot);
		sceGumTranslate(&pos);
	}

	// setup texture
	(void)tcache.jsaTCacheSetTexture(TEX_FONT_LARGE);
	sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);
	sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	sceGuTexScale(1.0f,1.0f);
	sceGuTexOffset(0.0f,0.0f);
	sceGuAmbient(0x66666666);
	sceGuColor(0x222222);

	// Update texture coordinates from text
	if (text)
	{
	int len = strlen(text);

		for (int i = 0, index = 0 ; i < PL_TEXT_LENGTH+2; i++)
		{
			struct char_map	*character;
			index += 2 * 3;

			/* Make sure we don't read past the end of the text, pad with 'space' */
			if (i < len)
			{
				/* Insert start symbol */
				if (i == 0)
				{
					character = FindCharMap('^');
				}
				else
				{
					character = FindCharMap(text[i-1]);
				}
			}
			else
			{
				character = FindCharMap(' ');
			}

			/* Insert end symbol */
			if (i == PL_TEXT_LENGTH-2)
				{
					character = FindCharMap('"');
				}

			// vertex 0
			vertices[index + 0].u 	= character->min_x;
			vertices[index + 0].v 	= character->max_y;

			// vertex 1
			vertices[index + 1].u 	= character->min_x;
			vertices[index + 1].v 	= character->min_y;

			// vertex 2
			vertices[index + 2].u 	= character->max_x; 
			vertices[index + 2].v 	= character->min_y; 

			// vertex 0
			vertices[index + 3].u 	= vertices[index + 0].u; 
			vertices[index + 3].v 	= vertices[index + 0].v; 

			// vertex 2
			vertices[index + 4].u 	= vertices[index + 2].u;
			vertices[index + 4].v 	= vertices[index + 2].v;

			// vertex 3
			vertices[index + 5].u 	= character->max_x; 
			vertices[index + 5].v 	= character->max_y; 
		}
		// flush cache before we use the object
		sceKernelDcacheWritebackAll();
	}

	// draw text-rotator
	sceGumDrawArray(GU_TRIANGLES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_NORMAL_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_3D,2*3*PL_TEXT_LENGTH,0,vertices);
	sceGuDisable(GU_TEXTURE_2D);
}

CSandbergUI::char_map* CSandbergUI::FindCharMap(char index)
{
struct char_map		*character = NULL;

	/* Find the texture coordinates for the character */
	for (int i = 0 ; i < NUMBER_OF_CHARS ; i++)
	{
		if (index == char_mapping[i].char_index)
			{
			character = &char_mapping[i];
			break;
			}
	}

	/* If we didn't find any matching character, then return a 'space' */
	if (character == NULL)
		{
		character = &char_mapping[0];
		}

	return character;
}

void CSandbergUI::RenderMetaDataFrame(void)
{
	sceGuEnable(GU_TEXTURE_2D);
	// setup texture
	(void)tcache.jsaTCacheSetTexture(TEX_PLATE);
	sceGuTexFunc(GU_TFX_BLEND,GU_TCC_RGBA);
	sceGuTexEnvColor(0xFF000000);
	sceGuDepthFunc(GU_ALWAYS);
	sceGuBlendFunc(GU_ADD, GU_SRC_COLOR, GU_DST_COLOR, 0, 0);
	sceGuEnable(GU_BLEND);

	struct Vertex* c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	c_vertices[0].u = 0; c_vertices[0].v = 0;
	c_vertices[0].x = 112-48; c_vertices[0].y = 92; c_vertices[0].z = 0;
	c_vertices[0].color = 0xFFFFFFFF;
	c_vertices[1].u = 64; c_vertices[1].v = 64;
	c_vertices[1].x = 368+48; c_vertices[1].y = 176; c_vertices[1].z = 0;
	c_vertices[1].color = 0xFFFFFFFF;
	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,c_vertices);

	sceGuDisable(GU_BLEND);
	sceGuDisable(GU_TEXTURE_2D);

	struct Vertex* l_vertices = (struct Vertex*)sceGuGetMemory(5 * sizeof(struct Vertex));
	l_vertices[0].x = 112-48;	l_vertices[0].y =  92; 	l_vertices[0].color = 0xFFFFFFFF;
	l_vertices[1].x = 368+48;	l_vertices[1].y =  92; 	l_vertices[1].color = 0xFFFFFFFF;
	l_vertices[2].x = 368+48;	l_vertices[2].y = 176; 	l_vertices[2].color = 0xFFFFFFFF;
	l_vertices[3].x = 112-48;	l_vertices[3].y = 176; 	l_vertices[3].color = 0xFFFFFFFF;
	l_vertices[4].x = 112-48;	l_vertices[4].y =  92; 	l_vertices[4].color = 0xFFFFFFFF;

	sceGuDrawArray(GU_LINE_STRIP,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,5,0,l_vertices);

	sceGuDepthFunc(GU_GEQUAL);
}
