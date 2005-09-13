/* 
 PSPApp
*/

#include <stdlib.h>
#include <string.h>
#include <limits.h>


#include "PSPApp.h"
extern CPSPApp PSPApp; /** You need to create a PSPApp class in your application! */

CPSPApp::CPSPApp()
{
	pspDebugScreenInit();
	
	m_thCallbackSetup = new CPSPThread("update_thread", callbacksetupThread);
	
	if (m_thCallbackSetup)
	{
		m_thCallbackSetup->Start();
	}

	printf("CPSPApp Constructor.\n");
	
}

CPSPApp::~CPSPApp()
{
	printf("CPSPApp Destructor.\n");
	
	return;
}

int CPSPApp::CallbackSetupThread(SceSize args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", exitCallback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();

	return 0;
}

int CPSPApp::OnAppExit(int arg1, int arg2, void *common)
{
	sceKernelExitGame();
	return 0;
}

/* ---statics--- */
/* System Callbacks */
int CPSPApp::exitCallback(int arg1, int arg2, void *common) 
{
	return PSPApp.OnAppExit(arg1, arg2, common);
}

/* Callback thread */
int CPSPApp::callbacksetupThread(SceSize args, void *argp) 
{
	return PSPApp.CallbackSetupThread(args, argp);
}


/** Overload new/delete */
void operator delete(void *p) { free(p); };
void operator delete[](void *p) { free(p); };
void *operator new(size_t iSize) { return (void*)malloc(iSize); };
void *operator new[](size_t iSize) { return (void *)malloc(iSize); };
