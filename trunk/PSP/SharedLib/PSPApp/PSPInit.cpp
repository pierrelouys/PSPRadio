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
#ifdef DEBUG
	#include <pspdebug.h>
#endif
#include <pspdisplay.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include <pspkernel.h>
#include <psphprm.h>
#include <psppower.h>
#include <pspnet.h>
#include <pspnet_resolver.h>
#include <pspnet_apctl.h>
#include <psputility_netparam.h>
#include <pspnet_inet.h>
#include "PSPApp.h"

#define EXIT_CALLBACK 

extern SceModuleInfo module_info;

int callbacksetupThread(SceSize args, void *argp);
int DriverLoadThread(SceSize args, void *argp);
int exitCallback(int arg1, int arg2, void *common);
int powerCallback(int arg1, int pwrflags, void *common);
u32 LoadStartModule(char *path);
/*--*/
static u32 FindProcEntry(u32 oid, u32 nid);
static int PatchMyLibraryEntries(SceModuleInfo * modInfoPtr, u32 oid);
static u32 LoadAndStartAndPatch(SceModuleInfo * modInfoPtr, const char* szFile);
static void FlushCaches();
int nlhLoadDrivers(SceModuleInfo * modInfoPtr);

/** Driver Loader Thread handle */
CPSPThread *thDriverLoader = NULL;
/** System callbacks thread */
CPSPThread *thCallbackSetup = NULL;
/** For Debug */
#ifdef DEBUG
	volatile bool flagGdbStubReady = false;
#endif

/** These variables are accessed externally, this allows for the Init.o module from linking correctly into
the final application -- Don't remove ! */
bool ge_USBDriversLoaded = false;
bool ge_WiFiDriversLoaded = false;

/**
 * Function that is called from _init in kernelmode before the
 * main thread is started in usermode.
 * We start a thread that will load the drivers here. As the main
 * app runs in usermode.
 * (This based on findings by PspPet and benji)
 */
__attribute__ ((constructor))
void loaderInit()
{
	
	#ifdef DEBUG
		pspDebugScreenInit();
		pspDebugScreenPrintf("loaderInit(): Starting DriverLoader Thread...\n");
	#endif
	
	pspKernelSetKernelPC();

	CPSPThread *thDriverLoader = new CPSPThread("driverloader_thread", DriverLoadThread, 0x11, 0xFA0, 0, 0);
	if (thDriverLoader)
	{
		thDriverLoader->Start();
	}
	
	CPSPThread *thCallbackSetup = new CPSPThread("update_thread", callbacksetupThread, 
												100, 1024, 0,0);//THREAD_ATTR_USER);
	if (thCallbackSetup)
	{
		thCallbackSetup->Start();
	}
}
	
int callbacksetupThread(SceSize args, void *argp)
{
	int cbid;
	#ifdef EXIT_CALLBACK
	cbid = sceKernelCreateCallback("Exit Callback", exitCallback, NULL);
	sceKernelRegisterExitCallback(cbid);
	#endif
    cbid = sceKernelCreateCallback("Power Callback", powerCallback, NULL);
    scePowerRegisterCallback(0, cbid);

	sceKernelSleepThreadCB();

	return 0;
}

/**** USB -- Based on USB Sample v1.0 by John_K - Based off work by PSPPet */
/** helper function to make things easier */
u32 LoadStartModule(char *path)
{
    u32 loadResult;
    u32 startResult;
    int status;
	u32 iRet = 0;

    loadResult = sceKernelLoadModule(path, 0, NULL);
	
    if (0 == (loadResult & 0x80000000))
	{
		startResult = sceKernelStartModule(loadResult, 0, NULL, &status, NULL);
		if (loadResult == startResult)
		{
			/** Success */
			iRet = 0;
		}
		else
		{
			iRet = startResult;
		}
	}
	else
	{
		iRet = loadResult;
	}
	
	
    return iRet;
}

/** This thread runs in Kernel Mode. All it does
 *  is load and start drivers 
 */
int DriverLoadThread(SceSize args, void *argp)
{
	/** Load and start Network drivers */
	nlhLoadDrivers(&module_info);
	ge_WiFiDriversLoaded = true;

	/** Load and start USB drivers */
	LoadStartModule("flash0:/kd/semawm.prx");
	LoadStartModule("flash0:/kd/usbstor.prx");
	LoadStartModule("flash0:/kd/usbstormgr.prx");
	LoadStartModule("flash0:/kd/usbstorms.prx");
	LoadStartModule("flash0:/kd/usbstorboot.prx");
	ge_USBDriversLoaded = true;
	
	#ifdef DEBUG
		pspDebugScreenPrintf("Initialising GDB stub...\n");
		pspDebugGdbStubInit();
	    flagGdbStubReady = true;
		pspDebugScreenPrintf("Ready.\n");
	#endif

	sceKernelSleepThreadCB();

	return 0;
}

/* System Callbacks */
int exitCallback(int arg1, int arg2, void *common)
{
	return pPSPApp->OnAppExit(arg1, arg2, common);
}

