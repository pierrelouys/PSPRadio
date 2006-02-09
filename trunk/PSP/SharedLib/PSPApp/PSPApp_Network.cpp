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
#include <stdio.h>
#include <pspkernel.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include <pspnet.h>
#include <pspnet_resolver.h>
#include <pspnet_apctl.h>
#include <psputility_netparam.h>
#include <pspnet_inet.h>
#include <pspusb.h>
#include <pspusbstor.h>
#include "PSPApp.h"

#undef ReportError

static int nlhInit();

extern bool ge_WiFiDriversLoaded; /** Defined in Init.o -- Don't remove; necessary for correct linkage of Init.o */

#define SCE_NET_APCTL_INFO_IP_ADDRESS		8
#define apctl_state_IPObtained				4
/** Resolver */
#include <netinet/in.h>

int  CPSPApp::ResolveHostname(char *strHostname, struct in_addr *addr)
{
	/* RC Let's try aton first in case the address is in dotted numerical form */
	Log(LOG_LOWLEVEL, "ResolveHostname: Calling aton.. (host='%s')", strHostname );
	memset(addr, 0, sizeof(in_addr));
	int rc = sceNetInetInetAton(strHostname, addr);
	if (rc == 0)
	{
		/** That didn't work!, it must be a hostname, let's try the resolver... */
		Log(LOG_LOWLEVEL, "ResolveHostname: Calling sceNetResolverStartNtoA() with resolverid = %d",
			 pPSPApp->GetResolverId());
		rc = sceNetResolverStartNtoA(GetResolverId(), strHostname, addr, 2, 3);
	}

	return rc;
}
	
/** End Resolver */

/** Based on Code from VNC for PSP*/
#define      SO_NONBLOCK     0x1009          /* non-blocking I/O */
int ConnectWithTimeout(SOCKET sock, struct sockaddr *addr, int size, size_t timeout/* in s */) 
{
	u32 err = 0;
	int one = 1, zero = 0;
	setsockopt(sock, SOL_SOCKET, SO_NONBLOCK, (char *)&one, sizeof(one));
	
	err = connect(sock, addr, sizeof(struct sockaddr));
	if (err == 0)
	{
		setsockopt(sock, SOL_SOCKET, SO_NONBLOCK, (char *)&zero, sizeof(zero));
		pPSPApp->SendEvent(MID_TCP_CONNECTING_SUCCESS);
		return 0;
	}
	
	if (err == 0xFFFFFFFF && sceNetInetGetErrno() == 0x77)
	{
		size_t ticks;
		for (ticks = 0; ticks < timeout; ticks++) 
		{
			err = connect(sock, addr, sizeof(struct sockaddr));
			if (err == 0 || (err == 0xFFFFFFFF && sceNetInetGetErrno() == 0x7F)) 
			{
				setsockopt(sock, SOL_SOCKET, SO_NONBLOCK, (char *)&zero, sizeof(zero));
				pPSPApp->SendEvent(MID_TCP_CONNECTING_SUCCESS);
				return 0;
			}
			sceKernelDelayThread(1000000); /* 1 s */
			pPSPApp->SendEvent(MID_TCP_CONNECTING_PROGRESS);
		}
	}
	
	//Log(LOG_LOWLEVEL, "Could not connect (Timeout?) geterrno = 0x%x", sceNetInetGetErrno());
	
	setsockopt(sock, SOL_SOCKET, SO_NONBLOCK, (char *)&zero, sizeof(zero));
	pPSPApp->SendEvent(MID_TCP_CONNECTING_FAILED);
	return err;
}

