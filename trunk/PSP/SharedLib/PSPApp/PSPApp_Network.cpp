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
#include <pspsdk.h>
#include <pspkernel.h>
#include <errno.h>
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
#include <sys/select.h>
#include <sys/fcntl.h>

#include <netinet/in.h>

#include <psputility_netmodules.h>
#include <psputility_netparam.h>
#include <pspwlan.h>
#include <pspnet.h>
#include <pspnet_apctl.h>

#include "PSPApp.h"

#undef ReportError

#define SCE_NET_APCTL_INFO_IP_ADDRESS		8
#define apctl_state_IPObtained				4

int CPSPApp::ResolveHostname(char *strHostname, struct in_addr *addr)
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
    
    memset(addr, 0, sizeof(in_addr));
		rc = sceNetResolverStartNtoA(GetResolverId(), strHostname, addr, 2, 3);

    Log(LOG_LOWLEVEL, "ResolveHostname: rc=%d. '%s' resolved to '%s'", rc, strHostname, inet_ntoa(*addr));
	}

	return rc;
}
	
/*
    Connect with timeout  From: 
    VTun - Virtual Tunnel over TCP/IP network.

    Copyright (C) 1998-2000  Maxim Krasnyansky <max_mk@yahoo.com>

    VTun has been derived from VPPP package by Maxim Krasnyansky.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/
int connect_t(int s, struct sockaddr *svr, time_t timeout)
{
	int sock_flags;
	fd_set fdset;
	struct timeval tv;
	
	tv.tv_usec=0; tv.tv_sec=timeout;
	
	sock_flags=fcntl(s,F_GETFL);
	if( fcntl(s,F_SETFL,O_NONBLOCK) < 0 )
	{
		pPSPApp->SendEvent(MID_TCP_CONNECTING_FAILED);
		Log(LOG_ERROR, "connect_t(): Error setting socket mode to non-blocking: %s", (errno < 139)?strerror(errno):"Unknown SCE error");
		return -1;
	}
	
	if( connect(s,svr,sizeof(struct sockaddr)) < 0 && errno != EINPROGRESS)
	{
		pPSPApp->SendEvent(MID_TCP_CONNECTING_FAILED);
		Log(LOG_ERROR, "connect_t(): Error connecting. %s", (errno < 139)?strerror(errno):"Unknown SCE error");
		return -1;
	}
	
	FD_ZERO(&fdset);
	FD_SET(s,&fdset);
	if( select(s+1,NULL,&fdset,NULL,timeout?&tv:NULL) > 0 )
	{
		socklen_t l=sizeof(errno);
		errno=0;
		getsockopt(s,SOL_SOCKET,SO_ERROR,&errno,&l);
	} 
	else
		errno=ETIMEDOUT;
	
	fcntl(s,F_SETFL,sock_flags);

	if( errno )
	{
		pPSPApp->SendEvent(MID_TCP_CONNECTING_FAILED);
		Log(LOG_ERROR, "connect_t(): Error connecting. %s", (errno < 139)?strerror(errno):"Unknown SCE error");
		return -1;
	}

	pPSPApp->SendEvent(MID_TCP_CONNECTING_SUCCESS);
	return 0;
}

/** Based on Code from VNC for PSP*/
int ConnectWithTimeout(SOCKET sock, struct sockaddr *addr, size_t timeout/* in s */) 
{
	#if 0
	return connect_t(sock, addr, timeout);
	
	#else
	int err = 0;
	int one = 1, zero = 0;
	
	setsockopt(sock, SOL_SOCKET, SO_NONBLOCK, (char *)&one, sizeof(one));
	
	err = connect(sock, addr, sizeof(struct sockaddr));
	if (err == 0 /* No error - connected */)
	{
		setsockopt(sock, SOL_SOCKET, SO_NONBLOCK, (char *)&zero, sizeof(zero));
		pPSPApp->SendEvent(MID_TCP_CONNECTING_SUCCESS);
		return 0;
	}
	
	if (err == -1 /* Not connected */ && errno == EINPROGRESS)
	{
		size_t ticks;
		for (ticks = 0; ticks < timeout; ticks++) 
		{
			err = connect(sock, addr, sizeof(struct sockaddr));
			if (err == 0 || (err == -1 && errno == EISCONN)) 
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
	Log(LOG_ERROR, "ConnectWithTimeout(): Error connecting. %s", (errno < 139)?strerror(errno):"Unknown SCE error");
	return errno;
	#endif
}

int CPSPApp::InitializeNetworkDrivers()
{
  int iRet = 0;
  int err;

	Log(LOG_LOWLEVEL, "Loading Network Drivers...");
  err = sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
  if (err != 0)
  {
    Log(LOG_ERROR, "Error Loading Network Modules --sceUtilityLoadNetModule()=0x%04x\n", err);
    iRet = -1;
  }
  
  err = sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
  if (err != 0)
  {
    Log(LOG_ERROR, "Error Loading Network Modules -- sceUtilityLoadNetModule()=0x%04x\n", err);
    iRet = -1;
  }
  
  err = pspSdkInetInit();
  if (err != 0)
  {
    Log(LOG_ERROR, "Error Initializing Network Drivers -- err=0x%04x\n", err);
    iRet = -1;
  }
	else
	{
		Log(LOG_INFO, "Network Drivers Initialized Correctly.");
		iRet = 0;
	}
	  
  if (m_ResolverId == 0)
  {
    memset(m_ResolverBuffer, 0, sizeof(m_ResolverBuffer));
    int rc = sceNetResolverCreate(&m_ResolverId, m_ResolverBuffer, sizeof(m_ResolverBuffer));
    if (rc < 0)
    {
      Log(LOG_ERROR, "InitializeNetworkDrivers, Resolvercreate = 0x%0x rid = %d\n", rc, m_ResolverId);
      iRet = -1;
    }
  }

  return 0;
}

int CPSPApp::StopNetworkDrivers()
{
	pspSdkInetTerm();
	sceNetApctlDisconnect();
	sceNetApctlTerm();
	
	return 0;
}

int CPSPApp::EnableNetwork(int profile)
{
	int iRet = 0;
	if (sceNetApctlGetInfo(SCE_NET_APCTL_INFO_IP_ADDRESS, m_strMyIP) != 0)
	{
		if (WLANConnectionHandler(profile) == 0)
		{
			iRet = 0;
			
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
		Log(LOG_ERROR, "IP already assigned: '%s'.", m_strMyIP);
		m_NetworkEnabled = true;
		iRet = 0;
	}
	return iRet;
}

void CPSPApp::DisableNetwork()
{
	u32 err = 0;
	
	if (true == IsNetworkEnabled())
	{
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
	int iNumProfiles = 0, blank_found = 0;
	for( int i = 0; i < 100; i++)
	{
		/** Updated to not stop immediately when a non-zero value is found -- idea by danzel (forums.ps2dev.org/viewtopic.php?t=5349) */
		if (sceUtilityCheckNetParam(i) == 0)
		{
			iNumProfiles++;
		}
		else
		{
			/** If we found 5 invalid ones in a row, we assume we're done, and stop enumeration */
			blank_found++;
			if (blank_found > 5)
				break;
		}
	}

	return iNumProfiles;
}

void CPSPApp::GetNetworkProfileName(int iProfile, char *buf, size_t size)
{
	netData data;
	memset(&data, 0, sizeof(netData));
	data.asUint = 0xBADF00D;
	memset(&data.asString[4], 0, 124);
	if (sceUtilityCheckNetParam(iProfile) == 0)
	{
		sceUtilityGetNetParam(iProfile, 0/** 0 = Profile Name*/, &data);
		strlcpy(buf, data.asString, size);
	}
	else
	{
		buf[0] = 0;
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

