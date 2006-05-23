/*
	PSPTris - The game - Intro sequence
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
#include <psprtc.h>

#include "PSPTris.h"
#include "PSPTris_intro.h"
#include "PSPTris_audio.h"
#include "jsaTextureCache.h"
#include "jsaParticle.h"
#include "jsaRand.h"

#define		TRUE			1
#define		FALSE			0

#define 	LETTER_COUNT	7

#define		COLOR_LIGHT		0xFFFFFFFF

#define		COLOR_BLUE		0x80FF0000
#define		COLOR_GREEN		0x8000FF00
#define		COLOR_ORANGE	0x800190FF
#define		COLOR_PINK		0x80E349FF
#define		COLOR_YELLOW	0x8000FFFF

static unsigned int __attribute__((aligned(16))) gu_list[65536];

static const struct CVertex __attribute__((aligned(16))) pink_vertices[] =
{
	{COLOR_PINK, -0.2,  0.2,  0.1},
	{COLOR_PINK,  0.0,  0.2,  0.1},
	{COLOR_PINK,  0.0,  0.0,  0.1},
	{COLOR_PINK,  0.2,  0.0,  0.1},
	{COLOR_PINK,  0.2, -0.4,  0.1},
	{COLOR_PINK,  0.0, -0.4,  0.1},
	{COLOR_PINK,  0.0, -0.2,  0.1},
	{COLOR_PINK, -0.2, -0.2,  0.1},
	{COLOR_PINK, -0.2,  0.2, -0.1},
	{COLOR_PINK,  0.0,  0.2, -0.1},
	{COLOR_PINK,  0.0,  0.0, -0.1},
	{COLOR_PINK,  0.2,  0.0, -0.1},
	{COLOR_PINK,  0.2, -0.4, -0.1},
	{COLOR_PINK,  0.0, -0.4, -0.1},
	{COLOR_PINK,  0.0, -0.2, -0.1},
	{COLOR_PINK, -0.2, -0.2, -0.1},
};

static const struct Face __attribute__((aligned(16))) pink_indices[] =
{
	{0, 1, 7}, {7, 1, 6},
	{2, 3, 5}, {5, 3, 4},
	{0, 8, 9}, {0, 9, 1},
	{1, 9, 2}, {2, 9, 10},
	{10, 11, 2}, {2, 11, 3},
	{3, 11, 4}, {4, 11, 12},
	{4, 12, 5}, {5, 12, 13},
	{14, 6, 13}, {13, 6, 5},
	{7, 6, 15}, {15, 6, 14},
	{8, 0, 15}, {15, 0, 7},
	{9, 8, 14}, {14, 8, 15},
	{11, 10, 12}, {12, 10, 13},
};

static int pink_face_count = sizeof(pink_indices)/sizeof(unsigned short);


static const struct CVertex __attribute__((aligned(16))) yellow_vertices[] =
{
	{COLOR_YELLOW, -0.2,  0.2,  0.1},
	{COLOR_YELLOW,  0.4,  0.2,  0.1},
	{COLOR_YELLOW,  0.4,  0.0,  0.1},
	{COLOR_YELLOW,  0.2,  0.0,  0.1},
	{COLOR_YELLOW,  0.2, -0.2,  0.1},
	{COLOR_YELLOW,  0.0, -0.2,  0.1},
	{COLOR_YELLOW,  0.0,  0.0,  0.1},
	{COLOR_YELLOW, -0.2,  0.0,  0.1},
	{COLOR_YELLOW, -0.2,  0.2, -0.1},
	{COLOR_YELLOW,  0.4,  0.2, -0.1},
	{COLOR_YELLOW,  0.4,  0.0, -0.1},
	{COLOR_YELLOW,  0.2,  0.0, -0.1},
	{COLOR_YELLOW,  0.2, -0.2, -0.1},
	{COLOR_YELLOW,  0.0, -0.2, -0.1},
	{COLOR_YELLOW,  0.0,  0.0, -0.1},
	{COLOR_YELLOW, -0.2,  0.0, -0.1},
};

static const struct Face __attribute__((aligned(16))) yellow_indices[] =
{
	{0, 1, 7}, {7, 1, 2},
	{6, 3, 5}, {5, 3, 4},
	{8, 9, 0}, {0, 9, 1},
	{2, 1, 9}, {2, 9, 11},
	{8, 0, 15}, {15, 0, 8},
	{7, 6, 15}, {15, 6, 14},
	{3, 2, 11}, {11, 2, 10},
	{5, 4, 13}, {13, 4, 12},
	{9, 8, 10}, {10, 8, 15},
	{11, 14, 12}, {12, 14, 13},
	{3, 11, 4}, {4, 11, 12},
	{14, 6, 13}, {13, 6, 5}
};

static int yellow_face_count = sizeof(yellow_indices)/sizeof(unsigned short);

static const struct CVertex __attribute__((aligned(16))) orange_vertices[] =
{
	{COLOR_ORANGE, -0.2,  0.2,  0.1},
	{COLOR_ORANGE,  0.4,  0.2,  0.1},
	{COLOR_ORANGE,  0.4,  0.0,  0.1},
	{COLOR_ORANGE,  0.0,  0.0,  0.1},
	{COLOR_ORANGE,  0.0, -0.2,  0.1},
	{COLOR_ORANGE, -0.2, -0.2,  0.1},
	{COLOR_ORANGE, -0.2,  0.0,  0.1},
	{COLOR_ORANGE, -0.2,  0.2, -0.1},
	{COLOR_ORANGE,  0.4,  0.2, -0.1},
	{COLOR_ORANGE,  0.4,  0.0, -0.1},
	{COLOR_ORANGE,  0.0,  0.0, -0.1},
	{COLOR_ORANGE,  0.0, -0.2, -0.1},
	{COLOR_ORANGE, -0.2, -0.2, -0.1},
	{COLOR_ORANGE, -0.2,  0.0, -0.1},
};

static const struct Face __attribute__((aligned(16))) orange_indices[] =
{
	{0, 1, 6}, {6, 1, 2},
	{6, 3, 5}, {5, 3, 4},
	{0, 7, 8}, {0, 8, 1},
	{1, 8, 2}, {2, 8, 9},
	{3, 2, 10}, {10, 2, 9},
	{3, 10, 4}, {4, 10, 11},
	{5, 4, 12}, {12, 4, 11},
	{7, 0, 12}, {12, 0, 5},
	{8, 7, 9}, {9, 7, 13},
	{10, 13, 11}, {11, 13, 12}
};

static int orange_face_count = sizeof(orange_indices)/sizeof(unsigned short);

static const struct CVertex __attribute__((aligned(16))) blue_vertices[] =
{
	{COLOR_BLUE, -0.1,  0.4,  0.1},
	{COLOR_BLUE,  0.1,  0.4,  0.1},
	{COLOR_BLUE,  0.1, -0.4,  0.1},
	{COLOR_BLUE, -0.1, -0.4,  0.1},
	{COLOR_BLUE, -0.1,  0.4, -0.1},
	{COLOR_BLUE,  0.1,  0.4, -0.1},
	{COLOR_BLUE,  0.1, -0.4, -0.1},
	{COLOR_BLUE, -0.1, -0.4, -0.1},
};

static const struct CVertex __attribute__((aligned(16))) green_vertices[] =
{
	{COLOR_GREEN, -0.3,  0.3,  0.1},
	{COLOR_GREEN,  0.3,  0.3,  0.1},
	{COLOR_GREEN,  0.3, -0.3,  0.1},
	{COLOR_GREEN, -0.3, -0.3,  0.1},
	{COLOR_GREEN, -0.3,  0.3, -0.1},
	{COLOR_GREEN,  0.3,  0.3, -0.1},
	{COLOR_GREEN,  0.3, -0.3, -0.1},
	{COLOR_GREEN, -0.3, -0.3, -0.1},
};

static const struct Face __attribute__((aligned(16))) blue_indices[] =
{
	{0, 1, 3}, {1, 2, 3},
	{1, 5, 2}, {5, 6, 2},
	{5, 4, 7}, {5, 7, 6},
	{4, 0, 7}, {0, 3, 7},
	{0, 4, 1}, {4, 5, 1},
	{3, 2, 6}, {6, 7, 3},
};

static int blue_face_count = sizeof(blue_indices)/sizeof(unsigned short);

static const int		 bricks_facecount[] = {blue_face_count, blue_face_count, yellow_face_count, orange_face_count, pink_face_count};
static struct CVertex	*bricks_vertices[] = {(struct CVertex *)&blue_vertices, (struct CVertex *)&green_vertices, (struct CVertex *)&yellow_vertices, (struct CVertex *)&orange_vertices, (struct CVertex *)&pink_vertices};
static struct Face		*bricks_faces[] =	{(struct Face *)&blue_indices, (struct Face *)&blue_indices, (struct Face *)&yellow_indices,(struct Face *)&orange_indices, (struct Face *)&pink_indices};

#define	BRICK_COUNT		100

typedef struct
	{
	int				facecount;
	struct CVertex	*vertices;
	struct Face		*faces;
	int				delay;
	int				rotation;
	float			x_offset;
	float			y_offset;
	float			z_offset;
	} brick;

static brick	bricks[BRICK_COUNT];

/* original */
static struct NCVertex __attribute__((aligned(16))) vertices[] =
{
	{0.0f, 0.0f, COLOR_LIGHT, -1.5,  1.0,  0.0},
	{1.0f, 0.0f, COLOR_LIGHT,  1.5,  1.0,  0.0},
	{1.0f, 1.0f, COLOR_LIGHT,  1.5, -1.0,  0.0},
	{0.0f, 1.0f, COLOR_LIGHT, -1.5, -1.0,  0.0},
};

