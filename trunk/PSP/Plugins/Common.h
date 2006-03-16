#ifndef _MODULE_COMMON_H_
	#define _MODULE_COMMON_H_
	extern "C" 
	{
	
		int main(int argc, char **argv);
		
		void* getModuleInfo(void);
		
		int module_stop(int args, void *argp);
	
	}

#endif
