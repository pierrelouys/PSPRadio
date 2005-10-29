/* 
	by PspPet
	Modified by Raf with netbsd socket, etc.
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
////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <pspkernel.h>
#include "PSPNet.h"

////////////////////////////////////////////////////////////////////
// SLIME NOTE: Module library linkage trickery
//  in theory shouldn't be necessary
//  in practice appears to be necessary

// SLIME NOTE: symbols exported from standard 'startup.s'
extern char __lib_stub_top[], __lib_stub_bottom[];

static u32 FindProcEntry(u32 oid, u32 nid)
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

static int PatchMyLibraryEntries(SceModuleInfo * modInfoPtr, u32 oid)
{
    int nPatched = 0;

    int* stubPtr; // 20 byte structure
    for (stubPtr = modInfoPtr->stub_top;
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

static void FlushCaches()
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
    	printf("nlhInit(): sceNetInit returns %i\n", err);
        return err;
    }
	
	err = sceNetInetInit();
	if (err != 0)
	{
		printf("nlhInit(): sceNetInetInit returns %i\n", err);
        return err;
    }

	err = sceNetResolverInit();
    if (err != 0)
    {
		printf("nlhInit(): sceNetResolverInit returns %i\n", err);
        return err;
	}
	err = sceNetApctlInit(0x1000, 0x42);
    if (err != 0)
    {
		printf("nlhInit(): sceNetApctlInit returns %i\n", err);
        return err;
    }

    return 0;
}

int nlhTerm()
{
    u32 err = 0;
	
	err = sceNetApctlTerm();
	if (err & 0x80000000)
	{
		printf("sceNetApctlTerm returns %i\n", err);
	}

	err = sceNetResolverTerm();
	if (err & 0x80000000)
	{
		printf("sceNetResolverTerm returns %i\n", err);
	}

	err = sceNetInetTerm();
	if (err & 0x80000000)
	{
		printf("sceNetInetTerm returns %i\n", err);
	}

	err = sceNetTerm();
	if (err & 0x80000000)
	{
		printf("sceNetTerm returns %i\n", err);
	}
	
    return 0;
}


/**
 * inet_ntoa
 * By Raf
 *
 * This implementation is not thread safe.
 * Be sure to protect the call using semaphores/mutexes
 * If working with multiple threads.
 */
char *sceNetInetInetNtoa(struct in_addr in)
{
  static char buf[256]; 
  struct in_addr_b *inb = (struct in_addr_b*)&in;
  sprintf(buf, "%d.%d.%d.%d", inb->b1, inb->b2, inb->b3, inb->b4);
  return buf;
}
////////////////////////////////////////////////////////////////////
