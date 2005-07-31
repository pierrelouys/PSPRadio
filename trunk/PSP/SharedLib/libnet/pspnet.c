////////////////////////////////////////////////////////////////////
// Must be linked with KMEM memory access

#include <pspkernel.h>
#include <pspdebug.h>
#include "pspnet.h"
#define LogPrintf	pspDebugScreenPrintf


////////////////////////////////////////////////////////////////////
// SLIME NOTE: Module library linkage trickery
//  in theory shouldn't be necessary
//  in practice appears to be necessary

// SLIME NOTE: symbols exported from standard 'startup.s'
extern char __lib_stub_top[], __lib_stub_bottom[];

// SYSTEM ENTRY: sceKernelModuleFromUID
// 1.0 ->	88014318:	27bdffd0	addiu sp,sp,#0xffffffd0
// 1.50 ->	88017308:	27bdffd0	addiu sp,sp,#0xffffffd0

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
            LogPrintf("ERROR: version error (find)!\n");
            return 0;   // something terribly wrong
        }
    }
    pfnMap = (MAP_PROC)addr;
	modPtr = (*pfnMap)(oid);

    if ((((long)modPtr) & 0xFF000000) != 0x88000000)
        return 0;
	if ((modPtr[18] - modPtr[16]) < 40)
        return 0;

	// assume standard library order
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

static int PatchMyLibraryEntries(u32 oid)
{
	//REVIEW: should match single module name
    // this version is dumb and walks all of them (assumes NIDs are unique)

    int nPatched = 0;

    int* stubPtr; // 20 byte structure
    for (stubPtr = (int*)__lib_stub_top;
     stubPtr + 5 <= (int*)__lib_stub_bottom;
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
		                LogPrintf("!! NOT PATCH\n");
	                }
	                else
	                {
	                    u32 val = (proc & 0x03FFFFFF) >> 2;
	                    procPtr[0] = 0x0A000000 | val;
	                    procPtr[1] = 0;
	                    
		                nPatched++;
	                }
#if 0
                        LogPrintf("PATCH 0x%x jump_to 0x%x\n", (u32)procPtr, proc);
#endif
	            }
	        }
	        idPtr++;
	        procPtr += 2; // 2 opcodes
        }
    }
    return nPatched;
}

u32 LoadAndStartAndPatch(const char* szFile)
    // return oid or error code
{
	u32 oid;

	oid = sceKernelLoadModule(szFile, 0, NULL);
	LogPrintf("Load: %s = %i\n", szFile, oid);

//REVIEW: if already loaded - assume ok
    if (oid & 0x80000000)
        return oid; // error code

    // Start it
    {
        u32 err;
        s32 fake = 0;
        LogPrintf("  +start : ");
        err = sceKernelStartModule(oid, 0, 0, &fake, 0);
	LogPrintf("%i\n", err);

        if (err != oid)
        {
            LogPrintf(" -- DID NOT START\n");
            return err;
        }
    }

    // Patch it
    {
		int n = PatchMyLibraryEntries(oid);
		LogPrintf("  +patch : %i\n", n);
    }
    return oid;
}

// SYSTEM ENTRY: sceKernelIcacheInvalidateAll:
// 1.0 -> 	88054618:	40088000	mfc0 r8,cop0[16]
// 1.50 ->	880584f0:	40088000	mfc0 r8,cop0[16]
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
            LogPrintf("ERROR: version error (flush)!\n");
            return;   // something terribly wrong
        }
    }
    pfnFlush = (VOID_PROC)addr;

    sceKernelDcacheWritebackAll();
    (*pfnFlush)();
}

int nlhLoadDrivers()
{
	LoadAndStartAndPatch("flash0:/kd/ifhandle.prx"); // kernel

	// wipeout list
	// LoadAndStartAndPatch("flash0:/kd/memab.prx");
	// LoadAndStartAndPatch("flash0:/kd/pspnet_adhoc_auth.prx");
	LoadAndStartAndPatch("flash0:/kd/pspnet.prx");
	// LoadAndStartAndPatch("flash0:/kd/pspnet_adhoc.prx");
	// LoadAndStartAndPatch("flash0:/kd/pspnet_adhocctl.prx");
	LoadAndStartAndPatch("flash0:/kd/pspnet_inet.prx");
	LoadAndStartAndPatch("flash0:/kd/pspnet_apctl.prx");
	LoadAndStartAndPatch("flash0:/kd/pspnet_resolver.prx");
	// LoadAndStartAndPatch("flash0:/kd/pspnet_ap_dialog_dummy.prx");

    // jumps have been patched - flush DCache and ICache
    FlushCaches();

    //REVIEW: add error checks
    return 0;
}

#if 0
// handler not needed
static u32 g_handlerHandle; // ?
void apctl_handler(u32 regs_unknown)
{
    // nothing
}
#endif

int nlhInit()
{
    u32 err;
    err = sceNetInit(0x20000, 0x20, 0x1000, 0x20, 0x1000);
    LogPrintf("sceNetInit returns %i\n", err);
    if (err != 0)
        return err;
	err = sceNetInetInit();
	LogPrintf("sceNetInetInit returns %i\n", err);
	if (err != 0)
        return err;

	err = sceNetResolverInit();
	LogPrintf("sceNetResolverInit returns %i\n", err);
    if (err != 0)
        return err;

	err = sceNetApctlInit(0x1000, 0x42);
	LogPrintf("sceNetApctlInit returns %i\n", err);
    if (err != 0)
        return err;

#if 0
    // add handler
	err = sceNetApctlAddHandler((u32)apctl_handler, 0);
	LogPrintf("sceNetApctlAddHandler returns %i\n", err);
    if (err & 0x80000000)
        return err;
    g_handlerHandle = err;
#endif
    return 0;   // it worked!
}

int nlhTerm()
{
    u32 err;
#if 0
	err = sceNetApctlDelHandler(g_handlerHandle);
	LogPrintf("sceNetApctlDelHandler returns %i\n", err);
#endif

//REVIEW: we need to do something first to stop the connection
//REVIEW: -- sceNetApctlTerm returns 80410A04 ??
	err = sceNetApctlTerm();
	LogPrintf("sceNetApctlTerm returns %i\n", err);

	err = sceNetResolverTerm();
	LogPrintf("sceNetResolverTerm returns %i\n", err);

	err = sceNetInetTerm();
	LogPrintf("sceNetInetTerm returns %i\n", err);
	err = sceNetTerm();
	LogPrintf("sceNetTerm returns %i\n", err);
    return 0; // assume it worked
}

////////////////////////////////////////////////////////////////////

// byte swap - REVIEW: is there a helper in the NetLib ?
unsigned short htons(unsigned short wIn)
{
    u8 bHi = (wIn >> 8) & 0xFF;
    u8 bLo = wIn & 0xFF;
    return ((unsigned short)bLo << 8) | bHi;
}

unsigned long htonl(unsigned long wIn)
{
    u8 bA = wIn & 0xFF;
    u8 bB = (wIn>>8) & 0xFF;
    u8 bC = (wIn>>16) & 0xFF;
    u8 bD = (wIn>>24) & 0xFF;
    return (bA<<24) | (bB<<16) | (bC<<8) | bD;
}

////////////////////////////////////////////////////////////////////
