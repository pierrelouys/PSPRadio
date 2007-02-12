#ifndef _PSPRADIO_GAME_EXPORTS_H_
	#define _PSPRADIO_GAME_EXPORTS_H_

	#ifdef __cplusplus
	extern "C"
	{
	#endif

		#ifndef PLUGIN_TYPE
			#define PLUGIN_TYPE PLUGIN_GAME
		#endif

		int module_stop(int args, void *argp);
		int ModuleStartGAME();

	#ifdef __cplusplus
	}
	#endif

#endif
