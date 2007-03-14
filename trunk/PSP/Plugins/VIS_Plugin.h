#ifndef _VIS_I_
	#define _VIS_I_

	#ifndef PLUGIN_TYPE
		#define PLUGIN_TYPE PLUGIN_VIS
	#endif

	#ifdef __cplusplus
		extern "C" {
	#endif

	#include <psptypes.h>
	
	typedef struct _VisPluginConfig
	{
		int x1;
		int y1;
		int x2;
		int y2;
		//int y_mid;
		//int pcm_right_shift;
	} VisPluginConfig;
		

	typedef struct _VisPlugin /* Based on xmms visual plugin to ease porting plugins written for xmms to PSPRadio */
	{
		void *handle; /* Filled in by PSPRadio */
		char *filename; /* Filled in by PSPRadio */
		int PSPRadio_session; /* not used atm *//* The session ID for attaching to the control socket */
		char *description; /* The description that is shown in the preferences box */
		int num_pcm_chs_wanted; /* not used atm *//* Numbers of PCM channels wanted in the call to render_pcm */
		int num_freq_chs_wanted; /* not used atm *//* Numbers of freq channels wanted in the call to render_freq */
		void (*init)(void); /* Called when the plugin is enabled */
		void (*cleanup)(void); /* Called when the plugin is disabled */
		void (*about)(void); /* Show the about box */
		void (*configure)(void); /* Show the configure box */
		void (*disable_plugin)(struct _VisPlugin *); /* Call this with a pointer to your plugin to disable the plugin */
		void (*playback_start)(void); /* Called when playback starts */
		void (*playback_stop)(void); /* Called when playback stops */
		void (*render_pcm)(u32* vram_frame, int16 *pcm_data); /* Render the PCM data, don't do anything time consuming in here -- pcm_data has channels interleaved */
		void (*render_freq)(u32* vram_frame, int16 *freq_data); /* not implemented *//* Render the freq data, don't do anything time consuming in here */
		void (*config_update)(); /* Called when config changes */
		VisPluginConfig *config; /* Filled in by PSPRadio */
	} VisPlugin; 

	/* The plugin exports this function */
	VisPlugin *get_vplugin_info();

	#ifdef __cplusplus
		};
	#endif

#endif
