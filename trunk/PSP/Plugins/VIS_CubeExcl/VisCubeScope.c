/* 
	PSPRadio / Music streaming client for the PSP. (Initial Release: Sept. 2005)
	Copyright (C) 2005  Rafael Cabezas a.k.a. Raf
	
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
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspge.h>
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <malloc.h>
#include <stdarg.h>
#include "../VIS_Plugin.h"
#include <pspmath.h>
#include <pspgu.h>
#include <pspgum.h>

PSP_MODULE_INFO("VIS_CUBESCOPE", 0, 1, 1);
PSP_HEAP_SIZE_KB(0);

#define GETGURELADDR(x) ((void*)((unsigned int)x&~0x44000000))

/* Prototypes */
void config_update();
void init();
void term();
void render_pcm(u32 *vram, u16 *pcm);
void SetupProjection();
void DrawCube();

u32 Texture[64*64];
unsigned int __attribute__((aligned(16))) list[262144];

typedef struct {
	float u, v;
	unsigned int color;
	float x, y, z;
} Vertex;

typedef struct _texture 
{
  int width, height;
} texture;

texture tex = {64, 64};

int render_on = 0;

#define SCR_WIDTH 480
#define SCR_HEIGHT 272

#define WHITE GU_COLOR( 1.0f, 1.0f, 1.0f, 1.0f )

Vertex __attribute__((aligned(16))) cube[3*12] = 
{
	{ 0, 1, WHITE,-1.0f, 1.0f,-1.0f },	// Top
	{ 1, 1, WHITE, 1.0f, 1.0f,-1.0f },
	{ 0, 0, WHITE,-1.0f, 1.0f, 1.0f },
 
	{ 0, 0, WHITE,-1.0f, 1.0f, 1.0f },
	{ 1, 1, WHITE, 1.0f, 1.0f,-1.0f },
	{ 1, 0, WHITE, 1.0f, 1.0f, 1.0f },
 
	{ 0, 1, WHITE,-1.0f,-1.0f, 1.0f },	// Bottom
	{ 1, 1, WHITE, 1.0f,-1.0f, 1.0f },
	{ 0, 0, WHITE,-1.0f,-1.0f,-1.0f },
 
	{ 0, 0, WHITE,-1.0f,-1.0f,-1.0f },
	{ 1, 1, WHITE, 1.0f,-1.0f, 1.0f },
	{ 1, 0, WHITE, 1.0f,-1.0f,-1.0f },
 
	{ 0, 1, WHITE,-1.0f, 1.0f, 1.0f },	// Front
	{ 1, 1, WHITE, 1.0f, 1.0f, 1.0f },
	{ 0, 0, WHITE,-1.0f,-1.0f, 1.0f },
 
	{ 0, 0, WHITE,-1.0f,-1.0f, 1.0f },
	{ 1, 1, WHITE, 1.0f, 1.0f, 1.0f },
	{ 1, 0, WHITE, 1.0f,-1.0f, 1.0f },
 
	{ 0, 1, WHITE,-1.0f,-1.0f,-1.0f },	// Back
	{ 1, 1, WHITE, 1.0f,-1.0f,-1.0f },
	{ 0, 0, WHITE,-1.0f, 1.0f,-1.0f },
 
	{ 0, 0, WHITE,-1.0f, 1.0f,-1.0f },
	{ 1, 1, WHITE, 1.0f,-1.0f,-1.0f },
	{ 1, 0, WHITE, 1.0f, 1.0f,-1.0f },
 
	{ 0, 1, WHITE,-1.0f, 1.0f,-1.0f },	// Left
	{ 1, 1, WHITE,-1.0f, 1.0f, 1.0f },
	{ 0, 0, WHITE,-1.0f,-1.0f,-1.0f },
 
	{ 0, 0, WHITE,-1.0f,-1.0f,-1.0f },
	{ 1, 1, WHITE,-1.0f, 1.0f, 1.0f },
	{ 1, 0, WHITE,-1.0f,-1.0f, 1.0f },
 
	{ 0, 1, WHITE, 1.0f, 1.0f, 1.0f },	// Right
	{ 1, 1, WHITE, 1.0f, 1.0f,-1.0f },
	{ 0, 0, WHITE, 1.0f,-1.0f, 1.0f },
 
	{ 0, 0, WHITE, 1.0f,-1.0f, 1.0f },
	{ 1, 1, WHITE, 1.0f, 1.0f,-1.0f },
	{ 1, 0, WHITE, 1.0f,-1.0f,-1.0f }
 
};

/** START of Plugin definitions setup */
VisPlugin vtable = 
{
	/* Populated by Plugin */
	PLUGIN_VIS_VERSION,		 		/* Populate with PLUGIN_VIS_VERSION */
	"Cube-Scope Excl Visualizer Plugin",		/* Plugin description */
	"By Raf",	 					/* Plugin about info */
	VIS_TYPE_EXCL,					
	init,				 			/* Called when the plugin is enabled */
	term,				 			/* Called when the plugin is disabled */
	NULL,						 	/* not used atm *//* Called when playback starts */
	NULL,
	NULL,						 	/* not used atm *//* Called when playback stops */
	/* Render the PCM (2ch/44KHz) data, pcm_data has 2 channels interleaved */
	render_pcm, 
	/* Render the freq data, don't do anything time consuming in here */
	NULL,//spect_render_freq,
	config_update,	 		/* Called by PSPRadio when config changes */

	/* Set by PSPRadio */
	NULL,							/* Filled in by PSPRadio */
	0,0,
	NULL,
};
/** END of Plugin definitions setup */

/** Single Plugin Export */
VisPlugin *get_vplugin_info()
{
	return &vtable;
}

void init()
{
	vfpu_srand(0);
}

