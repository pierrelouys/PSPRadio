/* 
	PSPApp C++ OO Application Framework. (Initial Release: Sept. 2005)
	Copyright (C) 2005  Rafael Cabezas a.k.a. Raf
	
	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include <new>
#include <pspdisplay.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include <pspnet.h>
#include <psphprm.h>
#include "PSPApp.h"

#undef ReportError

PSP_MODULE_INFO("PSPAPP", 0x1000, 1, 1);

class CPSPApp *pPSPApp = NULL; /** Do not access / Internal Use. */

CPSPApp::CPSPApp(char *strProgramName, char *strVersionNumber)
{
	m_Exit = false;
	m_NetworkEnabled = false;
	m_USBEnabled = false;
	m_ExitSema = NULL;
	m_EventToPSPApp = NULL;
	m_thCallbackSetup = NULL; /** Callback thread */
	m_thRun = NULL; /** Run Thread */
	memset(&m_pad, 0, sizeof (SceCtrlData));
	strcpy(m_strMyIP, "0.0.0.0");
	m_ResolverId = 0;
	m_strProgramName = strdup(strProgramName);
	m_strVersionNumber = strdup(strVersionNumber);
	pPSPApp = this;
	m_Polling = false;
	
	m_thCallbackSetup = new CPSPThread("update_thread", callbacksetupThread, 100, 1024, THREAD_ATTR_USER);
	m_ExitSema = new CSema("PSPApp_Exit_Sema");

	if (m_thCallbackSetup)
	{
		m_thCallbackSetup->Start();
		
		sceCtrlSetSamplingCycle(0);
		sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
		
		m_Exit = false;
	}
	else /** Oops, error, let's exit the app */
	{
		m_Exit = true;
	}
	
	m_EventToPSPApp = new CPSPEventQ("msg_to_pspapp_q");

	
}

int CPSPApp::StartPolling()
{
	if (NULL == m_thRun)
	{
		m_thRun = new CPSPThread("run_thread", runThread, 80, 80000);
		
		/** Start Polling for Vblank and buttons */
		m_thRun->Start();
	}
	m_Polling = true;
	
	return 0;
}

int CPSPApp::StopPolling()
{
	m_Polling = false;
	sceKernelDelayThread(1000*50); /* Wait 50ms */
	return 0;
}

CPSPApp::~CPSPApp()
{
	Log(LOG_VERYLOW, "Destructor Called.");
	
	if (true == IsUSBEnabled())
	{
		Log(LOG_VERYLOW, "Disabling USB.");
		DisableUSB();
	}
	
	if (true == IsNetworkEnabled())
	{
		Log(LOG_VERYLOW, "Disabling Network.");
		DisableNetwork();
	}
	
	if (m_ExitSema)
	{
		Log(LOG_VERYLOW, "deleting exitsema");
		delete m_ExitSema;
	}

	if (m_EventToPSPApp)
	{
		Log(LOG_VERYLOW, "deleting eventtopspapp");
		delete(m_EventToPSPApp), m_EventToPSPApp = NULL;
	}

	Log(LOG_VERYLOW, "freeing program name.");
	free(m_strProgramName);
	Log(LOG_VERYLOW, "freeing version number.");
	free(m_strVersionNumber);
	
	Log(LOG_INFO, "Bye!.");
	
	sceKernelExitGame();

	return;
}

