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

PSP_MODULE_INFO("VIS_SPECTRUM", 0, 1, 1);
PSP_HEAP_SIZE_KB(0);

/* Prototypes */
void spect_config_update();
void spect_init();
void spect_render_freq(u32* vram_frame, float freq_data[2][257]);
void VertLine (u32* vram, int x, int y1, int y2, int color);
void Rectangle(u32* vram, int x1, int y1, int x2, int y2, int color);

/** START of Plugin definitions setup */
VisPlugin sample_vtable = {
	PLUGIN_VIS_VERSION, /* Interface version -- Don't change */
	NULL, //void *handle; /* Filled in by PSPRadio */
	NULL, //char *filename; /* Filled in by PSPRadio */
	"Spectrum Visualizer Plugin", //char *description; /* The description that is shown in the preferences box */
	spect_init, //void (*init)(void); /* Called when the plugin is enabled */
	NULL, //void (*cleanup)(void); /* Called when the plugin is disabled */
	NULL, //void (*about)(void); /* Show the about box */:
	NULL, //void (*configure)(void); /* Show the configure box */
	NULL, //void (*disable_plugin)(struct _VisPlugin *); /* Call this with a pointer to your plugin to disable the plugin */
	NULL, //void (*playback_start)(void); /* Called when playback starts */
	NULL, //void (*playback_stop)(void); /* Called when playback stops */
	NULL, //spect_render_pcm, //void (*render_pcm)(int16 *pcm_data); /* Render the PCM data, don't do anything time consuming in here -- pcm_data has channels interleaved */
	spect_render_freq,  //void (*render_freq)(int16 *freq_data); /* not implemented *//* Render the freq data, don't do anything time consuming in here */
	spect_config_update, /* Called when config changes */
	NULL, //VisPluginConfig *config; /* Filled in by PSPRadio */
};
/** START of Plugin definitions setup */

/** Single Plugin Export */
VisPlugin *get_vplugin_info()
{
	return &sample_vtable;
}

/* Linearity of the amplitude scale (0.5 for linear, keep in [0.1, 0.9]) */
#define d 0.33

/* Time factor of the band dinamics. 3 means that the coefficient of the
   last value is half of the current one's. (see source) */
#define tau 3

/* Factor used for the diffusion. 4 means that half of the height is
   added to the neighbouring bars */
#define dif 4

/*static gint timeout_tag;*/
static double scale, x00, y00;
static int16 bar_heights[512];
#define NUM_BANDS 20
int conf_width = 480;
int conf_height = 271;
int band_width = 48;

/* Called from PSPRadio on initialization */
void spect_init()
{
	vfpu_srand(0);
	spect_config_update();
}

/* Called from PSPRadio when the config pointer has been updated */
void spect_config_update()
{
	if(sample_vtable.config)
	{
		conf_width  = (sample_vtable.config->x2 - sample_vtable.config->x1);
		conf_height = (sample_vtable.config->y2 - sample_vtable.config->y1);
		band_width  = conf_width / NUM_BANDS;

		scale = conf_height / ( vfpu_logf((1 - d) / d) * 2 );
		x00 = d*d*32768.0/(2 * d - 1);
		y00 = -vfpu_logf(-x00) * scale;

	}

	sceKernelChangeCurrentThreadAttr(0, PSP_THREAD_ATTR_VFPU);
}	

/* This is called from PSPRadio */
/* (actual visualizer routine) */
/* Based on "finespectrum.c" visualizer for XMMS
   Copyright (C) 1998-2001 Vágvölgyi Attila, Peter Alm, Mikael Alm,
   Olle Hallnas, Thomas Nilsson and 4Front Technologies */
void spect_render_freq(u32* vram_frame, float data[2][257])
{
	int i;
	double y;
	float xo;
	float xof = ((float)conf_width - (float)band_width)/257;
	xo = 0;

	for (i = 0; i < 257; i+=(257/NUM_BANDS)) {
		y = (double)(data[0][i]) * (i + 1); /* Compensating the energy */
		y = ( vfpu_logf(y - x00) * scale + y00 ); /* Logarithmic amplitude */

		y = ( (dif-2)*y + /* FIXME: conditionals should be rolled out of the loop */
			(i==0            ? y : bar_heights[i-1]) +
			(i==conf_width-1 ? y : bar_heights[i+1])) / dif; /* Add some diffusion */
		y = ((tau-1)*bar_heights[i] + y) / tau; /* Add some dynamics */
		bar_heights[i] = (int16)y;

		Rectangle(vram_frame, 
				  sample_vtable.config->x1 + xo, sample_vtable.config->y2, 
				  sample_vtable.config->x1 + xo + band_width - 5, sample_vtable.config->y2 - bar_heights[i], 
				  0xFFFFFF);

		xo = (i*xof)+band_width;
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
	
	if (x < 0)
		x = 0;
	if (x > 480)
		x = 480;

	if (y1 < y2)
	{
		if (y1 < 0)
			y1 = 0;
		if (y2 > 272)
			y2 = 272;
		
		for (y = (y1<0)?0:y1; y <= y2; y++)
		{
			Plot(vram, x, y, color);
		}
	}
	else if (y2 < y1)
	{
		if (y2 < 0)
			y2 = 0;
		if (y1 > 272)
			y1 = 272;

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

void Rectangle(u32* vram, int x1, int y1, int x2, int y2, int color)
{
	int x;
	for (x = x1; x <= x2; x++)
	{
		VertLine(vram, x, y1, y2, color);
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
