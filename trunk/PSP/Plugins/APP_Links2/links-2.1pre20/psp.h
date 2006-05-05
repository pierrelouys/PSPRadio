#ifndef psp_h
	#define psp_h

	#ifndef tBoolean
	#define tBoolean int
	#define truE 1
	#define falsE 0
	#define KEY_BACKSPACE 8
	#endif
		
	extern volatile tBoolean g_PSPEnableRendering;
	extern volatile tBoolean g_PSPEnableInput;

	#include <unistd.h>

	typedef struct _extension_download_dirs 
	{
		char music[MAXPATHLEN+1];
		char mp4[MAXPATHLEN+1];
		char videos[MAXPATHLEN+1];
		char images[MAXPATHLEN+1];
		char other[MAXPATHLEN+1];
	}extension_download_dirs;

	extern extension_download_dirs ext_dl_dir;

#endif
