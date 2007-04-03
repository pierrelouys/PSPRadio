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


#define RGB(r,g,b) (((b&0xFF)<<16) | ((g&0xFF)<<8) | ((r&0xFF)))

/** PRX **/
PSP_MODULE_INFO("VIS_SCOPE", 0, 1, 1);
PSP_HEAP_SIZE_KB(0);
PSP_NO_CREATE_MAIN_THREAD();

int module_start(SceSize argc, void* argp)
{
	return 0;
}

int module_stop(int args, void *argp)
{
	return 0;
}
/** PRX **/

short *s_pcmbuffer = NULL;

/* Plugin setup */
void scope_init();
void scope_cleanup();
void scope_start();
void scope_stop();
void scope_render_pcm(u32* vram_frame, int16 *pcm_data);
void scope_config_update();

/** START of Plugin definitions setup */
VisPlugin vtable = 
{
	/* Populated by Plugin */
	PLUGIN_VIS_VERSION,		 		/* Populate with PLUGIN_VIS_VERSION */
	"Scope Visualizer Plugin",		/* Plugin description */
	"By Raf",	 					/* Plugin about info */
	0,								/* Set to 1 if plugin needs GU; 0 if not. */
	scope_init,			 			/* Called when the plugin is enabled */
	scope_cleanup,		 			/* Called when the plugin is disabled */
	scope_start,				 	/* not used atm *//* Called when playback starts */
	scope_stop,					 	/* not used atm *//* Called when playback stops */
	/* Render the PCM (2ch/44KHz) data, pcm_data has 2 channels interleaved */
	scope_render_pcm, 
	/* Render the freq data, don't do anything time consuming in here */
	NULL,
	scope_config_update,	 		/* Called by PSPRadio when config changes */

	/* Set by PSPRadio */
	NULL,							/* Filled in by PSPRadio */
	NULL							/* GU functions to use in plugin */
};
/** END of Plugin definitions setup */


/* We export this function */
VisPlugin *get_vplugin_info()
{
	return &vtable;
}

#define m_Width  480
#define m_Height 272
#define m_Pitch  512
#define m_BytesPerPixel 4
#define FRAMESIZE (m_Pitch*m_Height*m_BytesPerPixel)
void scope_init()
{
	scope_config_update();
}

int y_mid = 127, pcm_shdiv = 15;