void SetupProjection( void )
{
	// setup matrices for the triangle
	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadIdentity();
	sceGumPerspective( 75.0f, 16.0f/9.0f, 0.5f, 1000.0f);
 
	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();
 
	sceGuClearColor( GU_COLOR( 0.0f, 0.0f, 0.0f, 1.0f ) ); 
	sceGuClearDepth(0);	
}

void term()
{
	sceGuTerm();
}

/* Called from PSPRadio when the config pointer has been updated */
void config_update()
{
	int temp_render_on = (vtable.config)?(vtable.config->fullscreen):0;

	if (temp_render_on == 1 && render_on == 0)
	{
		sceGuInit();

		sceGuStart(GU_DIRECT, list);
	
		// Set Buffers
		sceGuDrawBuffer( GU_PSM_8888, 0, 512 );
		sceGuDispBuffer( 480, 272, (void*)0x88000, 512);
		sceGuDepthBuffer( (void*)0x110000, 512);
	
		sceGuOffset( 2048 - (SCR_WIDTH/2), 2048 - (SCR_HEIGHT/2));
		sceGuViewport( 2048, 2048, SCR_WIDTH, SCR_HEIGHT);
		sceGuDepthRange( 65535, 0);
		
		// Set Render States
		sceGuScissor(0,0, 480, 272);
		sceGuEnable( GU_SCISSOR_TEST );
		sceGuDepthFunc( GU_GEQUAL );
		sceGuEnable( GU_DEPTH_TEST );
		sceGuFrontFace( GU_CW );
		sceGuShadeModel( GU_SMOOTH );
		sceGuEnable( GU_CULL_FACE );
		sceGuEnable( GU_CLIP_PLANES );
	
		sceGuEnable(GU_TEXTURE_2D);
		sceGuTexMode( GU_PSM_8888, 0, 0, 0);
		sceGuTexFunc( GU_TFX_DECAL, GU_TCC_RGB );
		sceGuTexFilter( GU_LINEAR, GU_LINEAR );
		sceGuTexScale( 1.0f, 1.0f );
		sceGuTexOffset( 0.0f, 0.0f );
	
		sceGuFinish();
		sceGuSync(0,0);
	
		sceDisplayWaitVblankStart();
		sceGuDisplay(GU_TRUE);
	
		SetupProjection();

	}
	else
	{	
		sceGuTerm();
	}

	render_on = temp_render_on;
}	

/* Some basic drawing routines */
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#define RGB(r,g,b) ((((b)&0xFF)<<16) | (((g)&0xFF)<<8) | (((r)&0xFF)))
void Plot(u32* vram, int x, int y, int color)
{
	u32 *pixel = vram + 64*y + x;
	*pixel = /* *pixel |*/ color;
}
void VertLine(u32* vram, int x, int y1, int y2, int color)
{
	int y;
	if (y1 < y2)
	{
		for (y = y1<0?0:y1; y <= y2; y++)
		{
			Plot(vram, x, y, color);
		}
	}
	else if (y2 < y1)
	{
		for (y = y2<0?0:y2; y <= y1; y++)
		{
			Plot(vram, x, y, color);
		}
	}
	else 
	{
		Plot(vram, x, y1, color);
	}
	
}

void scope(u16 *pcm_data)
{
	int x;
	int yL, yR;
	int old_yL = 32;
	int old_yR = 32;
	for (x = 0; x < 64; x++)
	{
		yL = 32 + (pcm_data[x*2]   >> 10); // L component
		yR = 32 - (pcm_data[x*2+1] >> 10); // R component
		VertLine(Texture, x, old_yL, yL, 0x00FFFF);
		VertLine(Texture, x, old_yR, yR, 0x00FF00);
		old_yL = yL;
		old_yR = yR;
	}
}

/* This is called from PSPRadio */
/* (actual visualizer routine) */
void render_pcm(u32 *vram_frame, u16 *pcm_data)
{
	int i;


	if (render_on == 0)
		return;

	for (i = 0 ; i < 64*64; i++)
		Texture[i] = 0xFFFFAA88;

	//memset(Texture, 0xFF, sizeof(Texture));

 	sceKernelDcacheWritebackAll(); 
	scope(pcm_data);
 	sceKernelDcacheWritebackAll(); 

	DrawCube();

	sceDisplayWaitVblankStart();
	sceGuSwapBuffers();
}

/* based on NeHe's port to pspgu tutorial from pspprogramming.com */
void DrawCube( void )
{
	static float rquad= 0.0f; /* Rotation of triangle and quad */

	sceGuStart( GU_DIRECT, list );
		
	/* Clear screen buffers */
	sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);
	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();		// Reset the Matrix
	{	// Move 1.5 units left and 4 units back
		ScePspFVector3 move = { 0.0f, 0.0f, -4.0f };
		ScePspFVector3 rot  = { rquad, rquad, rquad };		// Define rotation structure (NEW)
 
		sceGumTranslate( &move );				// Move the Object
		sceGumRotateXYZ( &rot );				// Rotate the Object (NEW)
	}
 
	sceGuTexImage( 0, tex.width, tex.height, tex.width, Texture ); 
 
	sceGumDrawArray( GU_TRIANGLES, GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D,
					3*12, 0, cube );	// Draw the Cube (NEW)

	/* Rotate */
	rquad-= 0.02f; //(1.0f * dt);

	sceGuFinish();
	sceGuSync(0,0);	

}

/** START Plugin Boilerplate -- shouldn't need to change **/
PSP_NO_CREATE_MAIN_THREAD();
int module_start(SceSize argc, void* argp)
{
	return 0;
}
int module_stop(int args, void *argp)
{
	return 0;
}
/** END Plugin Boilerplate -- shouldn't need to change **/
