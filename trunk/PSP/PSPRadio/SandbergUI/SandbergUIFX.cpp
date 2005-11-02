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
#include <jsaP3OLoad.h>

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

#define	BLUR_REPEAT		50
#define BLUR_INCREASE	2
#define SIN_COUNT       128

static float __attribute__((aligned(16))) sintable[] = {
	0.000000f,1.570166f,3.136549f,4.695375f,6.242890f,7.775366f,9.289110f,10.780475f,
	12.245870f,13.681763f,15.084695f,16.451287f,17.778247f,19.062378f,20.300585f,21.489887f,
	22.627417f,23.710436f,24.736335f,25.702641f,26.607027f,27.447315f,28.221480f,28.927657f,
	29.564145f,30.129410f,30.622091f,31.041000f,31.385129f,31.653648f,31.845911f,31.961455f,
	32.000000f,31.961455f,31.845911f,31.653648f,31.385129f,31.041000f,30.622090f,30.129411f,
	29.564145f,28.927658f,28.221482f,27.447315f,26.607029f,25.702640f,24.736335f,23.710434f,
	22.627417f,21.489889f,20.300584f,19.062379f,17.778245f,16.451288f,15.084699f,13.681762f,
	12.245872f,10.780473f,9.289111f,7.775362f,6.242890f,4.695378f,3.136547f,1.570167f,
	-0.000003f,-1.570165f,-3.136545f,-4.695376f,-6.242888f,-7.775368f,-9.289109f,-10.780479f,
	-12.245870f,-13.681760f,-15.084697f,-16.451286f,-17.778250f,-19.062377f,-20.300582f,-21.489887f,
	-22.627415f,-23.710438f,-24.736334f,-25.702643f,-26.607024f,-27.447314f,-28.221481f,-28.927660f,
	-29.564143f,-30.129410f,-30.622092f,-31.040998f,-31.385129f,-31.653649f,-31.845912f,-31.961454f,
	-32.000000f,-31.961454f,-31.845912f,-31.653649f,-31.385128f,-31.040998f,-30.622092f,-30.129410f,
	-29.564143f,-28.927660f,-28.221481f,-27.447314f,-26.607032f,-25.702643f,-24.736333f,-23.710432f,
	-22.627420f,-21.489887f,-20.300582f,-19.062383f,-17.778249f,-16.451286f,-15.084690f,-13.681767f,
	-12.245869f,-10.780471f,-9.289115f,-7.775367f,-6.242887f,-4.695368f,-3.136552f,-1.570164f,
};


static float __attribute__((aligned(16))) costable[] =
{
	32.000000f,31.961455f,31.845911f,31.653648f,31.385129f,31.041000f,30.622091f,30.129410f,
	29.564145f,28.927657f,28.221481f,27.447316f,26.607028f,25.702641f,24.736335f,23.710436f,
	22.627417f,21.489887f,20.300585f,19.062377f,17.778248f,16.451289f,15.084697f,13.681763f,
	12.245870f,10.780475f,9.289108f,7.775368f,6.242891f,4.695376f,3.136548f,1.570165f,
	-0.000001f,-1.570164f,-3.136547f,-4.695375f,-6.242890f,-7.775367f,-9.289111f,-10.780474f,
	-12.245869f,-13.681763f,-15.084692f,-16.451289f,-17.778245f,-19.062379f,-20.300584f,-21.489889f,
	-22.627417f,-23.710434f,-24.736335f,-25.702640f,-26.607029f,-27.447315f,-28.221479f,-28.927658f,
	-29.564144f,-30.129411f,-30.622090f,-31.041001f,-31.385129f,-31.653648f,-31.845911f,-31.961455f,
	-32.000000f,-31.961455f,-31.845912f,-31.653648f,-31.385129f,-31.041000f,-30.622091f,-30.129409f,
	-29.564145f,-28.927659f,-28.221480f,-27.447316f,-26.607026f,-25.702641f,-24.736337f,-23.710435f,
	-22.627419f,-21.489885f,-20.300586f,-19.062375f,-17.778254f,-16.451290f,-15.084694f,-13.681758f,
	-12.245874f,-10.780476f,-9.289106f,-7.775372f,-6.242892f,-4.695373f,-3.136542f,-1.570170f,
	0.000000f,1.570170f,3.136543f,4.695374f,6.242893f,7.775373f,9.289106f,10.780476f,
	12.245875f,13.681758f,15.084695f,16.451291f,17.778241f,19.062376f,20.300586f,21.489891f,
	22.627414f,23.710436f,24.736337f,25.702637f,26.607026f,27.447317f,28.221484f,28.927656f,
	29.564145f,30.129412f,30.622089f,31.041000f,31.385130f,31.653649f,31.845911f,31.961455f,
};

