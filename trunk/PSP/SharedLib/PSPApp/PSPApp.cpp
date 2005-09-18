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
int CPSPApp::EnableNetwork()
{
	int iRet = 0;
	if (nlhLoadDrivers() == 0)
	{
		if (WLANConnectionHandler() == 0)
		{
			printf("PSP IP = %s\n", GetMyIP());
			iRet = 0;
		}
		else
		{
			printf("Error starting network\n");
			iRet = -1;
		}
	}
	else
	{
		printf("Error loading network drivers\n");
		iRet = -1;
	}
	return iRet;
}

void CPSPApp::DisableNetwork()
{
	u32 err;
	err = sceNetApctlDisconnect();
	if (err != 0) 
	{
		printf("ERROR - DisableNetwork: sceNetApctlDisconnect returned '%d'.\n", err);
    }

    err = nlhTerm();
	if (err != 0) 
	{
		printf("ERROR - DisableNetwork: nlhTerm returned '%d'.\n", err);
    }
}

/** From FTPD */
int CPSPApp::WLANConnectionHandler() 
{
    u32 err;
    int iRet = 0;

    err = nlhInit();
    if (err != 0) {
		printf("ERROR - WLANConnectionHandler : nlhInit returned '%d'.\n", err);
        DisableNetwork();
        iRet = -1;
    }

	err = sceNetApctlConnect(/**profile */0);
    if (err != 0) {
		printf("ERROR - WLANConnectionHandler : sceNetApctlConnect returned '%d'.\n", err);
        DisableNetwork();
        iRet =-1;
    }
    
	sceKernelDelayThread(500000);  
	  
    if (NetApctlHandler() == 0)
    {
		iRet = 0;
	}
	else
	{
		iRet = -1;
	}
	
	return iRet;
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

int CPSPApp::NetApctlHandler() 
{
	int iRet = 0;
	u32 state1 = 0;
	u32 err = sceNetApctlGetState(&state1);
	if (err != 0)
	{
		printf("NetApctlHandler: getstate: err=%d state=%d\n", err, state1);
		iRet = -1;
	}
	
	u32 statechange=0;
	u32 ostate=0xffffffff;

	while ((m_Exit == FALSE) && iRet == 0)
	{
		u32 state;
		
		err = sceNetApctlGetState(&state);
		if (err != 0)
		{
			printf("NetApctlHandler: sceNetApctlGetState returns %d\n", err);
			iRet = -1;
			break;
		}
		
		if(statechange > 180) 
		{
			iRet = -1;
			break;
		} 
		else if(state == ostate) 
		{
			statechange++;
		} 
		else 
		{
			statechange=0;
		}
		ostate=state;
		
		sceKernelDelayThread(50000);  /** 50ms */
		
		if (state == apctl_state_IPObtained)
		{
			break;  // connected with static IP
		}
	}

	if((m_Exit == FALSE) && (iRet == 0)) 
	{
		// get IP address
		if (sceNetApctlGetInfo(SCE_NET_APCTL_INFO_IP_ADDRESS, m_strMyIP) != 0)
		{
			strcpy(m_strMyIP, "0.0.0.0");
			printf("NetApctlHandler: Error-could not get IP\n");
			iRet = -1;
		}
		//else
		//{
		//	printf("sceNetApctlGetInfo (SCE_NET_APCTL_INFO_IP_ADDRESS): ipaddr=%s\n",m_strMyIP);
		//}
	}
	
	return iRet;
}

#if 0
/** Overload new/delete */
void operator delete(void *p) { free(p); };
void operator delete[](void *p) { free(p); };
void *operator new(size_t iSize) { return (void*)malloc(iSize); };
void *operator new[](size_t iSize) { return (void *)malloc(iSize); };
#endif