/** This is a thread */
int CPSPApp::Run()
{
	short oldAnalogue = 0;
	int   oldButtonMask = 0;
	SceCtrlLatch latch; 
	u32 hprmlatch = 0;
	
	Log(LOG_INFO, "Run(): Going into main loop.");
	
	sceCtrlSetSamplingCycle(10); 
	while (false == m_Exit)
	{
		sceDisplayWaitVblankStart();
		sceKernelDelayThread(10);
		//sceCtrlReadBufferPositive(&m_pad, 1); 
		
		if (false == m_Polling)
			continue;
		
		OnVBlank();
		//SendEvent(MID_ONVBLANK);
 
		sceCtrlReadLatch(&latch);
		//printf("latch: uiMake=%d; uiBreak=%d; uiPress=%d; uiRelease=%d;\n",
		//	latch.uiMake, latch.uiBreak, latch.uiPress, latch.uiRelease);
		
		if (latch.uiMake)
		{
			/** Button Pressed */
			oldButtonMask = latch.uiPress;
			//OnButtonPressed(oldButtonMask);
			SendEvent(MID_ONBUTTON_PRESSED, &oldButtonMask);
		}
		else if (latch.uiBreak)
		{
			/** Button Released */
			//OnButtonReleased(oldButtonMask);
			SendEvent(MID_ONBUTTON_RELEASED, &oldButtonMask);
		}
		
		if (oldAnalogue != (short)m_pad.Lx)
		{
			//pspDebugScreenSetXY(0,10);
			//printf ("Analog Lx=%03d Ly=%03d     ", m_pad.Lx, m_pad.Ly);
			//pspDebugScreenSetXY(0,5);
			oldAnalogue = (short)m_pad.Lx;
			OnAnalogueStickChange(m_pad.Lx, m_pad.Ly);
		}

		if (sceHprmIsRemoteExist())
		{
			sceHprmReadLatch(&hprmlatch);
			if (hprmlatch != 0x00)
				{
				SendEvent(MID_ONHPRM_RELEASED, &hprmlatch);
				Log(LOG_VERYLOW, "HPRM latch = %04x\n", hprmlatch);
				}
		}
	}
	
	Log(LOG_VERYLOW, "Run:: Wait()");
	m_ExitSema->Wait();
	Log(LOG_VERYLOW, "Run:: Calling OnExit().");
	OnExit();
	SendEvent(MID_PSPAPP_EXITING);

	Log(LOG_VERYLOW, "Run:: Right before calling sceKernelExitThread.");
	sceKernelExitThread(0);
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

/** Note: OnAppExit is executed from the exit callback thread, which is running in USER MODE */
int CPSPApp::OnAppExit(int arg1, int arg2, void *common)
{

	m_Exit = true;
	
	return 0;
}

/** Network */
//#include <arpa/inet.h>	
int CPSPApp::EnableNetwork(int profile)
{
	int iRet = 0;
	static bool fDriversLoaded = false, fnlhInit = false;
	
	if (false == fDriversLoaded)
	{
		if (0 == nlhLoadDrivers(&module_info))
		{
			fDriversLoaded = true;
		}
	}
	if (false == fnlhInit)
	{
		int err = nlhInit();
		if (err != 0) 
		{
			ReportError("ERROR - WLANConnectionHandler : nlhInit returned '0x%x'.\n", err);
			DisableNetwork();
			iRet = -1;
		}
		else
		{
			fnlhInit = true;
		}
	}
	if (fDriversLoaded && fnlhInit)
	{
		if (WLANConnectionHandler(profile) == 0)
		{
			//printf("PSP IP = %s\n", GetMyIP());
			
			//sceNetResolverInit();
			iRet = 0;
			if (0 == m_ResolverId)
			{
				int rc = sceNetResolverCreate(&m_ResolverId, m_ResolverBuffer, sizeof(m_ResolverBuffer));
				if (rc < 0)
				{
					Log(LOG_LOWLEVEL, "EnableNetwork, Resolvercreate = 0x%0x rid = %d\n", rc, m_ResolverId);
					iRet = -1;
				}
			}
			
			if (0 == iRet)
			{
				m_NetworkEnabled = true;
				/** Test! */
				//printf ("Getting google.com's address...");
				//in_addr addr;
				//rc = sceNetResolverStartNtoA(m_ResolverId, "google.com", &addr, 2, 3);
				//printf ("Got it! '%s' rc=%d\n", inet_ntoa(addr), rc);
			}
		}
		else
		{
			ReportError("Error starting network\n");
			iRet = -1;
		}
	}
	else
	{
		ReportError("Error loading network drivers\n");
		iRet = -1;
	}
	return iRet;
}

void CPSPApp::DisableNetwork()
{
	u32 err = 0;
	
	if (true == IsNetworkEnabled())
	{
		if (0)//m_ResolverId)
		{
			err = sceNetResolverStop(m_ResolverId);
			err = sceNetResolverDelete(m_ResolverId);
			err = sceNetResolverTerm();
			m_ResolverId = 0;
		}
		
		err = sceNetApctlDisconnect();
		if (err != 0) 
		{
			ReportError("ERROR - DisableNetwork: sceNetApctlDisconnect returned '0x%x'.\n", err);
	    }
	
		/*
	    err = nlhTerm();
		if (err != 0) 
		{
			ReportError("ERROR - DisableNetwork: nlhTerm returned '0x%x'.\n", err);
	    }
		*/
    }
    else
    {
	    Log(LOG_ERROR, "DisableNetwork() Called, but networking was not enabled. Ignoring.");
    }
}

/** From FTPD */
int CPSPApp::WLANConnectionHandler(int profile) 
{
    u32 err;
    int iRet = 0;

	err = sceNetApctlConnect(profile);
    if (err != 0) 
	{
		ReportError("ERROR - WLANConnectionHandler : sceNetApctlConnect returned '0x%x'.\n", err);
        //DisableNetwork();
        iRet =-1;
    }
    
	sceKernelDelayThread(500000);  
	
	if (0 == iRet)  
	{
		if (NetApctlHandler() == 0)
		{
			iRet = 0;
		}
		else
		{
			iRet = -1;
		}
	}
		
	return iRet;
}

int CPSPApp::ReportError(char *format, ...)
{
	char *strMessage;
	va_list args;

	strMessage   = (char *)malloc(4096);

	va_start (args, format);         /* Initialize the argument list. */
	
	vsprintf(strMessage, format, args);
	
	va_end (args);
	
	return SendEvent(MID_ERROR, strMessage);
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

int CPSPApp::runThread(SceSize args, void *argp) 
{
	return pPSPApp->Run();
}

int CPSPApp::NetApctlHandler() 
{
	int iRet = 0;
	u32 state1 = 0;
	u32 err = sceNetApctlGetState(&state1);
	if (err != 0)
	{
		ReportError("NetApctlHandler: getstate: err=%d state=%d\n", err, state1);
		iRet = -1;
	}
	
	u32 statechange=0;
	u32 ostate=0xffffffff;

	while ((false == m_Exit) && iRet == 0)
	{
		u32 state;
		
		err = sceNetApctlGetState(&state);
		if (err != 0)
		{
			ReportError("NetApctlHandler: sceNetApctlGetState returns %d\n", err);
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

	if((false == m_Exit) && (iRet == 0)) 
	{
		// get IP address
		if (sceNetApctlGetInfo(SCE_NET_APCTL_INFO_IP_ADDRESS, m_strMyIP) != 0)
		{
			strcpy(m_strMyIP, "0.0.0.0");
			ReportError("NetApctlHandler: Error-could not get IP\n");
			iRet = -1;
		}
		//else
		//{
		//	printf("sceNetApctlGetInfo (SCE_NET_APCTL_INFO_IP_ADDRESS): ipaddr=%s\n",m_strMyIP);
		//}
	}
	
	return iRet;
}

//helper function to make things easier
int LoadStartModule(char *path)
{
    u32 loadResult;
    u32 startResult;
    int status;

    loadResult = sceKernelLoadModule(path, 0, NULL);
    if (loadResult & 0x80000000)
	return -1;
    else
	startResult =
	    sceKernelStartModule(loadResult, 0, NULL, &status, NULL);

    if (loadResult != startResult)
	return -2;

    return 0;
}

//Based on USB Sample v1.0 by John_K - Based off work by PSPPet
#include <pspusb.h>
#include <pspusbstor.h>
int  CPSPApp::EnableUSB()
{
	int retVal;
	int state = 0;
	
	Log(LOG_INFO, "Starting USB...");
	
	//start necessary drivers
	LoadStartModule("flash0:/kd/semawm.prx");
	LoadStartModule("flash0:/kd/usbstor.prx");
	LoadStartModule("flash0:/kd/usbstormgr.prx");
	LoadStartModule("flash0:/kd/usbstorms.prx");
	LoadStartModule("flash0:/kd/usbstorboot.prx");
	
	//setup USB drivers
	retVal = sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0);
	if (retVal != 0) 
	{
		Log(LOG_ERROR, "Error starting USB Bus driver (0x%08X)\n", retVal);
		return retVal;
	}
	retVal = sceUsbStart(PSP_USBSTOR_DRIVERNAME, 0, 0);
	if (retVal != 0) 
	{
		Log(LOG_ERROR, "Error starting USB Mass Storage driver (0x%08X)\n", retVal);
		return retVal;
	}
	retVal = sceUsbstorBootSetCapacity(0x800000);
	if (retVal != 0) 
	{
		Log(LOG_ERROR, "Error setting capacity with USB Mass Storage driver (0x%08X)\n", retVal);
		return retVal;
	}
	
	retVal = sceUsbActivate(0x1c8);
	state = sceUsbGetState();
	if (state & PSP_USB_ACTIVATED == 0) //PSP_USB_CABLE_CONNECTED , PSP_USB_CONNECTION_ESTABLISHED
	{
		Log(LOG_ERROR, "Error Activating USB\n", retVal);
		return retVal;
	}
	
	m_USBEnabled = true;
	
	return retVal;
}

int CPSPApp::DisableUSB()
{
	int retVal = 0;
	int state = 0;
	
	if (m_USBEnabled)
	{
		Log(LOG_INFO, "Stopping USB...");
		
		state = sceUsbGetState();
		//if (state & PSP_USB_ACTIVATED)
		{
			retVal = sceUsbDeactivate();
			if (retVal != 0)
			{
				Log(LOG_ERROR, "Error calling sceUsbDeactivate (0x%08X)\n", retVal);
			}
		}
		retVal = sceUsbStop(PSP_USBSTOR_DRIVERNAME, 0, 0);
		if (retVal != 0)
		{
			Log(LOG_ERROR, "Error stopping USB Mass Storage driver (0x%08X)\n", retVal);
		}
		retVal = sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0);
		if (retVal != 0)
		{
			Log(LOG_ERROR, "Error stopping USB BUS driver (0x%08X)\n", retVal);
		}
	}
	
	return retVal;
}