/* 1 system for each letter */
static struct NCVertex *letter_vertices[LETTER_COUNT];

static struct Face __attribute__((aligned(16))) indices[] =
{
	{0, 1, 3}, {1, 2, 3},
};

static int face_count = sizeof(indices)/sizeof(unsigned short);

#define	START_SIZE	0.2f
#define GELLY		0.3f

/* Original system */
static particle_str __attribute__((aligned(16))) cube_vertex[] =
	{
	{1.0f, {-START_SIZE-GELLY,  START_SIZE,			0}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, FALSE},	// 0
	{1.0f, { START_SIZE+GELLY,  START_SIZE+GELLY,	0}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, FALSE},	// 1
	{1.0f, { START_SIZE,     	-START_SIZE,		0}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, FALSE},	// 2
	{1.0f, {-START_SIZE,     	-START_SIZE-GELLY,	0}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, FALSE},	// 3
	};

/* 1 system for each letter */
static particle_str *letter_vertex[LETTER_COUNT];

#define	KSPRING		0.075f
#define	KDAMP		0.01f

static particlespring_str __attribute__((aligned(16))) cube_spring[] =
	{
	{0, 1, KSPRING, KDAMP, 2.000000f},
	{0, 2, KSPRING, KDAMP, 2.828427f},
	{0, 3, KSPRING, KDAMP, 2.000000f},
	{1, 2, KSPRING, KDAMP, 2.000000f},
	{1, 3, KSPRING, KDAMP, 2.828427f},
	{2, 3, KSPRING, KDAMP, 2.000000f},
	};