void scope_config_update()
{
	int sh = 0, amp = 0;
	int mid_a = 0;
	y_mid = (vtable.config->y2 + vtable.config->y1) / 2;
	
	if(vtable.config)
	{
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

void scope_cleanup()
{
}

void scope_start()
{
}

void scope_stop()
{
}

/* This is called from PSPRadio */
void draw_pcm(u32* vram);
void scope_render_pcm(u32* vram_frame, int16 *pcm_data)
{
	s_pcmbuffer = pcm_data;
	draw_pcm(vram_frame);
}
  
/* Some basic drawing routines */
void Plot(u32* vram, int x, int y, int color)
{
	u32 *pixel = vram + m_Pitch*y + x;
	*pixel = /* *pixel |*/ color;
}

void Rectangle(u32* vram, int x1, int y1, int x2, int y2, int color)
{
	int x,y;
	for (x = x1;x <= x2; x++)
	{
		for (y = y1;y <= y2; y++)
		{
			Plot(vram, x, y, color);
		}
	}
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

/* actual visualizer routine */
#ifdef SCOPE_BARS
void draw_pcm(u32* vram) /* BARS */
{
	int x;
	static int color = 0;
	for (x = vtable.config->x1; x < vtable.config->x2; x++)
	{
		//convert fixed point int to int (the integer part is the most significant byte)
		// (fixed_point >> 8) == integer part. We get a range from 0 < y < 128
		Rectangle(vram, x, vtable.config->y2 - (s_pcmbuffer[x*5] >> (pcm_shdiv - 1)), 
										   x+4, vtable.config->y2, RGB(color,color*2,color*8));
		color++;
	}
}
#endif

#ifdef SCOPE_DOTS
void draw_pcm(u32* vram) /* DOTS */
{
	//m_Screen->DrawBackground(iBuffer, 0, 0, 100, 100);
	//m_Screen->Rectangle(iBuffer, 0,0, 128, 128, 0);
	int x;
	int y1, y2;
	for (x = vtable.config->x1; x < vtable.config->x2; x++)
	{
		//convert fixed point int to int (the integer part is the most significant byte)
		// (fixed_point >> 8) == integer part. We get a range from 0 < y < 256
		y1 = y_mid + (s_pcmbuffer[x*5] >> pcm_shdiv);
		y2 = y_mid - (s_pcmbuffer[x*5+1] >> pcm_shdiv);
		Plot(vram, x, (y1 >= 0 && y1 < y_mid*2)?y1:y_mid, 0x00FFFF);
		Plot(vram, x, (y2 >= 0 && y2 < y_mid*2)?y2:y_mid, 0x11FFFF);
	}
}
#endif

#ifdef SCOPE_LINES
void draw_pcm(u32* vram) /* LINES */
{
	int x;
	int y1, y2;
	for (x = vtable.config->x1; x < vtable.config->x2; x++)
	{
		//convert fixed point int to int (the integer part is the most significant byte)
		// (fixed_point >> 8) == integer part. But I'll use >> 9 to get a range from 64 < y < 192
		y1 = y_mid - (s_pcmbuffer[x*5+1] >> pcm_shdiv); // L component
		y2 = y_mid + (s_pcmbuffer[x*5] >> pcm_shdiv);   // R component
		VertLine(vram, x, y1, y2, 0x00FF00);
	}
}
#endif

#ifdef SCOPE_TRAIL
void draw_pcm(u32* vram) /* TRAIL */
{
	int x;
	int y, old_y;
	old_y = y_mid;
	static u32 prev_pcm[512/*max*/];
	static int first_time = 1;
	if (first_time)
	{
		first_time = 0;
	}
	else
	{
		for (x = vtable.config->x1; x < vtable.config->x2; x++)
		{
			//convert fixed point int to int (the integer part is the most significant byte)
			// (fixed_point >> 8) == integer part. But I'll use >> 9 to get a range from 64 < y < 192
			y = prev_pcm[x - vtable.config->x1]; // L component
			VertLine(vram, x, old_y, y, 0xFFA0A0);
			old_y = y;
		}
	}
	for (x = vtable.config->x1; x < vtable.config->x2; x++)
	{
		//convert fixed point int to int (the integer part is the most significant byte)
		// (fixed_point >> 8) == integer part. But I'll use >> 9 to get a range from 64 < y < 192
		y = y_mid + (s_pcmbuffer[x*5] >> pcm_shdiv); // L component
		prev_pcm[x - vtable.config->x1] = y;
		VertLine(vram, x, old_y, y, 0x0AFF0A);
		old_y = y;
	}
}
#endif

#ifdef SCOPE_TWOCH 
void draw_pcm(u32* vram)
{
	int x;
	int yL, yR;
	int old_yL = y_mid;
	int old_yR = y_mid;
	for (x = vtable.config->x1; x < vtable.config->x2; x++)
	{
		//convert fixed point int to int (the integer part is the most significant byte)
		// (fixed_point >> 8) == integer part. But I'll use >> 9 to get a range from 64 < y < 192
		yL = y_mid + (s_pcmbuffer[x*5] >> pcm_shdiv); // L component
		yR = y_mid + (s_pcmbuffer[x*5+1] >> pcm_shdiv); // L component
		VertLine(vram, x, old_yL, yL, 0x00FFFF);
		VertLine(vram, x, old_yR, yR, 0x00FF00);
		old_yL = yL;
		old_yR = yR;
	}
}
#endif

