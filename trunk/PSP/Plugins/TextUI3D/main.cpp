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
//PSP_NO_CREATE_MAIN_THREAD();
//PSP_HEAP_SIZE_KB(12288);
PSP_HEAP_SIZE_KB(8192);

volatile bool g_ExitModule = false;

extern "C" 
{
int main(int argc, char **argv)
{
	SceSize am = sceKernelTotalFreeMemSize();
	ModuleLog(LOG_INFO, "TEXTUI3D: main(): Available memory: %dbytes (%dKB or %dMB)", am, am/1024, am/1024/1024);

	while (g_ExitModule == false)
	{
		sleep(1);
	}
	return 0;
}

IPSPRadio_UI *ModuleStartUI()
{
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
	g_ExitModule = true;
	return 0;
}
}
