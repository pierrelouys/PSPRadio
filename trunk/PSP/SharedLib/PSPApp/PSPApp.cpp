/* 
 PSPApp
*/
#include "PSPApp.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

class CPSPApp *pPSPApp = NULL; /** Do not access / Internal Use. */

CPSPApp::CPSPApp()
{
	pspDebugScreenInit();
	
	m_thCallbackSetup = new CPSPThread("update_thread", callbacksetupThread);
	
	memset(&m_pad, 0, sizeof (m_pad));
	pPSPApp = this;
	
	
	if (m_thCallbackSetup)
	{
		m_thCallbackSetup->Start();
		
		sceCtrlSetSamplingCycle(0);
		sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
		
		m_Exit = FALSE;
	}
	else /** Oops, error, let's exit the app */
	{
		m_Exit = TRUE;
	}

	printf("CPSPApp Constructor.\n");
	
}

CPSPApp::~CPSPApp()
{
	printf("CPSPApp Destructor.\n");
	
	return;
}

int CPSPApp::Run()
{
	short oldAnalogue = 0;
	
	while (m_Exit == FALSE)
	{
		sceDisplayWaitVblankStart();
		sceCtrlReadBufferPositive(&m_pad, 1); 
		
		OnVBlank();
 
		if (m_pad.Buttons != 0)
		{
			/** Button Pressed */
			OnButtonPressed(m_pad.Buttons);
		}
		
		if (oldAnalogue != (short)m_pad.Lx)
		{
			//pspDebugScreenSetXY(0,10);
			//printf ("Analog Lx=%03d Ly=%03d     ", m_pad.Lx, m_pad.Ly);
			//pspDebugScreenSetXY(0,5);
			oldAnalogue = (short)m_pad.Lx;
			OnAnalogueStickChange(m_pad.Lx, m_pad.Ly);
		}
	}
	
	sceKernelExitGame();

	return 0;
}

void CPSPApp::OnButtonPressed(int iButtonMask)
{
	if (iButtonMask & PSP_CTRL_SQUARE){
		printf("Square pressed \n");
	}
	if (iButtonMask & PSP_CTRL_TRIANGLE){
		printf("Triangle pressed \n");
	} 
}

int CPSPApp::CallbackSetupThread(SceSize args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", CPSPApp::exitCallback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();

	return 0;
}

int CPSPApp::OnAppExit(int arg1, int arg2, void *common)
{
	m_Exit = TRUE;
	return 0;
}

/** Audio */
void CPSPApp::EnableAudio()
{
	pspAudioInit();
	pspAudioSetChannelCallback(0, (void *)CPSPApp::audioCallback);
}

/* ---statics--- */
/* System Callbacks */
int CPSPApp::exitCallback(int arg1, int arg2, void *common) 
{
	return pPSPApp->OnAppExit(arg1, arg2, common);
}

/* Callback thread */
int CPSPApp::callbacksetupThread(SceSize args, void *argp) 
{
	return pPSPApp->CallbackSetupThread(args, argp);
}

void CPSPApp::audioCallback(void* buf, unsigned int length) 
{
	pPSPApp->OnAudioBufferEmpty(buf, length);
}

/** Overload new/delete */
void operator delete(void *p) { free(p); };
void operator delete[](void *p) { free(p); };
void *operator new(size_t iSize) { return (void*)malloc(iSize); };
void *operator new[](size_t iSize) { return (void *)malloc(iSize); };
