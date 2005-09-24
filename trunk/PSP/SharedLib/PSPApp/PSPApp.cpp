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

#define Log(level, format, args...) m_Log.Log("CPSPApp", level, format, ## args)

CPSPApp::CPSPApp(char *strProgramName, char *strVersionNumber)
{
	pspDebugScreenInit();
	
	m_thCallbackSetup = new CPSPThread("update_thread", callbacksetupThread, 0x11, 0xFA0, THREAD_ATTR_USER);
	
	memset(&m_pad, 0, sizeof (m_pad));
	pPSPApp = this;
	strcpy(m_strMyIP, "0.0.0.0");
	m_ResolverId = 0;
	
	m_strProgramName = strdup(strProgramName);
	m_strVersionNumber = strdup(strVersionNumber);
	
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

	
	//printf("CPSPApp Constructor.\n");
	
}

CPSPApp::~CPSPApp()
{
	Log(LOG_LOWLEVEL, "Destructor Called.");
	DisableNetwork();
	
	free(m_strProgramName);
	free(m_strVersionNumber);

	Log(LOG_LOWLEVEL, "Bye!.");

	return;
}

int CPSPApp::Run()
{
	short oldAnalogue = 0;
	int   oldButtonMask = 0;
	SceCtrlLatch latch; 
	
	
	while (m_Exit == FALSE)
	{
		sceDisplayWaitVblankStart();
		sceKernelDelayThread(10);
		//sceCtrlReadBufferPositive(&m_pad, 1); 
		
		OnVBlank();
 
		sceCtrlSetSamplingCycle(10); 
		sceCtrlReadLatch(&latch);
		//printf("latch: uiMake=%d; uiBreak=%d; uiPress=%d; uiRelease=%d;\n",
		//	latch.uiMake, latch.uiBreak, latch.uiPress, latch.uiRelease);
		
		if (latch.uiMake)
		{
			/** Button Pressed */
			oldButtonMask = latch.uiPress;
			OnButtonPressed(oldButtonMask);
		}
		else if (latch.uiBreak)
		{
			/** Button Released */
			OnButtonReleased(oldButtonMask);
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
	
	sceKernelDelayThread(50000); /** 50ms */

	sceKernelExitGame();

	return 0;
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
	OnExit();
	sceKernelDelayThread(50000); /** 50ms */
	m_Exit = TRUE;
	return 0;
}

/** Network */
//	int sceNetResolverInit();
//	int sceNetResolverTerm();
//	int sceNetResolverCreate(int *rid, void *buf, SceSize buflen);
// 	int sceNetResolverDelete(int rid);
//	int sceNetResolverStartNtoA(int rid, const char *hostname, struct SceNetInetInAddr *addr, unsigned int timeout, int retry);
//	int sceNetResolverStartAtoN(int rid, const struct SceNetInetInAddr *addr, char *hostname, SceSize hostname_len, unsigned int timeout, int retry);
//	int sceNetResolverStop(int rid);
#include <arpa/inet.h>	
int CPSPApp::EnableNetwork(int profile)
{
	int iRet = 0;
	static BOOLEAN fDriversLoaded = FALSE;
	
	if (fDriversLoaded == FALSE)
	{
		if (nlhLoadDrivers() == 0)
		{
			fDriversLoaded = TRUE;
		}
	}
	
	if (fDriversLoaded == TRUE)
	{
		if (WLANConnectionHandler(profile) == 0)
		{
			//printf("PSP IP = %s\n", GetMyIP());
			
			//sceNetResolverInit();
			int rc = sceNetResolverCreate(&m_ResolverId, m_ResolverBuffer, sizeof(m_ResolverBuffer));
			if (rc < 0)
			{
				printf ("resolvercreate = %d rid = %d\n", rc, m_ResolverId);
				iRet = -1;
			}
			else
			{
				iRet = 0;
				
				/** Test! */
				//printf ("Getting google.com's address...");
				//in_addr addr;
				//rc = sceNetResolverStartNtoA(m_ResolverId, "google.com", &addr, 2, 3);
				//printf ("Got it! '%s' rc=%d\n", inet_ntoa(addr), rc);
			}
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
	
	if (m_ResolverId)
	{
		err = sceNetResolverStop(m_ResolverId);
		err = sceNetResolverDelete(m_ResolverId);
		err = sceNetResolverTerm();
		m_ResolverId = 0;
	}
	
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
int CPSPApp::WLANConnectionHandler(int profile) 
{
    u32 err;
    int iRet = 0;

    err = nlhInit();
    if (err != 0) {
		printf("ERROR - WLANConnectionHandler : nlhInit returned '%d'.\n", err);
        DisableNetwork();
        iRet = -1;
    }

	err = sceNetApctlConnect(profile);
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
