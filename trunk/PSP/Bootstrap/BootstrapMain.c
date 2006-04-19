/*
 * PSPRadio Loader 
 * -----------------------------------------------------------------------
 * 
 * By Raf.
 *
 * (Based on PSPLINK bootstrap Copyright (c) 2005 James F <tyranid@gmail.com>)
 * (Licensed under the BSD license, see LICENSE in PSPLINK root for details.)
 * 
 *
 */
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <pspsdk.h>
#include <string.h>
#include <stdio.h>
#include <PSPRadio_Exports.h>

#define PSPRADIO_PRX "PSPRadio.prx"
#define LINKS2_PRX   "APP_Links2.prx"

int DriverLoadThread(SceSize args, void *argp);
u32 LoadStartModule(char *path);
void MyExceptionHandler(PspDebugRegBlock *regs);


PSP_MODULE_INFO("PSPRADIOLOADER", 0x1000, 1, 1);
/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(0);
PSP_HEAP_SIZE_KB(64);

/* Define printf, just to make typing easier */
#define printf	pspDebugScreenPrintf

#define MAX_ARGS 16

char *g_argv[MAX_ARGS];
int  g_argc = 0;
static int s_module_text_addr = 0;

/** -- Exception handler */
void MyExceptionHandler(PspDebugRegBlock *regs)
{
	static int bFirstTime = 1;

	if (bFirstTime)
	{
		pspDebugScreenInit();
		pspDebugScreenSetBackColor(0x000000FF);
		pspDebugScreenSetTextColor(0xFFFFFFFF);
		pspDebugScreenClear();

		pspDebugScreenPrintf("PSPRadio -- Exception Caught:\n");
		pspDebugScreenPrintf("Please provide the following information when filing a bug report:\n\n");
		pspDebugScreenPrintf("Exception Details:\n");
		pspDebugDumpException(regs);
		pspDebugScreenPrintf("\nHolding select to capture a shot of this screen for reference\n");
		pspDebugScreenPrintf("may or may not work at this point.\n");
		pspDebugScreenPrintf("\nPlease Use the Home Menu to return to the VSH.\n");
		pspDebugScreenPrintf("-----------------------------------------------------------------\n");

		bFirstTime = 0;
	}
	pspDebugScreenPrintf("******* Important Registers: epc=0x%x ra=0x%x\n", regs->epc, regs->r[31]);
	pspDebugScreenPrintf("******* Module text_addr=0x%x so o-epc=0x%x o-ra=0x%x\n",
			s_module_text_addr, regs->epc - s_module_text_addr, regs->r[31] - s_module_text_addr);
/// doesn't work :(	ModuleLog(LOG_ERROR, "******* Exception! epc=0x%x ra=0x%x", regs->epc, regs->r[31]);
}

SceUID load_module(const char *path, int flags, int type)
{
        SceKernelLMOption option;
        SceUID mpid;

        /* If the type is 0, then load the module in the kernel partition, otherwise load it
           in the user partition. */
        if (type == 0) {
                mpid = 1;
        } else {
                mpid = 2;
        }

        memset(&option, 0, sizeof(option));
        option.size = sizeof(option);
        option.mpidtext = mpid;
        option.mpiddata = mpid;
        option.position = 0;
        option.access = 1;

        return sceKernelLoadModule(path, flags, type > 0 ? &option : NULL);
}

void parse_args(SceSize args, void *argp)
{
        int  loc = 0;
        char *ptr = argp;

        while(loc < args)
        {
                g_argv[g_argc] = &ptr[loc];
                loc += strlen(&ptr[loc]) + 1;
                g_argc++;
                if(g_argc == (MAX_ARGS-1))
                {
                        break;
                }
        }
        g_argv[g_argc] = NULL;
}

int build_args(char *args, const char *bootfile, SceUID thid, SceUID modid, int text_addr, const char *execfile, int argc, char **argv)
{
        int loc = 0;
        int i;

        strcpy(args, bootfile);
        loc += strlen(bootfile) + 1;
        sprintf(&args[loc], "%08X", thid);
        loc += strlen(&args[loc]) + 1;
        sprintf(&args[loc], "Module Id = 0x%08X", modid);
        loc += strlen(&args[loc]) + 1;
        sprintf(&args[loc], "Module Text Address = 0x%08X", text_addr);
        loc += strlen(&args[loc]) + 1;
        if(execfile != NULL)
        {
                strcpy(&args[loc], execfile);
                loc += strlen(execfile) + 1;
                for(i = 0; i < argc; i++)
                {
                        strcpy(&args[loc], argv[i]);
                        loc += strlen(argv[i]) + 1;
                }
        }

        return loc;
}

