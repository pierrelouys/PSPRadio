#ifndef _PSPRADIO_EXPORTS_H_
	#define _PSPRADIO_EXPORTS_H_

	#include <Logging.h>
	#include "../SharedLib/PSPApp/PSPSoundDeviceBuffer.h"

	typedef enum
	{
		PLUGIN_NA = 0,
		PLUGIN_UI,
		PLUGIN_FSS,
		PLUGIN_APP,
		PLUGIN_GAME,
		PLUGIN_VIS,
		/** This has to be the last */
		NUMBER_OF_PLUGIN_TYPES,
	}plugin_type;

	typedef struct
	{
		int BTN_TAKE_SCREENSHOT;
		int BTN_OK;
		int BTN_CANCEL;
		int BTN_STOP;
		int BTN_OPTIONS;
		int BTN_OPTIONS_EXIT;
		int BTN_CYCLE_SCREENS;
		int BTN_CYCLE_SCREENS_BACK;
		int BTN_BACK;
		int BTN_FWD;
		int BTN_PGDN;
		int BTN_PGUP;
		//for options screen
		int BTN_OPT_NAMES_FWD;
 		int BTN_OPT_NAMES_BACK;
		int BTN_OPT_OPTIONS_FWD;
		int BTN_OPT_OPTIONS_BACK;
		int BTN_OPT_ACTIVATE;
		
	}_button_mappings_struct_;
	
	#ifdef __cplusplus
		extern "C"
		{
	#endif

		#if defined(STAND_ALONE_APP)
			#define ModuleLog(level, format, args...) pspDebugScreenPrintf(format, ## args)
			#define PSPRadioExport_PluginExits(type) -1
			#define PSPRadioExport_GetProgramVersion() "N/A"
			#define PSPRadioExport_IsUSBEnabled() -1
			#define PSPRadioExport_GetMyIP() "N/A"
			#define PSPRadioExport_RequestExclusiveAccess(type) -1
			#define PSPRadioExport_GiveUpExclusiveAccess() -1
			#define PSPRadioExport_GetVersion() "N/A"
			#define PSPRadioExport_TakeScreenShot() -1
			#define PSPRadioExport_GetPCMBuffer() -1

		#else

			#define ModuleLog(level, format, args...) PSPRadioExport_Log(__FILE__, __LINE__, level, format, ## args)

			#ifndef __cplusplus
				#define bool int
			#endif

			int main(int, char**);
			int module_stop(int args, void *argp);

			void  PSPRadioExport_PluginExits(plugin_type type); /* Notify PSPRadio when exiting, so pspradio can unload the plugin */
			int   PSPRadioExport_Log(char *file, int line, loglevel_enum LogLevel, char *strFormat, ...);
			char *PSPRadioExport_GetProgramVersion();
			bool  PSPRadioExport_IsUSBEnabled();
			char *PSPRadioExport_GetMyIP();
			void  PSPRadioExport_RequestExclusiveAccess(plugin_type type);
			void  PSPRadioExport_GiveUpExclusiveAccess();
			char *PSPRadioExport_GetVersion();
			void  PSPRadioExport_TakeScreenShot();
			DeviceBuffer *PSPRadioExport_GetPCMBuffer();
			
			typedef enum
			{
				PSPRADIOIF_SET_BUTTONMAP_CONFIG,
				PSPRADIOIF_GET_SOUND_OBJECT,
				PSPRADIOIF_SET_RENDER_PCM,
				PSPRADIOIF_GET_VISUALIZER_CONFIG,
			}pspradioexport_types;
			
			typedef struct
			{
				int   Number;
				void *Pointer;
			}pspradioexport_ifdata;
				

			int PSPRadioIF(pspradioexport_types type, pspradioexport_ifdata *Data);

		#endif // #ifdef STAND_ALONE_APP
	#ifdef __cplusplus
		};
	#endif

#endif

