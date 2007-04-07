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
void HorizLine(u32* vram, int y, int x1, int x2, int color);
void Rectangle(u32* vram, int x1, int y1, int x2, int y2, int color);

/** START of Plugin definitions setup */
VisPlugin vtable = 
{
	/* Populated by Plugin */
	PLUGIN_VIS_VERSION,		 		/* Populate with PLUGIN_VIS_VERSION */
	"Spectrum Visualizer Plugin",		/* Plugin description */
	"By Raf",	 					/* Plugin about info */
	VIS_TYPE_SW,
	spect_init,			 			/* Called when the plugin is enabled */
	NULL,				 			/* Called when the plugin is disabled */
	NULL,						 	/* not used atm *//* Called when playback starts */
	NULL,
	NULL,						 	/* not used atm *//* Called when playback stops */
	/* Render the PCM (2ch/44KHz) data, pcm_data has 2 channels interleaved */
	NULL, 
	/* Render the freq data, don't do anything time consuming in here */
	spect_render_freq,
	spect_config_update,	 		/* Called by PSPRadio when config changes */

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

/* Linearity of the amplitude scale (0.5 for linear, keep in [0.1, 0.9]) */
#define d 0.33

/* Time factor of the band dinamics. 3 means that the coefficient of the
   last value is half of the current one's. (see source) */
#define tau 3

/* Factor used for the diffusion. 4 means that half of the height is
   added to the neighbouring bars */
#define dif 4

/*static gint timeout_tag;*/
static float scale, x00, y00;
static int16 bar_heights[512];
#define NUM_BANDS 20
int conf_width = 480;
int conf_height = 271;
float band_width = 48.0f;

/* Called from PSPRadio on initialization */
void spect_init()
{
	vfpu_srand(0);
	spect_config_update();
}

/* Called from PSPRadio when the config pointer has been updated */
void spect_config_update()
{
	if(vtable.config)
	{
		conf_width  = (vtable.config->x2 - vtable.config->x1);
		conf_height = (vtable.config->y2 - vtable.config->y1);
		band_width  = conf_width / NUM_BANDS;

		scale = conf_height / ( vfpu_logf((1 - d) / d) * 2 );
		x00 = d*d*32768.0f/(2 * d - 1);
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
	float y;
	float xo;
	float xof = ((float)conf_width - (float)band_width)/257;
	xo = 0;

	for (i = 0; i < 257; i+=(257/NUM_BANDS)) {
		y = (float)(data[0][i]) * (i + 1); /* Compensating the energy */
		y = ( vfpu_logf(y - x00) * scale + y00 ); /* Logarithmic amplitude */

		y = ( (dif-2)*y + /* FIXME: conditionals should be rolled out of the loop */
			(i==0            ? y : bar_heights[i-1]) +
			(i==conf_width-1 ? y : bar_heights[i+1])) / dif; /* Add some diffusion */
		y = ((tau-1)*bar_heights[i] + y) / tau; /* Add some dynamics */
		bar_heights[i] = (int16)y;

		Rectangle(vram_frame, 
				  vtable.config->x1 + xo, vtable.config->y2, 
				  vtable.config->x1 + xo + band_width - 5, vtable.config->y2 - bar_heights[i], 
				  0xFFFFFF);

		xo = (i*xof)+band_width;
	}
}
  
/* Some basic drawing routines */
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#define RGB(r,g,b) ((((b)&0xFF)<<16) | (((g)&0xFF)<<8) | (((r)&0xFF)))
void Rectangle(u32* vram, int x1, int y1, int x2, int y2, int color)
{
	int y, ya, yb;
	int x, xa, xb;
	u32 *linestart;
	u32 *pixel;
	float color_factor;
	ya = min(y1,y2);
	yb = max(y1,y2);
	xa = min(x1,x2);
	xb = max(x1,x2);

	xa = xa<0?0:xa;
	xb = xb<0?0:xb;
	ya = ya<0?0:ya;
	yb = yb<0?0:yb;
	
	linestart = vram + vtable.config->sc_pitch*ya + xa;
	pixel = linestart;
	color_factor = 255.0f/(yb-ya+1.0f);

	for (y = ya; y <= yb; y++)
	{
		if (y%2)
		for (x = x1; x <= x2; x+=2)
		{
			*pixel++ = RGB(0, 0xFF - ((u32)((y-ya+1.0f)*color_factor)), 0xFF);
			pixel++;
		}
		linestart += vtable.config->sc_pitch;
		pixel = linestart;
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
