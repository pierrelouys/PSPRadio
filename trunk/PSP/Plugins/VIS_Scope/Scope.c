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
#include <Tools.h>
#include <stdarg.h>
#include <pthread.h>
#include "../VIS_Plugin.h"
#include "Scope.h"

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
};

#define true 1
#define false 0
#define RGB2BGR(x) (((x>>16)&0xFF) | (x&0xFF00) | ((x<<16)&0xFF0000))

//#define VIS_WIDTH (scope_vtable.config->x2 - scope_vtable.config->x1)
short *s_pcmbuffer = NULL;

/* Plugin setup */
void scope_init();
void scope_cleanup();
void scope_start();
void scope_stop();
void scope_render_pcm(u32* vram_frame, int16 *pcm_data);
VisPlugin scope_vtable = {
	//void *handle; /* Filled in by PSPRadio */
	NULL, 
	//char *filename; /* Filled in by PSPRadio */
	NULL, 
	//VisPluginConfig *config; /* Filled in by PSPRadio */
	NULL,
	//int PSPRadio_session; /* not used atm *//* The session ID for attaching to the control socket */
	0, 
	//char *description; /* The description that is shown in the preferences box */
	"Scope Plugin", 
	//int num_pcm_chs_wanted; /* not used atm *//* Numbers of PCM channels wanted in the call to render_pcm */
	2, 
	//int num_freq_chs_wanted; /* not used atm *//* Numbers of freq channels wanted in the call to render_freq */
	0, 
	//void (*init)(void); /* Called when the plugin is enabled */
	scope_init, 
	//void (*cleanup)(void); /* Called when the plugin is disabled */
	scope_cleanup, 
	//void (*about)(void); /* Show the about box */:
	NULL, 
	//void (*configure)(void); /* Show the configure box */
	NULL, 
	//void (*disable_plugin)(struct _VisPlugin *); /* Call this with a pointer to your plugin to disable the plugin */
	NULL, 
	//void (*playback_start)(void); /* Called when playback starts */
	scope_start, 
	//void (*playback_stop)(void); /* Called when playback stops */
	scope_stop, 
	//void (*render_pcm)(int16 *pcm_data); /* Render the PCM data, don't do anything time consuming in here -- pcm_data has channels interleaved */
	scope_render_pcm, 
	/* not implemented *//* Render the freq data, don't do anything time consuming in here */
	NULL, //void (*render_freq)(int16 *freq_data); 
};
/* We export this function */
VisPlugin *get_vplugin_info()
{
	return &scope_vtable;
}