int GetModuleTextAddr(modid)
{
	SceKernelModuleInfo modinfo;
	memset(&modinfo, 0, sizeof(modinfo));
	modinfo.size = sizeof(modinfo);
	
	int iRet = sceKernelQueryModuleInfo(modid, &modinfo);
	
	if (iRet >= 0)
	{
		return modinfo.text_addr;
	}
	else
	{
		return -1;
	}
}

int main_thread(SceSize args, void *argp)
{
	char prx_args[512];
	char prx_path[256];
	char *path;
	SceUID modid;
	int ret;
	
	pspDebugScreenInit();
	sceDisplayWaitVblankStart();
	
	pspDebugInstallErrorHandler(MyExceptionHandler);
	
	pspSdkInstallNoDeviceCheckPatch();
	pspSdkInstallNoPlainModuleCheckPatch();
	
	if(pspSdkLoadInetModules() < 0)
	{
		printf("** Error, could not load inet modules\n");
		sceKernelSleepThread();
	}
	
	parse_args(args, argp);
	path = strrchr(g_argv[0], '/');
	if(path != NULL)
	{
		SceCtrlData pad;
		sceCtrlReadBufferPositive(&pad, 1);
		
		memcpy(prx_path, g_argv[0], path - g_argv[0] + 1);
		prx_path[path - g_argv[0] + 1] = 0;
		if (pad.Buttons & PSP_CTRL_TRIANGLE)
		{
			strcat(prx_path, LINKS2_PRX);
			/* Start mymodule.prx and dump its information */
			printf("PSPRadio Loader - TRIANGLE PRESSED\n\n");
			
			printf("- Loading Links2 . .\n");
		}
		else
		{
			strcat(prx_path, PSPRADIO_PRX);
			/* Start mymodule.prx and dump its information */
			printf("PSPRadio Loader.\n\n");
			
			printf("- Loading Main Module. . .\n");
		}

		modid = load_module(prx_path, 0, PSP_MEMORY_PARTITION_USER/*0*/);
		if(modid >= 0)
		{
		
				int size;
				int status;
				
				s_module_text_addr = GetModuleTextAddr(modid);

				printf("- Starting Module. . .\n");
				
				size = build_args(prx_args, g_argv[0], sceKernelGetThreadId(), modid, s_module_text_addr, g_argv[1], g_argc-2, &g_argv[2]);

				ret = sceKernelStartModule(modid, size, prx_args, &status, NULL);
				printf("- Done.\n");
		}
		else
		{
				printf("** Error loading '%s' module %08X\n", PSPRADIO_PRX, modid);
		}
	}

	/* Let's bug out */
	sceKernelExitDeleteThread(0);
	
	return 0;
}

int module_start(SceSize args, void *argp) __attribute__((alias("_start")));

/* Entry point */
int _start(SceSize args, void *argp)
{
	int mainthid, driverthid;
	u32 func;
	
	func = (u32) main_thread;
	func |= 0x80000000;
	
	driverthid = sceKernelCreateThread("driverloader_thread", DriverLoadThread, 0x11, 0xFA0, 0, NULL);
	if(driverthid >= 0)
	{
		sceKernelStartThread(driverthid, args, argp);
	}
	
	/* Create a high priority thread */
	mainthid = sceKernelCreateThread("main_thread", (void *) func, 0x20, 0x10000, 0, NULL);
	if(mainthid >= 0)
	{
		sceKernelStartThread(mainthid, args, argp);
	}
	
	return 0;
}

int module_stop(int args, void *argp)
{
	return 0;
}


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
#include <pspsdk.h>
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
	/** Load and start USB drivers */
	LoadStartModule("flash0:/kd/semawm.prx");
	LoadStartModule("flash0:/kd/usbstor.prx");
	LoadStartModule("flash0:/kd/usbstormgr.prx");
	LoadStartModule("flash0:/kd/usbstorms.prx");
	LoadStartModule("flash0:/kd/usbstorboot.prx");

	sceKernelSleepThreadCB();

	return 0;
}
