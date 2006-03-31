#ifndef _PSPRADIO_APP_EXPORTS_H_
	#define _PSPRADIO_APP_EXPORTS_H_

	#ifdef __cplusplus
	extern "C" 
	{
	#endif

		int module_stop(int args, void *argp);
		int ModuleStartAPP();
		int ModuleContinueApp();

	#ifdef __cplusplus
	}
	#endif

#endif
