#ifndef _VIS_I_
	#define _VIS_I_

	#ifndef PLUGIN_TYPE
		#define PLUGIN_TYPE PLUGIN_VIS
	#endif

	#ifdef __cplusplus
		extern "C" {
	#endif

	#include <psptypes.h>
	
	/* Config is populated by PSPRadio */
	typedef struct _VisPluginConfig
	{
		/* Screen properties */
		int sc_width, sc_height, sc_pitch, sc_pixel_format;

		/* Upper left / Lower right corners (rectangle) for visualization to use when not in fullscreen mode */
		int x1,y1, x2,y2;

		/* reserved */
		char reserved[8];
	} VisPluginConfig;
		

	typedef struct _VisPlugin /* Based on xmms visual plugin to ease porting plugins written for xmms to PSPRadio */
	{
		void *handle; /* Filled in by PSPRadio */
		char *filename; /* Filled in by PSPRadio */
		char *description; /* The description that is shown in the preferences box */
		void (*init)(void); /* Called when the plugin is enabled */
		void (*cleanup)(void); /* Called when the plugin is disabled */
		void (*about)(void); /* not used atm *//* Show the about box */
		void (*configure)(void); /* not used atm *//* Show the configure box */
		void (*disable_plugin)(struct _VisPlugin *); /* not used atm *//* Call this with a pointer to your plugin to disable the plugin */
		void (*playback_start)(void); /* Called when playback starts */
		void (*playback_stop)(void); /* Called when playback stops */
		void (*render_pcm)(u32* vram_frame, int16 *pcm_data); /* Render the PCM (2ch/44KHz) data, don't do anything time consuming in here -- pcm_data has channels interleaved */
		void (*render_freq)(u32* vram_frame, int16 *freq_data); /* not implemented *//* Render the freq data, don't do anything time consuming in here */
		void (*config_update)(); /* Called by PSPRadio when config changes */
		VisPluginConfig *config; /* Filled in by PSPRadio */
	} VisPlugin; 

	/* The plugin exports this function */
	VisPlugin *get_vplugin_info();

	#ifdef __cplusplus
		};
	#endif

#endif
