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
#include <pspusb.h>
#include <pspusbstor.h>

#include "PSPApp.h"
#include "PSPUSBStorage.h"
#include "PSPThread.h"

#undef ReportError

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

int thDriverLoader(SceSize args, void *argp)
{
  pspSdkInstallNoDeviceCheckPatch();
  pspSdkInstallNoPlainModuleCheckPatch();
  pspSdkInstallKernelLoadModulePatch();

	sceKernelSleepThread();
	return 0;
}

CPSPUSBStorage::CPSPUSBStorage(CPSPApp *pspapp)
{
	m_PSPApp = pspapp;
	m_USBEnabled = false;
	
	m_thDriverLoader = new CPSPThread("m_thDriverLoader", thDriverLoader, 
										32, 4*1024, 0/*THREAD_ATTR_KERNEL*/);
	if (m_thDriverLoader)
	{
		m_thDriverLoader->Start();
	}
	sceKernelDelayThread(50*1000); /* Wait 50ms */
	
  LoadStartModule("flash0:/kd/semawm.prx");
	LoadStartModule("flash0:/kd/usbstor.prx");
	LoadStartModule("flash0:/kd/usbstormgr.prx");
	LoadStartModule("flash0:/kd/usbstorms.prx");
	LoadStartModule("flash0:/kd/usbstorboot.prx");
}

CPSPUSBStorage::~CPSPUSBStorage()
{
	if (true == IsUSBEnabled())
	{
		Log(LOG_VERYLOW, "~CPSPUSBStorage(): Disabling USB.");
		DisableUSB();
	}
}

int CPSPUSBStorage::EnableUSB()
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

int CPSPUSBStorage::DisableUSB()
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
			retVal = sceUsbDeactivate(0x1c8);
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
