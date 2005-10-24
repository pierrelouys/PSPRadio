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

#define ROTSIZE		128

static float __attribute__((aligned(16))) sintable[] = {

0.000000f,0.004907f,0.009802f,0.014673f,0.019509f,0.024298f,0.029028f,0.033689f,
0.038268f,0.042756f,0.047140f,0.051410f,0.055557f,0.059570f,0.063439f,0.067156f,
0.070711f,0.074095f,0.077301f,0.080321f,0.083147f,0.085773f,0.088192f,0.090399f,
0.092388f,0.094154f,0.095694f,0.097003f,0.098079f,0.098918f,0.099518f,0.099880f,
0.100000f,0.099880f,0.099518f,0.098918f,0.098079f,0.097003f,0.095694f,0.094154f,
0.092388f,0.090399f,0.088192f,0.085773f,0.083147f,0.080321f,0.077301f,0.074095f,
0.070711f,0.067156f,0.063439f,0.059570f,0.055557f,0.051410f,0.047140f,0.042756f,
0.038268f,0.033689f,0.029028f,0.024298f,0.019509f,0.014673f,0.009802f,0.004907f,
-0.000000f,-0.004907f,-0.009802f,-0.014673f,-0.019509f,-0.024298f,-0.029028f,-0.033689f,
-0.038268f,-0.042756f,-0.047140f,-0.051410f,-0.055557f,-0.059570f,-0.063439f,-0.067156f,
-0.070711f,-0.074095f,-0.077301f,-0.080321f,-0.083147f,-0.085773f,-0.088192f,-0.090399f,
-0.092388f,-0.094154f,-0.095694f,-0.097003f,-0.098079f,-0.098918f,-0.099518f,-0.099880f,
-0.100000f,-0.099880f,-0.099518f,-0.098918f,-0.098079f,-0.097003f,-0.095694f,-0.094154f,
-0.092388f,-0.090399f,-0.088192f,-0.085773f,-0.083147f,-0.080321f,-0.077301f,-0.074095f,
-0.070711f,-0.067156f,-0.063439f,-0.059570f,-0.055557f,-0.051410f,-0.047140f,-0.042756f,
-0.038268f,-0.033689f,-0.029028f,-0.024298f,-0.019509f,-0.014673f,-0.009802f,-0.004907f,
};


static float __attribute__((aligned(16))) costable[] = {

0.100000f,0.099880f,0.099518f,0.098918f,0.098079f,0.097003f,0.095694f,0.094154f,
0.092388f,0.090399f,0.088192f,0.085773f,0.083147f,0.080321f,0.077301f,0.074095f,
0.070711f,0.067156f,0.063439f,0.059570f,0.055557f,0.051410f,0.047140f,0.042756f,
0.038268f,0.033689f,0.029028f,0.024298f,0.019509f,0.014673f,0.009802f,0.004907f,
-0.000000f,-0.004907f,-0.009802f,-0.014673f,-0.019509f,-0.024298f,-0.029028f,-0.033689f,
-0.038268f,-0.042756f,-0.047140f,-0.051410f,-0.055557f,-0.059570f,-0.063439f,-0.067156f,
-0.070711f,-0.074095f,-0.077301f,-0.080321f,-0.083147f,-0.085773f,-0.088192f,-0.090399f,
-0.092388f,-0.094154f,-0.095694f,-0.097003f,-0.098079f,-0.098918f,-0.099518f,-0.099880f,
-0.100000f,-0.099880f,-0.099518f,-0.098918f,-0.098079f,-0.097003f,-0.095694f,-0.094154f,
-0.092388f,-0.090399f,-0.088192f,-0.085773f,-0.083147f,-0.080321f,-0.077301f,-0.074095f,
-0.070711f,-0.067156f,-0.063439f,-0.059570f,-0.055557f,-0.051410f,-0.047140f,-0.042755f,
-0.038268f,-0.033689f,-0.029028f,-0.024298f,-0.019509f,-0.014673f,-0.009802f,-0.004907f,
0.000000f,0.004907f,0.009802f,0.014673f,0.019509f,0.024298f,0.029028f,0.033689f,
0.038268f,0.042755f,0.047140f,0.051410f,0.055557f,0.059570f,0.063439f,0.067156f,
0.070711f,0.074095f,0.077301f,0.080321f,0.083147f,0.085773f,0.088192f,0.090399f,
0.092388f,0.094154f,0.095694f,0.097003f,0.098079f,0.098918f,0.099518f,0.099880f,
};