int powerCallback(int arg1, int pwrflags, void *common)
{
	pPSPApp->OnPowerEvent(pwrflags);

	/** Register again (or it won't happen anymore) */
    int cbid = sceKernelCreateCallback("Power Callback", powerCallback, NULL);
    scePowerRegisterCallback(0, cbid);
	return 0;
}


/*-----------------*/
//From PSPNEt.h
////////////////////////////////////////////////////////////////////
extern char __lib_stub_top[], __lib_stub_bottom[];

u32 FindProcEntry(u32 oid, u32 nid)
{
	typedef u32* (*MAP_PROC)(u32);
	MAP_PROC pfnMap;
	u32* modPtr;

	u32* addr = (u32*)0x88014318;
	if (*addr != 0x27bdffd0)
	{
		addr = (u32*)0x88017308;
		if (*addr != 0x27bdffd0)
		{
			printf("ERROR: version error (find)!\n");
			return 0;   // something terribly wrong
		}
	}
	pfnMap = (MAP_PROC)addr;
	modPtr = (*pfnMap)(oid);

	if ((((long)modPtr) & 0xFF000000) != 0x88000000)
		return 0;
	if ((modPtr[18] - modPtr[16]) < 40)
		return 0;

	/* assume standard library order */
	{
		u32* modPtr2 = (u32*)modPtr[16];
		int count = (modPtr2[6] >> 16) & 0xFFFF;
		u32* idPtr = (u32*)modPtr2[7];
		u32* procAddrPtr = idPtr + count;
		int i;
	
		for (i = 0; i < count; i++)
		{
			if (*idPtr == nid)
				return (*procAddrPtr);
			idPtr++;
			procAddrPtr++;
		}
	}
	return 0;
}

int PatchMyLibraryEntries(SceModuleInfo * modInfoPtr, u32 oid)
{
	int nPatched = 0;

	int* stubPtr; // 20 byte structure
	for (stubPtr = (int*)modInfoPtr->stub_top;
	 stubPtr + 5 <= (int*)modInfoPtr->stub_end;
	  stubPtr += 5)
	{
		int count = (stubPtr[2] >> 16) & 0xFFFF;
		int* idPtr = (int*)stubPtr[3];
		int* procPtr = (int*)stubPtr[4];

		if (stubPtr[1] != 0x90000)
			continue;   // skip non-lazy loaded modules
		while (count--)
		{
			if (procPtr[0] == 0x54C && procPtr[1] == 0)
			{
				// SWI - scan for NID
				u32 proc = FindProcEntry(oid, *idPtr);
				if (proc != 0)
				{
					if (((u32)procPtr & 0xF0000000) != (proc & 0xF0000000))
					{
						// if not in user space we can't use it
						printf("!! NOT PATCH\n");
					}
					else
					{
						u32 val = (proc & 0x03FFFFFF) >> 2;
						procPtr[0] = 0x0A000000 | val;
						procPtr[1] = 0;
						
						nPatched++;
					}
				}
			}
			idPtr++;
			procPtr += 2; // 2 opcodes
		}
	}
	return nPatched;
}

/* return oid or error code */
u32 LoadAndStartAndPatch(SceModuleInfo * modInfoPtr, const char* szFile)
{
	u32 oid;

	oid = sceKernelLoadModule(szFile, 0, NULL);

	if (oid == 0x80020146)
	{
		printf("Not allowed to load module!");
	}
	if (oid & 0x80000000)
	{
		printf("Error Loading Module: %s = %i\n", szFile, oid);
		return oid; // error code
	}

	/* Start it */
	u32 err;
	s32 fake = 0;
	err = sceKernelStartModule(oid, 0, 0, &fake, 0);

	if (err != oid)
	{
		printf(" -- DID NOT START\n");
		return err;
	}

	PatchMyLibraryEntries(modInfoPtr, oid);
	
	return oid;
}

void FlushCaches()
{
	typedef void (*VOID_PROC)(void);
	VOID_PROC pfnFlush;
	u32* addr = (u32*)0x88054618;
	if (*addr != 0x40088000)
	{
		addr = (u32*)0x880584f0;
		if (*addr != 0x40088000)
		{
			printf("ERROR: version error (flush)!\n");
			return;   // something terribly wrong
		}
	}
	pfnFlush = (VOID_PROC)addr;

	sceKernelDcacheWritebackAll();
	(*pfnFlush)();
}

/** This function loads and starts the network drivers.
 *  It is called from a kernel level thread on startup.
 */
int nlhLoadDrivers(SceModuleInfo * modInfoPtr)
{
	LoadAndStartAndPatch(modInfoPtr, "flash0:/kd/ifhandle.prx"); // kernel
	LoadAndStartAndPatch(modInfoPtr, "flash0:/kd/pspnet.prx");
	LoadAndStartAndPatch(modInfoPtr, "flash0:/kd/pspnet_inet.prx");
	LoadAndStartAndPatch(modInfoPtr, "flash0:/kd/pspnet_apctl.prx");
	LoadAndStartAndPatch(modInfoPtr, "flash0:/kd/pspnet_resolver.prx");

	// jumps have been patched - flush DCache and ICache
	FlushCaches();

	return 0;
}
