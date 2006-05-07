#ifndef _PSPRADIO_EXPORTS_H_
	#define _PSPRADIO_EXPORTS_H_

	#include <Logging.h>
	#include "../SharedLib/PSPApp/PSPSoundDeviceBuffer.h"

		typedef enum
		{
			PLUGIN_UI,
			PLUGIN_FSS,
			PLUGIN_APP,
			PLUGIN_GAME,
			/** This has to be the last */
			NUMBER_OF_PLUGIN_TYPES,
		}plugin_type;


	#ifdef __cplusplus
		extern "C"
		{
	#endif

		#ifdef STAND_ALONE_APP
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

		#endif // #ifdef STAND_ALONE_APP
	#ifdef __cplusplus
		};
	#endif

#endif

