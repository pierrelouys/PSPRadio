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
#include <time.h>

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>

#include <pspgu.h>
#include <pspgum.h>

#include "SandbergUI.h"

#define		CUBE_COUNT	7
#define		COLOR_LIGHT	0xFFE8709C
#define		COLOR_DARK	0xFF785060

#define 	FX_TIME_LENGTH	15.0f


static struct CSandbergUI::NVertex __attribute__((aligned(16))) vertices[12*3] =
{
	{COLOR_LIGHT, 0, 0,-1,-1,-1, 0.2}, // 0
	{COLOR_LIGHT, 0, 0,-1,-1, 1, 0.2}, // 4
	{COLOR_LIGHT, 0, 0,-1, 1, 1, 0.2}, // 5

	{COLOR_LIGHT, 0, 0,-1,-1,-1, 0.2}, // 0
	{COLOR_LIGHT, 0, 0,-1, 1, 1, 0.2}, // 5
	{COLOR_LIGHT, 0, 0,-1, 1,-1, 0.2}, // 1


	{COLOR_LIGHT, 0, 0, 1,-1,-1,-0.2}, // 3
	{COLOR_LIGHT, 0, 0, 1, 1,-1,-0.2}, // 2
	{COLOR_LIGHT, 0, 0, 1, 1, 1,-0.2}, // 6

	{COLOR_LIGHT, 0, 0, 1,-1,-1,-0.2}, // 3
	{COLOR_LIGHT, 0, 0, 1, 1, 1,-0.2}, // 6
	{COLOR_LIGHT, 0, 0, 1,-1, 1,-0.2}, // 7


	{COLOR_DARK,  1,-1,-0, 1,-1,-0.2}, // 0
	{COLOR_DARK,  1,-1, 0, 1,-1, 0.2}, // 3
	{COLOR_DARK,  1, 1, 0, 1, 1, 0.2}, // 7

	{COLOR_DARK,  1,-1,-0, 1,-1,-0.2}, // 0
	{COLOR_DARK,  1, 1, 0, 1, 1, 0.2}, // 7
	{COLOR_DARK,  1, 1,-0, 1, 1,-0.2}, // 4


	{COLOR_DARK, -1,-1,-0,-1,-1,-0.2}, // 0
	{COLOR_DARK, -1, 1,-0,-1, 1,-0.2}, // 3
	{COLOR_DARK, -1, 1, 0,-1, 1, 0.2}, // 7

	{COLOR_DARK, -1,-1,-0,-1,-1,-0.2}, // 0
	{COLOR_DARK, -1, 1, 0,-1, 1, 0.2}, // 7
	{COLOR_DARK, -1,-1, 0,-1,-1, 0.2}, // 4


	{COLOR_DARK, -1, 1,-0,-1, 1,-0.2}, // 0
	{COLOR_DARK,  1, 1,-0, 1, 1,-0.2}, // 1
	{COLOR_DARK,  1, 1, 0, 1, 1, 0.2}, // 2

	{COLOR_DARK, -1, 1,-0,-1, 1,-0.2}, // 0
	{COLOR_DARK,  1, 1, 0, 1, 1, 0.2}, // 2
	{COLOR_DARK, -1, 1, 0,-1, 1, 0.2}, // 3


	{COLOR_DARK, -1,-1,-0,-1,-1,-0.2}, // 4
	{COLOR_DARK, -1,-1, 0,-1,-1, 0.2}, // 7
	{COLOR_DARK,  1,-1, 0, 1,-1, 0.2}, // 6

	{COLOR_DARK, -1,-1,-0,-1,-1,-0.2}, // 4
	{COLOR_DARK,  1,-1, 0, 1,-1, 0.2}, // 6
	{COLOR_DARK,  1,-1,-0, 1,-1,-0.2}, // 5
};


static int __attribute__((aligned(16))) fx_list[] =
{
	CSandbergUI::FX_CUBES,
	CSandbergUI::FX_HEART
};

void CSandbergUI::InitFX(char *strCWD)
{
	int	return_value;
	char	filename[MAXPATHLEN];

	gettimeofday(&tval,0);
	curr = start = tval.tv_sec + (((float)tval.tv_usec) / 1000000);
	current_fx = fx_list[0];

	sprintf(filename, "%s/SandbergUI/%s", strCWD, "heart.p3o");
	return_value = p3oloader.jsaP3OLoadFile(filename, &object_info, false);
	if (return_value != jsaP3OLoad::JSAP3O_ERROR_OK)
	{
		Log(LOG_ERROR, "JSA:Error loading!!");
	}
//TEST
	#include <jsaP3OLoad.h>
	jsaP3OLoad::jsaP3OVertex *current = object_info.vertices;
	for (int i = 0  ; i < object_info.vertice_count  ; i++)
	{
		current->color = 0xFF0000FF;
		current++;
	}
}

void CSandbergUI::RenderFX(void)
{
	gettimeofday(&tval,0);
	curr = tval.tv_sec + (((float)tval.tv_usec) / 1000000);

	if ((curr-start) >= FX_TIME_LENGTH)
	{
		current_fx = fx_list[(current_fx + 1) % (sizeof(fx_list)/sizeof(int))];
		start = curr;
	}

	switch(current_fx)
	{
		case	FX_CUBES:
		{
			RenderFX_1();
		}
		break;
		case	FX_HEART:
		{
			RenderFX_2();
		}
		break;
	}

}

void CSandbergUI::RenderFX_1(void)
{
	static int val = 100;

	sceGuAmbient(0x66666666);
	sceGuColor(0x222222);

	// setup matrices for cube
	for (int count = 0 ; count < CUBE_COUNT ; count++)
		{
		/* Draw cube */
		sceGumMatrixMode(GU_MODEL);
		sceGumLoadIdentity();
			{
			ScePspFVector3 pos = { -1*CUBE_COUNT/2+count, 0, -4.5f };
			ScePspFVector3 rot = { 	val * 0.59f * (GU_PI/180.0f) / 2.0f,
						val * 0.78f * (GU_PI/180.0f) / 2.0f, 
						val * 1.12f * (GU_PI/180.0f) / 2.0f};
			sceGumTranslate(&pos);
			sceGumRotateXYZ(&rot);
			}
		sceGumDrawArray(GU_TRIANGLES, GU_COLOR_8888|GU_NORMAL_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_3D, 12*3, 0, ::vertices);
		}

  	val++;
}

void CSandbergUI::RenderFX_2(void)
{
	static int val = 100;

	sceGuAmbient(0x66666666);
	sceGuColor(0x222222);
	
	// setup matrices for heart
	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();
		{
		ScePspFVector3 pos = { 0, 0, -4.5f };
		ScePspFVector3 rot = { 	val * 0.59f * (GU_PI/180.0f) / 2.0f,
					val * 0.78f * (GU_PI/180.0f) / 2.0f, 
					val * 1.12f * (GU_PI/180.0f) / 2.0f};
		sceGumTranslate(&pos);
		sceGumRotateXYZ(&rot);

		/* Draw heart */
		sceGumDrawArray(GU_TRIANGLES, GU_COLOR_8888|GU_NORMAL_32BITF|GU_VERTEX_32BITF|GU_INDEX_16BIT|GU_TRANSFORM_3D, object_info.face_count*3, object_info.faces, object_info.vertices);
		}

  	val++;
}
