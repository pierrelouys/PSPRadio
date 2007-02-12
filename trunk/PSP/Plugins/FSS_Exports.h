#ifndef _PSPRADIO_FSS_EXPORTS_H_
	#define _PSPRADIO_FSS_EXPORTS_H_

	#ifdef __cplusplus
	extern "C" 
	{
	#endif

		#ifndef PLUGIN_TYPE
			#define PLUGIN_TYPE PLUGIN_FSS
		#endif

		int module_stop(int args, void *argp);
		int ModuleStartFSS();

	#ifdef __cplusplus
	}
	#endif

#endif
