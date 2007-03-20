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

PSP_MODULE_INFO("VIS_SAMPLE", 0, 1, 1);
PSP_HEAP_SIZE_KB(0);

/* Prototypes */
void scope_config_update();
void scope_init();
void scope_render_pcm(u32* vram_frame, int16 *pcm_data);
void VertLine(u32* vram, int x, int y1, int y2, int color);

/** START of Plugin definitions setup */
VisPlugin sample_vtable = {
	PLUGIN_VIS_VERSION, /* Interface version -- Don't change */
	NULL, //void *handle; /* Filled in by PSPRadio */
	NULL, //char *filename; /* Filled in by PSPRadio */
	"Sample Visualizer Plugin", //char *description; /* The description that is shown in the preferences box */
	scope_init, //void (*init)(void); /* Called when the plugin is enabled */
	NULL, //void (*cleanup)(void); /* Called when the plugin is disabled */
	NULL, //void (*about)(void); /* Show the about box */:
	NULL, //void (*configure)(void); /* Show the configure box */
	NULL, //void (*disable_plugin)(struct _VisPlugin *); /* Call this with a pointer to your plugin to disable the plugin */
	NULL, //void (*playback_start)(void); /* Called when playback starts */
	NULL, //void (*playback_stop)(void); /* Called when playback stops */
	scope_render_pcm, //void (*render_pcm)(int16 *pcm_data); /* Render the PCM data, don't do anything time consuming in here -- pcm_data has channels interleaved */
	NULL, //void (*render_freq)(int16 *freq_data); /* not implemented *//* Render the freq data, don't do anything time consuming in here */
	scope_config_update, /* Called when config changes */
	NULL, //VisPluginConfig *config; /* Filled in by PSPRadio */
};
/** START of Plugin definitions setup */

/** Single Plugin Export */
VisPlugin *get_vplugin_info()
{
	return &sample_vtable;
}

/* Called from PSPRadio on initialization */
void scope_init()
{
	scope_config_update();
}

/* Called from PSPRadio when the config pointer has been updated */
int y_mid = 127, pcm_shdiv = 15;
void scope_config_update()
{
	int sh = 0, amp = 0;
	int mid_a = 0;

	if(sample_vtable.config)
	{
		y_mid = (sample_vtable.config->y2 + sample_vtable.config->y1) / 2;

		mid_a = (sample_vtable.config->y2 - sample_vtable.config->y1) / 2;
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

/* This is called from PSPRadio */
/* (actual visualizer routine) */
void scope_render_pcm(u32* vram_frame, int16 *pcm_data)
{
	int x;
	int yL, yR;
	int old_yL = y_mid;
	int old_yR = y_mid;
	for (x = sample_vtable.config->x1; x < sample_vtable.config->x2; x++)
	{
		/* convert fixed point int to int (the integer part is the most significant byte) */
		/* (fixed_point >> 8) == integer part. But I'll use >> 9 to get a range from 64 < y < 192 */
		yL = y_mid + (pcm_data[x*5]   >> pcm_shdiv); /* L component */
		yR = y_mid + (pcm_data[x*5+1] >> pcm_shdiv); /* R component */
		VertLine(vram_frame, x, old_yL, yL, 0xFF0000/* Blue */);
		VertLine(vram_frame, x, old_yR, yR, 0x00FF00/* Green */);
		old_yL = yL;
		old_yR = yR;
	}
}
  
/* Some basic drawing routines */
void Plot(u32* vram, int x, int y, int color)
{
	u32 *pixel = vram + sample_vtable.config->sc_pitch*y + x;
	*pixel = color;
}

void VertLine(u32* vram, int x, int y1, int y2, int color)
{
	int y;
	if (y1 < y2)
	{
		for (y = (y1<0)?0:y1; y <= y2; y++)
		{
			Plot(vram, x, y, color);
		}
	}
	else if (y2 < y1)
	{
		for (y = (y2<0)?0:y2; y <= y1; y++)
		{
			Plot(vram, x, y, color);
		}
	}
	else 
	{
		Plot(vram, x, y1, color);
	}
	
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
