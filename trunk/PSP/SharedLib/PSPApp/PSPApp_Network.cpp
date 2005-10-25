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
#include <psppower.h>
#include "PSPApp.h"

#undef ReportError
extern SceModuleInfo module_info;

/** Network */
int CPSPApp::EnableNetwork(int profile)
{
	int iRet = 0;
	static bool fnlhInit = false;
	
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
	if (fnlhInit)
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

int NetworkAndUSBDriverLoadThread(SceSize args, void *argp)
{
	nlhLoadDrivers(&module_info);

	//start USB necessary drivers
	LoadStartModule("flash0:/kd/semawm.prx");
	LoadStartModule("flash0:/kd/usbstor.prx");
	LoadStartModule("flash0:/kd/usbstormgr.prx");
	LoadStartModule("flash0:/kd/usbstorms.prx");
	LoadStartModule("flash0:/kd/usbstorboot.prx");

    sceKernelSleepThreadCB();

	return 0;
}

/**
 * Function that is called from _init in kernelmode before the
 * main thread is started in usermode.
 */
__attribute__ ((constructor))
void loaderInit()
{
    pspKernelSetKernelPC();
    int thid = 0;

    thid = sceKernelCreateThread("network_thread", NetworkAndUSBDriverLoadThread,
				 0x11, 0xFA0, 0, 0);
    if (thid >= 0) 
	{
		sceKernelStartThread(thid, 0, 0);
    }
}