/* Spring system variables */
static int					nparticles	= sizeof(cube_vertex)/sizeof(particle_str);
static int					nsprings	= sizeof(cube_spring)/sizeof(particlespring_str);
static particlephys_str		physical;

/* Texture handler */
static jsaTextureCache				*tcache;

enum TEX_NAMES
	{
	TEX_P,
	TEX_S,
	TEX_T,
	TEX_R,
	TEX_I,
	} ;

static jsaTextureFile __attribute__((aligned(16))) texture_list[] =
	{
	{TEX_P, 	GU_PSM_8888, 64, 64, true, FT_PNG, "p.png"},
	{TEX_S, 	GU_PSM_8888, 64, 64, true, FT_PNG, "s.png"},
	{TEX_T, 	GU_PSM_8888, 64, 64, true, FT_PNG, "t.png"},
	{TEX_R, 	GU_PSM_8888, 64, 64, true, FT_PNG, "r.png"},
	{TEX_I, 	GU_PSM_8888, 64, 64, true, FT_PNG, "i.png"},
	};

#define	TEXTURE_COUNT		(sizeof(texture_list) / sizeof(jsaTextureFile))

void PSPTris_intro_setup_particles(void)
{
	physical.gravitational = 0;
	physical.viscousdrag = 0.05;

	/* Create 1 system for each letter */
	for (int i = 0 ; i < LETTER_COUNT  ; i++)
		{
		letter_vertex[i] = (particle_str *)memalign(16, sizeof(cube_vertex));
		memcpy(letter_vertex[i], cube_vertex, sizeof(cube_vertex));

		letter_vertices[i] = (struct NCVertex *)memalign(16, sizeof(vertices));
		memcpy(letter_vertices[i], vertices, sizeof(vertices));
		}
}