void CSandbergUI::RenderLogo(void)
{
static int rot = 0;

	sceGuEnable(GU_TEXTURE_2D);

	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();
	{
		ScePspFVector3 pos = { 0, 1.3, -2.5f };
		sceGumTranslate(&pos);
	}

	// setup texture
	(void)tcache.jsaTCacheSetTexture(TEX_LOGO);
	sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);
	sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	sceGuTexScale(1.0f,1.0f);
	sceGuTexOffset(0.0f,0.0f);
	sceGuAmbientColor(0xffffffff);

	sceGuDepthFunc(GU_ALWAYS);

	struct Vertex* l_vertices = (struct Vertex*)sceGuGetMemory(2 * 3 * sizeof(struct Vertex));

	sceGuColor(0xFFFFFFFF);
	// Vertex 1
	l_vertices[0].x = -2.0f + ::sintable[(rot+32)%ROTSIZE];
	l_vertices[0].y = -0.5f + ::costable[(rot)%ROTSIZE];
	l_vertices[0].z = ::costable[(rot+64)%ROTSIZE];
	l_vertices[0].u = 0.0f;
	l_vertices[0].v = 1.0f;
	l_vertices[3].x = -2.0f + ::sintable[(rot+32)%ROTSIZE];
	l_vertices[3].y = -0.5f + ::costable[(rot)%ROTSIZE];
	l_vertices[3].z = ::costable[(rot+64)%ROTSIZE];
	l_vertices[3].u = 0.0f;
	l_vertices[3].v = 1.0f;

	// Vertex 2
	l_vertices[1].x = -2.0f + ::sintable[(rot)%ROTSIZE];
	l_vertices[1].y =  0.5f + ::costable[(rot+32)%ROTSIZE];
	l_vertices[1].z = ::costable[(rot+96)%ROTSIZE];
	l_vertices[1].u = 0.0f;
	l_vertices[1].v = 0.0f;

	// Vertex 3
	l_vertices[2].x =  2.0f + ::sintable[(rot+64)%ROTSIZE];
	l_vertices[2].y =  0.5f + ::costable[(rot+96)%ROTSIZE];
	l_vertices[2].z = ::costable[(rot)%ROTSIZE];
	l_vertices[2].u = 1.0f;
	l_vertices[2].v = 0.0f;
	l_vertices[4].x =  2.0f + ::sintable[(rot+64)%ROTSIZE];
	l_vertices[4].y =  0.5f + ::costable[(rot+96)%ROTSIZE];
	l_vertices[4].z = ::costable[(rot)%ROTSIZE];
	l_vertices[4].u = 1.0f;
	l_vertices[4].v = 0.0f;

	// Vertex 4
	l_vertices[5].x =  2.0f + ::sintable[(rot+96)%ROTSIZE];
	l_vertices[5].y = -0.5f + ::costable[(rot+64)%ROTSIZE];
	l_vertices[5].z = ::costable[(rot+32)%ROTSIZE];
	l_vertices[5].u = 1.0f;
	l_vertices[5].v = 1.0f;

	// draw logo
	sceGumDrawArray(GU_TRIANGLES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D,2*3,0,l_vertices);

	sceGuDepthFunc(GU_GEQUAL);
	sceGuDisable(GU_TEXTURE_2D);
	rot++;
}

void CSandbergUI::RenderCommands(void)
{
	sceGuEnable(GU_TEXTURE_2D);
	sceGuDepthFunc(GU_ALWAYS);
	// setup texture
	(void)tcache.jsaTCacheSetTexture(TEX_COMMANDS);
	sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);

	struct Vertex* c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	c_vertices[0].u = 0; c_vertices[0].v = 0;
	c_vertices[0].x = 8; c_vertices[0].y = 8; c_vertices[0].z = 0;
	c_vertices[1].u = 64; c_vertices[1].v = 64;
	c_vertices[1].x = 8+64; c_vertices[1].y = 8+64; c_vertices[1].z = 0;
	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,c_vertices);
	sceGuDepthFunc(GU_GEQUAL);
	sceGuDisable(GU_TEXTURE_2D);
}
