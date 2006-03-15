/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * main.c - Simple PRX example.
 *
 * Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
 *
 * $Id: main.c 1531 2005-12-07 18:27:12Z tyranid $
 */
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <PSPApp.h>
#include <PSPSound.h>
#include <TextUI3D.h>

PSP_MODULE_INFO("TEXTUI3D", 0, 1, 1);
PSP_HEAP_SIZE_KB(8192);

volatile int g_blocker = 0;
#define BLOCKER_CREATE_AND_BLOCK(x,name) {x=sceKernelCreateSema(name,0,0,1,0);sceKernelWaitSema(x,1,NULL);}
#define BLOCKER_UNBLOCK_AND_DESTROY(x)   {sceKernelDeleteSema(x);sleep(1);}

extern "C" 
{
int main(int argc, char **argv)
{
	SceSize am = sceKernelTotalFreeMemSize();
	ModuleLog(LOG_INFO, "TEXTUI3D: main(): Available memory: %dbytes (%dKB or %dMB)", am, am/1024, am/1024/1024);

	BLOCKER_CREATE_AND_BLOCK(g_blocker, "ui_textui3d_blocker");

	return 0;
}

IPSPRadio_UI *ModuleStartUI()
{
	sleep(1);

	ModuleLog(LOG_LOWLEVEL, "TextUI3D: ModuleStartUI()");

	SceSize am = sceKernelTotalFreeMemSize();
	ModuleLog(LOG_INFO, "TEXTUI3D: startui(): Available memory: %dbytes (%dKB or %dMB)", am, am/1024, am/1024/1024);

	ModuleLog(LOG_LOWLEVEL, "TextUI3D: _global_impure_ptr=%p, _impure_ptr=%p", _global_impure_ptr, _impure_ptr);

	CTextUI3D *ret = new CTextUI3D();

	ModuleLog(LOG_LOWLEVEL, "new CTextUI3D address=%p", ret);
	
	return ret;
}

void* getModuleInfo(void)
{
	return (void *) &module_info;
}

int module_stop(int args, void *argp)
{
	ModuleLog(LOG_INFO, "TEXTUI3D: module_stop() called, unblocking main");
	BLOCKER_UNBLOCK_AND_DESTROY(g_blocker);

	return 0;
}
}