static void PSPTris_get_new_brick(int index)
{
	int new_brick = jsaRand() % 5;

	bricks[index].facecount = bricks_facecount[new_brick];
	bricks[index].vertices 	= bricks_vertices[new_brick];
	bricks[index].faces 	= bricks_faces[new_brick];
	bricks[index].delay 	= jsaRandRange(60,480+60);
	bricks[index].rotation 	= 0;
	bricks[index].x_offset 	= (float)((float)jsaRandRange(0,480) / 48) - 5.0f;
	bricks[index].z_offset 	= (float)-((float)jsaRandRange(0,480) / 96) - 1.0f;
	bricks[index].y_offset 	= 4.0f;
}

void PSPTris_setup_bricks()
{
	for (int i = 0 ; i < BRICK_COUNT ; i++)
		{
		PSPTris_get_new_brick(i);
		}
}

void PSPTris_intro_init(char *cwd)
{
	char path[1024];

	/* Start playing intro module */
	sprintf(path, "%s/Music/intro.mod", cwd);
#if !defined(DYNAMIC_BUILD)
	PSPTris_audio_play_module(path);
#endif /* !defined(DYNAMIC_BUILD) */

	/* Create a texture cache object */
	tcache = new jsaTextureCache();

	sprintf(path, "%s/Textures", cwd);
	tcache->jsaTCacheLoadTextureSet(path, texture_list, TEXTURE_COUNT);

	PSPTris_intro_setup_particles();
	PSPTris_setup_bricks();
	sceKernelDcacheWritebackAll();
}


void PSPTris_intro_destroy(void)
{
#if !defined(DYNAMIC_BUILD)
	PSPTris_audio_stop_module();
#endif /* !defined(DYNAMIC_BUILD) */

	/* dealocate memory */
	for (int i = 0 ; i < LETTER_COUNT  ; i++)
		{
		free(letter_vertex[i]);
		free(letter_vertices[i]);
		}
	/* Delete texture cache object */
	delete tcache;
}

void PSPTris_intro_spring_system(int system)
{
	UpdateParticles(letter_vertex[system], nparticles, physical, cube_spring, nsprings, 0.1f);

	/* Copy spring array into vertex array */
	for (int i = 0 ; i < nparticles ; i++)
		{
		letter_vertices[system][i].x = letter_vertex[system][i].p.x;
		letter_vertices[system][i].y = letter_vertex[system][i].p.y;
		letter_vertices[system][i].z = letter_vertex[system][i].p.z;
		}
	sceKernelDcacheWritebackAll();
}

void PSPTris_intro_letter(int texture, float x, int system)
{
	(void)tcache->jsaTCacheSetTexture(texture);

	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();
	ScePspFVector3 pos = { x, 0.0f, -2.5f };
	sceGumTranslate(&pos);
	ScePspFVector3 scale = { 0.75f, 0.75f, 0.0f };
	sceGumScale(&scale);

	sceGuTexFunc(GU_TFX_BLEND,GU_TCC_RGBA);
	sceGumDrawArray(GU_TRIANGLES, GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_INDEX_16BIT | GU_TRANSFORM_3D, face_count, indices, letter_vertices[system]);
	sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);
	sceGumDrawArray(GU_TRIANGLES, GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_INDEX_16BIT | GU_TRANSFORM_3D, face_count, indices, letter_vertices[system]);
}

void PSPTris_intro_logo(void)
{
	int				textures[LETTER_COUNT]	= {TEX_P, TEX_S, TEX_P, TEX_T, TEX_R, TEX_I, TEX_S};
	static int		start[LETTER_COUNT] 	= {    0,    20,    15,    10,     5,    30,    25};
	float			x = -2.5f;

	sceGuEnable(GU_TEXTURE_2D);
	sceGuDisable(GU_DEPTH_TEST);

	for (int i = 0 ; i < LETTER_COUNT ; i++)
		{
		if (start[i] == -120)
			{
			PSPTris_intro_spring_system(i);
			PSPTris_intro_letter(textures[i], x, i);
			}
		else
			{
			start[i]--;
			}
		x += 0.8f;
		}

	sceGuEnable(GU_DEPTH_TEST);
	sceGuDepthFunc(GU_GEQUAL);
	sceGuDisable(GU_TEXTURE_2D);
}