/** Network */
int CPSPApp::EnableNetwork(int profile)
{
	int iRet = 0;
	static bool fnlhInit = false;
	
	/** Don't remove; necessary for correct linkage of Init.o */
	if (ge_WiFiDriversLoaded == false)
	{
		Log(LOG_ERROR, "WiFi drivers not loaded? Maybe running eboot loader?.");
	}

	if (false == fnlhInit)
	{
		int err = nlhInit();
		if (err != 0) 
		{
			ReportError("ERROR - WLANConnectionHandler : nlhInit returned: 0x%x", err);
			DisableNetwork();
			iRet = -1;
		}
		else
		{
			fnlhInit = true;
		}
	}
	if (true == fnlhInit)
	{
		if (WLANConnectionHandler(profile) == 0)
		{
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
		ReportError("Error Initializing Network Drivers\n");
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
	m_NetworkEnabled = false;
}

int CPSPApp::GetNumberOfNetworkProfiles()
{
	int iNumProfiles = 0;
	while (sceUtilityCheckNetParam(iNumProfiles++) == 0)
	{};

	return iNumProfiles - 1;
}

void CPSPApp::GetNetworkProfileName(int iProfile, char *buf, size_t size)
{
	netData data;
	memset(&data, 0, sizeof(netData));
	data.asUint = 0xBADF00D;
	memset(&data.asString[4], 0, 124);
	sceUtilityGetNetParam(iProfile, 0/** 0 = Profile Name*/, &data);
	
	strlcpy(buf, data.asString, size);
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
        iRet =-1;
    }
    
	sceKernelDelayThread(500*1000);  /** 500ms */
	
	if (0 == iRet)  
	{
		/** Let's aquire the IP address */
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
	int state1 = 0;
	int err = sceNetApctlGetState(&state1);
	if (err != 0)
	{
		ReportError("NetApctlHandler: getstate: err=%d state=%d\n", err, state1);
		iRet = -1;
	}
	
	int statechange=0;
	int ostate=0xffffffff;

	while ((false == IsExiting()) && iRet == 0)
	{
		int state;
		
		err = sceNetApctlGetState(&state);
		if (err != 0)
		{
			ReportError("NetApctlHandler: sceNetApctlGetState returns %d\n", err);
			iRet = -1;
			break;
		}
		
		/** Timeout */
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
			break;  /** IP Address Ready */
		}
	}

	if((false == IsExiting()) && (iRet == 0)) 
	{
		/** get IP address */
		if (sceNetApctlGetInfo(SCE_NET_APCTL_INFO_IP_ADDRESS, m_strMyIP) != 0)
		{
			/** Error! */
			strcpy(m_strMyIP, "0.0.0.0");
			ReportError("NetApctlHandler: Error-could not get IP\n");
			iRet = -1;
		}
	}
	
	return iRet;
}

int CPSPApp::EnableUSB()
{
	int retVal = 0;
	int state  = 0;
	
	if (false == m_USBEnabled)
	{
		Log(LOG_INFO, "Starting USB...");
		
		/** setup USB drivers */
		retVal = sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0);
		if (retVal == 0) 
		{
			retVal = sceUsbStart(PSP_USBSTOR_DRIVERNAME, 0, 0);
			if (retVal == 0) 
			{
				retVal = sceUsbstorBootSetCapacity(0x800000);
				if (retVal == 0) 
				{
					retVal = sceUsbActivate(0x1c8);
					
					state = sceUsbGetState();
					if (state & PSP_USB_ACTIVATED != 0)
					{
						Log(LOG_INFO, "USB Activated.");
						m_USBEnabled = true;
						retVal = 0;
					}
					else
					{
						Log(LOG_ERROR, "Error Activating USB\n", retVal);
						retVal = -1;
					}
				}
				else
				{
					Log(LOG_ERROR, "Error setting capacity with USB Mass Storage driver (0x%08X)\n", retVal);
					retVal = -1;
				}
			
			}
			else
			{
				Log(LOG_ERROR, "Error starting USB Mass Storage driver (0x%08X)\n", retVal);
				retVal = -1;
			}
		
		}
		else
		{
			Log(LOG_ERROR, "Error starting USB Bus driver (0x%08X)\n", retVal);
			retVal = -1;
		}
		
	}
	if (retVal == 0)
	{
		pPSPApp->SendEvent(MID_USB_ENABLE);
	}
	return retVal;
}

int CPSPApp::DisableUSB()
{
	int retVal = 0;
	int state = 0;
	
	if (true == m_USBEnabled)
	{
		Log(LOG_INFO, "Stopping USB...");
		
		state = sceUsbGetState();
		if (state & 0x8) /** Busy */
		{
			Log(LOG_ERROR, "USB Busy, cannot disable right now...\n", retVal);
			retVal = -1; //./BUSY
		}
		else
		{
			retVal = sceUsbDeactivate();
			if (retVal != 0)
			{
				Log(LOG_ERROR, "Error calling sceUsbDeactivate (0x%08X)\n", retVal);
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
		
		if (retVal >= 0)
		{
			m_USBEnabled = false;
		}
	}
	if (retVal == 0)
	{
		pPSPApp->SendEvent(MID_USB_DISABLE);
	}
	return retVal;
}

/** This function inializes the Network Drivers.
 *  It needs to be called only once. It has to be called
 *  from a userlevel thread.
 */
int nlhInit()
{
	u32 err = 0;
	
	err = sceNetInit(0x20000, 0x20, 0x1000, 0x20, 0x1000);
	if (err != 0)
	{
		Log(LOG_ERROR, "nlhInit(): sceNetInit returns %i", err);
		return err;
	}
	
	err = sceNetInetInit();
	if (err != 0)
	{
		Log(LOG_ERROR, "nlhInit(): sceNetInetInit returns %i", err);
		return err;
	}

	err = sceNetResolverInit();
	if (err != 0)
	{
		Log(LOG_ERROR, "nlhInit(): sceNetResolverInit returns %i", err);
		return err;
	}
	err = sceNetApctlInit(0x1000, 0x42);
	if (err != 0)
	{
		Log(LOG_ERROR, "nlhInit(): sceNetApctlInit returns %i", err);
		return err;
	}

	return 0;
}
