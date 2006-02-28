#ifndef _PSPRADIO_EXPORTS_H_
	#define _PSPRADIO_EXPORTS_H_

	#define ModuleLog(level, format, args...) PSPRadioExport_Log(__FILE__, __LINE__, level, format, ## args)
	
	extern "C" 
	{
		int main(int, char**);
		int module_stop(int args, void *argp);

		int   PSPRadioExport_Log(char *file, int line, loglevel_enum LogLevel, char *strFormat, ...);
		char *PSPRadioExport_GetProgramVersion();
		bool  PSPRadioExport_IsUSBEnabled();
		char *PSPRadioExport_GetMyIP();
	}

#endif

