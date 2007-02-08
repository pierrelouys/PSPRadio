#ifndef _MODULE_COMMON_H_
	#define _MODULE_COMMON_H_
	
	#ifdef __cplusplus
	extern "C" 
	{
	#endif
	
		int main(int argc, char **argv);
		void* getModuleInfo(void);
		int module_stop(int args, void *argp);
	
	#ifdef __cplusplus
	}
	#endif

#endif
