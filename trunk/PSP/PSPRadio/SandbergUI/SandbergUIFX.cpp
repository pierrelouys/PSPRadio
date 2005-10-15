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
#include <fastmath.h>
#include <stdarg.h>

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>

#include <pspgu.h>
#include <pspgum.h>

#include "SandbergUI.h"

#define		CUBE_COUNT	7

struct Vertex
{
	unsigned int color;
	float x,y,z;
};

static struct Vertex __attribute__((aligned(16))) vertices[12*3] =
{
	{0xffCC8866,-1,-1, 0.2}, // 0
	{0xffCC8866,-1, 1, 0.2}, // 4
	{0xffCC8866, 1, 1, 0.2}, // 5

	{0xffCC8866,-1,-1, 0.2}, // 0
	{0xffCC8866, 1, 1, 0.2}, // 5
	{0xffCC8866, 1,-1, 0.2}, // 1

	{0xffCC8866,-1,-1,-0.2}, // 3
	{0xffCC8866, 1,-1,-0.2}, // 2
	{0xffCC8866, 1, 1,-0.2}, // 6

	{0xffCC8866,-1,-1,-0.2}, // 3
	{0xffCC8866, 1, 1,-0.2}, // 6
	{0xffCC8866,-1, 1,-0.2}, // 7

	{0xffCC8866, 1,-1,-0.2}, // 0
	{0xffCC8866, 1,-1, 0.2}, // 3
	{0xffCC8866, 1, 1, 0.2}, // 7

	{0xffCC8866, 1,-1,-0.2}, // 0
	{0xffCC8866, 1, 1, 0.2}, // 7
	{0xffCC8866, 1, 1,-0.2}, // 4

	{0xffCC8866,-1,-1,-0.2}, // 0
	{0xffCC8866,-1, 1,-0.2}, // 3
	{0xffCC8866,-1, 1, 0.2}, // 7

	{0xffCC8866,-1,-1,-0.2}, // 0
	{0xffCC8866,-1, 1, 0.2}, // 7
	{0xffCC8866,-1,-1, 0.2}, // 4

	{0xffCC8866,-1, 1,-0.2}, // 0
	{0xffCC8866, 1, 1,-0.2}, // 1
	{0xffCC8866, 1, 1, 0.2}, // 2

	{0xffCC8866,-1, 1,-0.2}, // 0
	{0xffCC8866, 1, 1, 0.2}, // 2
	{0xffCC8866,-1, 1, 0.2}, // 3

	{0xffCC8866,-1,-1,-0.2}, // 4
	{0xffCC8866,-1,-1, 0.2}, // 7
	{0xffCC8866, 1,-1, 0.2}, // 6

	{0xffCC8866,-1,-1,-0.2}, // 4
	{0xffCC8866, 1,-1, 0.2}, // 6
	{0xffCC8866, 1,-1,-0.2}, // 5
};

void CSandbergUI::InitFX(void)
	{
	}

void CSandbergUI::RenderFX(void)
{	
	static int val = 100;

	sceGuAmbient(0x66666666);

	sceGuDisable(GU_TEXTURE_2D);

	// setup matrices for cube
	for (int count = 0 ; count < CUBE_COUNT ; count++)
		{
		/* Draw cube */
		sceGumMatrixMode(GU_MODEL);
		sceGumLoadIdentity();
			{
			ScePspFVector3 pos = { -1*CUBE_COUNT/2+count, 0, -4.5f };
			ScePspFVector3 rot = { 	val * 0.59f * (M_PI/180.0f) / 2.0f,
						val * 0.78f * (M_PI/180.0f) / 2.0f, 
						val * 1.12f * (M_PI/180.0f) / 2.0f};
			sceGumRotateXYZ(&rot);
			sceGumTranslate(&pos);
			}
		sceGuColor(0x222222);
		sceGumDrawArray(GU_TRIANGLES, GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D, 12*3, 0, ::vertices);
		}

  	val++;
}