static int __attribute__((aligned(16))) fx_list[] =
{
	CSandbergUI::FX_CUBES,
	CSandbergUI::FX_HEART,
//	CSandbergUI::FX_PARTICLES,
	CSandbergUI::FX_BLUR,
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
	
	psystem.jsaParticleSystemInit(1000);

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

		case	FX_PARTICLES:
		{
			RenderFX_3();
		}
		break;

		case	FX_BLUR:
		{
			RenderFX_4();
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

void CSandbergUI::RenderFX_3(void)
{
	sceGuEnable(GU_TEXTURE_2D);

	sceGuAlphaFunc(GU_GREATER,0x00,0xff);
	sceGuEnable(GU_ALPHA_TEST);

	// setup texture
	(void)tcache.jsaTCacheSetTexture(TEX_PARTICLE_01);
	sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);

	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_DST_COLOR, 0, 0);
	sceGuEnable(GU_BLEND);
	sceGuDepthFunc(GU_ALWAYS);
	sceGuTexEnvColor(0xFFFFFFFF);


	// setup matrices for heart
	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();
		{
		ScePspFVector3 pos = {	0, 0, -4.5f };
		sceGumTranslate(&pos);
		}

	psystem.jsaParticleSystemRender();
	psystem.jsaParticleSystemUpdate();
	sceGuDepthFunc(GU_GEQUAL);
	sceGuDisable(GU_BLEND);
	sceGuDisable(GU_ALPHA_TEST);
	sceGuDisable(GU_TEXTURE_2D);
}

void CSandbergUI::RenderFX_4(void)
{
	static int		offset = 0;
	float			x1, x2, y1, y2;
	unsigned long	color = 0xff888888;

	sceGuEnable(GU_TEXTURE_2D);
	// setup texture
	(void)tcache.jsaTCacheSetTexture(TEX_BLUR);
	sceGuTexFunc(GU_TFX_BLEND,GU_TCC_RGBA);
	sceGuTexEnvColor(0xFF000000);
	sceGuDepthFunc(GU_ALWAYS);
	sceGuBlendFunc(GU_ADD, GU_SRC_COLOR, GU_DST_COLOR, 0, 0);
	sceGuEnable(GU_BLEND);
	sceGuAlphaFunc(GU_GREATER,0x80,0xff);
	sceGuEnable(GU_ALPHA_TEST);


	for (int i = 0 ; i < BLUR_REPEAT * BLUR_INCREASE ; i += BLUR_INCREASE)
	{
		struct Vertex* c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));

		x1 = 240 - (256/2) + (sintable[offset%SIN_COUNT] * ((float) i / BLUR_REPEAT)) - i;
		x2 = 240 + (256/2) + (sintable[offset%SIN_COUNT] * ((float) i / BLUR_REPEAT)) + i;
		y1 = 136 - (64/2)  + (costable[offset%SIN_COUNT] * ((float) i / BLUR_REPEAT)) - i;
		y2 = 136 + (64/2)  + (costable[offset%SIN_COUNT] * ((float) i / BLUR_REPEAT)) + i;

		c_vertices[0].u = 0; c_vertices[0].v = 0;
		c_vertices[0].x = x1; c_vertices[0].y = y1; c_vertices[0].z = 0;
		c_vertices[0].color = color;

		c_vertices[1].u = 128; c_vertices[1].v = 32;
		c_vertices[1].x = x2; c_vertices[1].y = y2; c_vertices[1].z = 0;
		c_vertices[1].color = color;

		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,c_vertices);
	}

	offset++;

	sceGuDisable(GU_ALPHA_TEST);
	sceGuDisable(GU_BLEND);
	sceGuDisable(GU_TEXTURE_2D);

	sceGuDepthFunc(GU_GEQUAL);
}
