/* 
 PSPApp
*/
#include <new>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <pspnet.h>
#include "PSPApp.h"

class CPSPApp *pPSPApp = NULL; /** Do not access / Internal Use. */

CPSPApp::CPSPApp()
{
	pspDebugScreenInit();
	
	m_thCallbackSetup = new CPSPThread("update_thread", callbacksetupThread);
	
	memset(&m_pad, 0, sizeof (m_pad));
	pPSPApp = this;
	strcpy(m_strMyIP, "0.0.0.0");
	
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
	DisableNetwork();

	return;
}

int CPSPApp::Run()
{
	short oldAnalogue = 0;
	
	while (m_Exit == FALSE)
	{
		//sceKernelSleepThread();
		sceDisplayWaitVblankStart();
		sceKernelDelayThread(10);
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
	//sceKernelDelayThread(1000);
	return 0;
}

/** Audio */
void CPSPApp::EnableNetwork()
{
	nlhLoadDrivers();
	WLANConnectionHandler();
}

void CPSPApp::DisableNetwork()
{
	u32 err;
	err = sceNetApctlDisconnect();
	if (err != 0) {
		printf("ERROR - main.WLANConnectionHandler : sceNetApctlDisconnect returned '%d'.\n", err);
    }

    err = nlhTerm();
	if (err != 0) {
		printf("ERROR - main.WLANConnectionHandler : nlhTerm returned '%d'.\n", err);
    }
}

/** From FTPD */
void CPSPApp::WLANConnectionHandler() 
{
    u32 err;

    err = nlhInit();
    if (err != 0) {
		printf("ERROR - main.WLANConnectionHandler : nlhInit returned '%d'.\n", err);
        DisableNetwork();
    }

	err = sceNetApctlAddHandler(NetApctlCallback, NULL);
	if (err != 0) {
		printf("ERROR - main.WLANConnectionHandler : sceNetApctlAddHandler returned '%d'.\n", err);
        DisableNetwork();
    }

	err = sceNetApctlConnect(0);
    if (err != 0) {
		printf("ERROR - main.WLANConnectionHandler : sceNetApctlConnect returned '%d'.\n", err);
        DisableNetwork();
    }
  
	printf("INFO  - main.WLANConnectionHandler : Started Successfully.\n");
	//threadFtpLoop=sceKernelCreateThread("THREAD_FTPD_SERVERLOOP", &ftpdLoop, 0x18, 0x10000, 0, NULL);
	//if(threadFtpLoop >= 0) {
	//	sceKernelStartThread(threadFtpLoop, 0, 0);
	//} else {
	//	printf("ERROR - main.WLANConnectionHandler : Impossible to create server loop thread.\n", err);
	//}

//	printf("INFO  - main.WLANConnectionHandler : Waiting for exit signal.\n");
//	/* waiting for exit */
//	sceKernelWaitSema(exitSema, 1, 0);


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

void CPSPApp::NetApctlCallback(int old_state, int state, int event, int error, void* arg) 
{

	if (state==0) 
	{
		// little pause
		sceKernelDelayThread(500000);
		// idle => connect
		sceNetApctlConnect(0);
	}
	
	printf("Establishing Connection... ");
	char *msg = "";
	switch (state) 
	{
	case apctl_state_disconnected:
		msg = "Disconnected";
		break;
	case apctl_state_scanning:
		msg = "Scanning";
		break;
	case apctl_state_joining:
		msg = "Joining";
		break;
	case apctl_state_IPObtaining:
		msg = "Obtaining IP";
		break;
	case apctl_state_IPObtained:
		msg = "IP Retrieved";

		char ipaddress[32]; ipaddress[0]=0;
		if (sceNetApctlGetInfo(8, ipaddress) != 0) {
			printf("ERROR - ApctlCallback: Impossible to get IP address of the PSP.\n");
		}
		strcat(pPSPApp->m_strMyIP, ipaddress);
		
		printf("PSP's IP is '%s'\n", pPSPApp->m_strMyIP);

		break;
	}
	
	printf ("APCTL State: %s\n", msg);
}

#if 0
/** Overload new/delete */
void operator delete(void *p) { free(p); };
void operator delete[](void *p) { free(p); };
void *operator new(size_t iSize) { return (void*)malloc(iSize); };
void *operator new[](size_t iSize) { return (void *)malloc(iSize); };
#endif
