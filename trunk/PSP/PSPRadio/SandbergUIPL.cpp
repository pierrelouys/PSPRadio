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

#include "SandbergUIFont.cpp"


struct Vertex
{
	float u, v;
	unsigned int color;
	float x,y,z;
};

static struct Vertex __attribute__((aligned(16))) vertices[2*3] =
{
	{0.0f, 1.0f, 0xffffffff,-1,-1, 1.5}, // 0
	{0.0f, 0.0f, 0xffffffff,-1, 1, 1.5}, // 4
	{1.0f, 0.0f, 0xffffffff, 1, 1, 1.5}, // 5

	{0.0f, 1.0f, 0xffffffff,-1,-1, 1.5}, // 0
	{1.0f, 0.0f, 0xffffffff, 1, 1, 1.5}, // 5
	{1.0f, 1.0f, 0xffffffff, 1,-1, 1.5}, // 1
};

void CSandbergUI::RenderPL(void)
{
	static int val = 0;

	sceGuEnable(GU_TEXTURE_2D);

	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();
	{
		ScePspFVector3 pos = { -1, -1, -3.5f };
		ScePspFVector3 rot = { 0 * 0.79f * (M_PI/180.0f), 0 * 0.98f * (M_PI/180.0f), 0 * 1.32f * (M_PI/180.0f) };
		sceGumRotateXYZ(&rot);
		sceGumTranslate(&pos);
	}

	// setup texture
	sceGuTexMode(GU_PSM_8888,0,0,0);
	sceGuTexImage(0,256,64,256,::font_01);
	sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);
	sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	sceGuTexScale(1.0f,1.0f);
	sceGuTexOffset(0.0f,0.0f);
	sceGuAmbientColor(0xffffffff);

	sceGuAlphaFunc(GU_ALWAYS, 0, 0xff);
	sceGuEnable(GU_ALPHA_TEST);

	// draw cube
	sceGumDrawArray(GU_TRIANGLES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D,2*3,0,::vertices);
/*
	sceGuDisable(GU_TEXTURE_2D);
	sceGuDisable(GU_ALPHA_TEST);
*/
	val++;
}