void PSPTris_intro_render_brick(bool front)
{
	sceGuEnable(GU_DEPTH_TEST);
	sceGuDisable(GU_TEXTURE_2D);

	for (int i = 0 ; i < BRICK_COUNT ; i++)
		{
		if (bricks[i].delay == 0)
			{
			sceGumMatrixMode(GU_MODEL);
			sceGumLoadIdentity();
			ScePspFVector3 pos1 = { bricks[i].x_offset, bricks[i].y_offset, bricks[i].z_offset };
			sceGumTranslate(&pos1);
			ScePspFVector3 scale = { 1, 1, 1 };
			sceGumScale(&scale);
			ScePspFVector3 rot = {bricks[i].rotation * 0.45 * (GU_PI/180.0f), bricks[i].rotation * 0.79f * (GU_PI/180.0f), bricks[i].rotation * 1.32f * (GU_PI/180.0f) };
			sceGumRotateXYZ(&rot);
			if (front && (bricks[i].z_offset > -3.0))
				{
				sceGumDrawArray(GU_TRIANGLES, GU_COLOR_8888 | GU_VERTEX_32BITF | GU_INDEX_16BIT | GU_TRANSFORM_3D, bricks[i].facecount, bricks[i].faces, bricks[i].vertices);
				}
			if (!front && (bricks[i].z_offset < -3.0))
				{
				sceGumDrawArray(GU_TRIANGLES, GU_COLOR_8888 | GU_VERTEX_32BITF | GU_INDEX_16BIT | GU_TRANSFORM_3D, bricks[i].facecount, bricks[i].faces, bricks[i].vertices);
				}
			bricks[i].rotation++;
			bricks[i].y_offset -= 0.015f;
			if (bricks[i].y_offset < -4.5)
				{
				PSPTris_get_new_brick(i);
				}
			}
		else
			{
			bricks[i].delay--;
			}
		}

	sceGuEnable(GU_TEXTURE_2D);
	sceGuDisable(GU_DEPTH_TEST);
}

u32 PSPTris_get_bg_color(void)
{
static	u32 bg_color = 0xFF000000;
u32			temp;

	temp = bg_color & 0xFF;
	if (temp < 0xB2)
		{
		bg_color += 0x00000001;
		}
	temp = (bg_color >> 8) & 0xFF;
	if (temp < 0x09)
		{
		bg_color += 0x00000100;
		}
	temp = (bg_color >> 16) & 0xFF;
	if (temp < 0x10)
		{
		bg_color += 0x00010000;
		}
	return bg_color;
}

void PSPTris_intro()
{
	/* Play intro sequence */
 	sceGuStart(GU_DIRECT,::gu_list);

	sceGuClearColor(PSPTris_get_bg_color());
	sceGuClearDepth(0);
	sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);

	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadIdentity();
	sceGumPerspective(75.0f,16.0f/9.0f,0.5f,1000.0f);

	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();

	sceGuAmbient(0xFFFFFFFF);
	sceGuColor(0xFFFFFFFF);
	sceGuTexEnvColor(0xFF000000);

	sceGuDisable(GU_CULL_FACE);
	sceGuAlphaFunc(GU_GREATER,0x0,0xff);
	sceGuEnable(GU_ALPHA_TEST);
	sceGuTexFilter(GU_LINEAR, GU_LINEAR);
	sceGuTexWrap(GU_CLAMP, GU_CLAMP);
	sceGuBlendFunc( GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0 );
	sceGuEnable( GU_BLEND );

	PSPTris_intro_render_brick(false);
	PSPTris_intro_logo();
	PSPTris_intro_render_brick(true);

	sceGuDisable(GU_BLEND);
	sceGuDisable(GU_ALPHA_TEST);
	sceGuEnable(GU_CULL_FACE);

	sceGuFinish();
	sceGuSync(0,0);

	sceGuSwapBuffers();
	sceDisplayWaitVblankStart();
}
