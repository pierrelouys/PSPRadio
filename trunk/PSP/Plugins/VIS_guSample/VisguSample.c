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
#include "VIS_Plugin.h"

PSP_MODULE_INFO("VIS_GUSAMPLE", 0, 1, 1);
PSP_HEAP_SIZE_KB(0);

/* Prototypes */
void scope_config_update();
void scope_init();
void scope_term();
void scope_render_pcm(u32* vram_frame, int16 *pcm_data);
#define GETGURELADDR(x) ((void*)((unsigned int)x&~0x44000000))

/* GU Display list */
unsigned int __attribute__((aligned(16))) list[262144];

/** START of Plugin definitions setup */
VisPlugin vtable = 
{
	/* Populated by Plugin */
	PLUGIN_VIS_VERSION,		 		/* Populate with PLUGIN_VIS_VERSION */
	"Sample GU Visualizer Plugin",	/* Plugin description */
	"By Raf",	 					/* Plugin about info */
	scope_init,			 			/* Called when the plugin is enabled */
	scope_term,			 			/* Called when the plugin is disabled */
	NULL,						 	/* not used atm *//* Called when playback starts */
	NULL,						 	/* not used atm *//* Called when playback stops */
	/* Render the PCM (2ch/44KHz) data, pcm_data has 2 channels interleaved */
	scope_render_pcm, 
	/* Render the freq data, don't do anything time consuming in here */
	NULL,
	scope_config_update,	 		/* Called by PSPRadio when config changes */

	/* Set by PSPRadio */
	NULL,							/* Filled in by PSPRadio */
};
/** END of Plugin definitions setup */

/** Single Plugin Export */
VisPlugin *get_vplugin_info()
{
	return &vtable;
}

/* Called from PSPRadio on initialization */
#define SCR_WIDTH 480
#define SCR_HEIGHT 272
void scope_init()
{
	scope_config_update();
	sceGuInit();
	sceGuStart(GU_DIRECT, list);

	/* Set Buffers */
	sceGuDrawBuffer( GU_PSM_8888, 0, 512 );
	sceGuDispBuffer( 480, 272, (void*)0x88000, 512);
	sceGuDepthBuffer( (void*)0x110000, 512);

	sceGuOffset( 2048 - (SCR_WIDTH/2), 2048 - (SCR_HEIGHT/2));
	sceGuViewport( 2048, 2048, SCR_WIDTH, SCR_HEIGHT);
	sceGuDepthRange( 65535, 0);
	
	sceGuScissor(0,0, 480, 272);
	sceGuEnable( GU_SCISSOR_TEST );

	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);
}

void scope_term()
{
	sceGuTerm();
}

/* Called from PSPRadio when the config pointer has been updated */
int y_mid = 127, pcm_shdiv = 15;
void scope_config_update()
{
	int sh = 0, amp = 0;
	int mid_a = 0;

	if(vtable.config)
	{
		y_mid = (vtable.config->y2 + vtable.config->y1) / 2;

		mid_a = (vtable.config->y2 - vtable.config->y1) / 2;
		pcm_shdiv = 15; 
		
		// We convert the fixed-point integer into an integer by doing binary right shifts.
		// So pcm_shdiv of 8 gives us the whole integer part, 9 is whole / 2, 10 is /4, etc.
		// at 15, we are left with a max of 1, so this is the minimum. 
		// shdiv then is 8 <= shdiv <= 15
		
		for (sh = 15, amp = 1; sh >= 8; sh--, amp *= 2)
		{
			if (mid_a >= amp)
				pcm_shdiv = sh;
		}
	}
}	

#include <pspgu.h>

typedef struct LineVertex
{
	float x,y,z;
}LineVertex;

/* This is called from PSPRadio */
/* (actual visualizer routine) */
void scope_render_pcm(u32* drawbuffer_absolute_addr, int16 *pcm_data)
{
	float		xpos = vtable.config->x1;
	int			width  = vtable.config->x2 - vtable.config->x1;
	int i;


	/* define were we want the GU to draw to */
	sceGuDrawBuffer( GU_PSM_8888, 
					 GETGURELADDR(drawbuffer_absolute_addr), 
					 vtable.config->sc_pitch );

	sceGuStart( GU_DIRECT, list );

	/*sceGuScissor(vtable.config->x1,vtable.config->y1, 
				width, 
				vtable.config->y2 - vtable.config->y1);
	sceGuEnable( GU_SCISSOR_TEST );*/

	sceGuEnable(GU_LINE_SMOOTH);

	LineVertex *l_vertices = (LineVertex *)sceGuGetMemory((width + 1 )* sizeof(LineVertex));

	/* Channel 0 */
	for (i = 0; i < width; i++)
	{
		l_vertices[i].x = xpos++;
		l_vertices[i].y = y_mid + (pcm_data[i*5] >> pcm_shdiv);
		l_vertices[i].z = 0;
	}
	sceGuColor(0xFF00FF00);
	sceGuDrawArray(GU_LINE_STRIP, GU_VERTEX_32BITF | GU_TRANSFORM_2D, width, 0, l_vertices);

	/* Channel 1 */
	xpos = vtable.config->x1;
	for (i = 0; i < width; i++)
	{
		l_vertices[i].x = xpos++;
		l_vertices[i].y = y_mid - (pcm_data[i*5+1] >> pcm_shdiv);
		l_vertices[i].z = 0;
	}
	sceGuColor(0xFFFFFF00);
	sceGuDrawArray(GU_LINE_STRIP, GU_VERTEX_32BITF | GU_TRANSFORM_2D, width, 0, l_vertices);

	sceGuDisable(GU_LINE_SMOOTH);

	/* we're done rendering the frame */
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