#define m_Width  480
#define m_Height 272
#define m_Pitch  512
#define m_BytesPerPixel 4
#define FRAMESIZE (m_Pitch*m_Height*m_BytesPerPixel)
void scope_init()
{
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
	*pixel = *pixel | color;
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
	for (x = scope_vtable.config->x1; x < scope_vtable.config->x2; x++)
	{
		//convert fixed point int to int (the integer part is the most significant byte)
		// (fixed_point >> 8) == integer part. We get a range from 0 < y < 128
		Rectangle(vram, x, scope_vtable.config->y_mid - (s_pcmbuffer[x*5] >> scope_vtable.config->pcm_right_shift), 
										   x+4, scope_vtable.config->y_mid, 0xAAAAAA);
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
	for (x = scope_vtable.config->x1; x < scope_vtable.config->x2; x++)
	{
		//convert fixed point int to int (the integer part is the most significant byte)
		// (fixed_point >> 8) == integer part. We get a range from 0 < y < 256
		y1 = scope_vtable.config->y_mid + (s_pcmbuffer[x*5] >> scope_vtable.config->pcm_right_shift);
		y2 = scope_vtable.config->y_mid - (s_pcmbuffer[x*5+1] >> scope_vtable.config->pcm_right_shift);
		Plot(vram, x, (y1 >= 0 && y1 < scope_vtable.config->y_mid*2)?y1:scope_vtable.config->y_mid, 0xAAAAAA);
		Plot(vram, x, (y2 >= 0 && y2 < scope_vtable.config->y_mid*2)?y2:scope_vtable.config->y_mid, 0xAAAAAA);
	}
}
#endif

#ifdef SCOPE_LINES
void draw_pcm(u32* vram) /* LINES */
{
	int x;
	int y1, y2;
	for (x = scope_vtable.config->x1; x < scope_vtable.config->x2; x++)
	{
		//convert fixed point int to int (the integer part is the most significant byte)
		// (fixed_point >> 8) == integer part. But I'll use >> 9 to get a range from 64 < y < 192
		y1 = scope_vtable.config->y_mid - (s_pcmbuffer[x*5+1] >> scope_vtable.config->pcm_right_shift); // L component
		y2 = scope_vtable.config->y_mid + (s_pcmbuffer[x*5] >> scope_vtable.config->pcm_right_shift);   // R component
		VertLine(vram, x, y1, y2, 0xAAAAAA);
	}
}
#endif

#ifdef SCOPE_LINES2
void draw_pcm(u32* vram)
{
	int x;
	int y, old_y;
	old_y = scope_vtable.config->y_mid;
	for (x = scope_vtable.config->x1; x < scope_vtable.config->x2; x++)
	{
		//convert fixed point int to int (the integer part is the most significant byte)
		// (fixed_point >> 8) == integer part. But I'll use >> 9 to get a range from 64 < y < 192
		y = scope_vtable.config->y_mid + (s_pcmbuffer[x*5] >> scope_vtable.config->pcm_right_shift); // L component
		VertLine(vram, x, old_y, y, 0xAAAAAA);
		old_y = y;
	}
}
#endif

#ifdef SCOPE_LINES3
void draw_pcm(u32* vram) /* TRAIL */
{
	int x;
	int y, old_y;
	old_y = scope_vtable.config->y_mid;
	static u32 prev_pcm[512/*max*/];
	static bool first_time = true;
	if (first_time)
	{
		first_time = false;
	}
	else
	{
		for (x = scope_vtable.config->x1; x < scope_vtable.config->x2; x++)
		{
			//convert fixed point int to int (the integer part is the most significant byte)
			// (fixed_point >> 8) == integer part. But I'll use >> 9 to get a range from 64 < y < 192
			y = prev_pcm[x - scope_vtable.config->x1]; // L component
			VertLine(vram, x, old_y, y, 0xFFA0A0);
			old_y = y;
		}
	}
	for (x = scope_vtable.config->x1; x < scope_vtable.config->x2; x++)
	{
		//convert fixed point int to int (the integer part is the most significant byte)
		// (fixed_point >> 8) == integer part. But I'll use >> 9 to get a range from 64 < y < 192
		y = scope_vtable.config->y_mid + (s_pcmbuffer[x*5] >> scope_vtable.config->pcm_right_shift); // L component
		prev_pcm[x - scope_vtable.config->x1] = y;
		VertLine(vram, x, old_y, y, 0x0AFF0A);
		old_y = y;
	}
}
#endif

#ifdef SCOPE_LINES4 
void draw_pcm(u32* vram)
{
	int x;
	int yL, yR;
	int old_yL = scope_vtable.config->y_mid;
	int old_yR = scope_vtable.config->y_mid;
	for (x = scope_vtable.config->x1; x < scope_vtable.config->x2; x++)
	{
		//convert fixed point int to int (the integer part is the most significant byte)
		// (fixed_point >> 8) == integer part. But I'll use >> 9 to get a range from 64 < y < 192
		yL = scope_vtable.config->y_mid + (s_pcmbuffer[x*5] >> scope_vtable.config->pcm_right_shift); // L component
		yR = scope_vtable.config->y_mid + (s_pcmbuffer[x*5+1] >> scope_vtable.config->pcm_right_shift); // L component
		VertLine(vram, x, old_yL, yL, 0xFF0A0A);
		VertLine(vram, x, old_yR, yR, 0xA0FFA0);
		old_yL = yL;
		old_yR = yR;
	}
}
#endif

