#ifndef _VIS_I_
	#define _VIS_I_

	#ifndef PLUGIN_TYPE
		#define PLUGIN_TYPE PLUGIN_VIS
	#endif

	#ifdef __cplusplus
		extern "C" {
	#endif

	#include <psptypes.h>
	#include <pspgu.h>	

	#define PLUGIN_VIS_VERSION	2

	/* Config is populated by PSPRadio */
	typedef struct _VisPluginConfig
	{
		/* Screen properties */
		int sc_width, sc_height, sc_pitch, sc_pixel_format;

		/* Upper left / Lower right corners (rectangle) for visualization */
		int x1,y1, x2,y2;

		/* Fullscreen: 1 = true, 0 = false */
		int fullscreen; 

		/* For future use */
		char reserved[32];
	} VisPluginConfig;
	
	/* Plugin specifies how it wants to render */
	typedef enum _visualizer_type
	{
		VIS_TYPE_SW,   /* Software; render in render_pcm() or render_freq() */
		VIS_TYPE_GU,   /* GU; render in render_pcm() or render_freq() */
		VIS_TYPE_EXCL  /* Exclusive mode - advanced - */
	} visualizer_type;

	/* Loosely based on xmms visual plugin interface */
	typedef struct _VisPlugin 
	{
		/* Set by Plugin */
		int  interface_version; 		/* Populate with PLUGIN_VIS_VERSION */
		char *description; 				/* Plugin description */
		char *about; 					/* Plugin about info */
		visualizer_type type;			/* Specify the type of visualizer */
		void (*init)(void); 			/* Called when the plugin is enabled */
		void (*cleanup)(void); 			/* Called when the plugin is disabled */
		void (*playback_start)(void); 	/* Called when playback starts */
		void (*playback_pause)(void); 	/* Called when playback pauses */
		void (*playback_stop)(void); 	/* Called when playback stops */
		/* Render the PCM (2ch/44KHz) data, pcm_data has 2 channels interleaved */
		void (*render_pcm)(u32* vram_frame, int16 *pcm_data); 
		/* Render the freq data */
		void (*render_freq)(u32* vram_frame, float freq_data[2][257]); 
		void (*config_update)(); 		/* Called by PSPRadio when *config changes */

		/* Set by PSPRadio */
		VisPluginConfig *config;		/* Filled in by PSPRadio */
		/* Song info: */
		int CurrentTime, TotalTime; /* Seconds */
		void (*get_song_data)(char *szURI, char *szTitle, char *szArtist, int buff_size); /* pass pre-allocated buffers */
	} VisPlugin; 

	/* The plugin exports this function */
	VisPlugin *get_vplugin_info();


	#ifdef __cplusplus
		};
	#endif

#endif
