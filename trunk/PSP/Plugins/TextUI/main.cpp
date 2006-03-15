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
#include <TextUI.h>

PSP_MODULE_INFO("TEXTUI", 0, 1, 1);
//PSP_NO_CREATE_MAIN_THREAD();
PSP_HEAP_SIZE_KB(4096);
//PSP_HEAP_SIZE_KB(12288);

volatile bool g_ExitModule = false;

extern "C" 
{
int main(int argc, char **argv)
{
	SceSize am = sceKernelTotalFreeMemSize();
	ModuleLog(LOG_INFO, "TEXTUI: main(): Available memory: %dbytes (%dKB or %dMB)", am, am/1024, am/1024/1024);

	while (g_ExitModule == false)
	{
		sleep(1);
	}
	ModuleLog(LOG_INFO, "TEXTUI: main(): Exiting");
	return 0;
}

IPSPRadio_UI *ModuleStartUI()
{
	ModuleLog(LOG_LOWLEVEL, "TextUI- ModuleStartUI()");

	SceSize am = sceKernelTotalFreeMemSize();
	ModuleLog(LOG_INFO, "TEXTUI: startui(): Available memory: %dbytes (%dKB or %dMB)", am, am/1024, am/1024/1024);


	ModuleLog(LOG_LOWLEVEL, "TextUI- _global_impure_ptr=%p, _impure_ptr=%p", _global_impure_ptr, _impure_ptr);

	return new CTextUI();
}

void* getModuleInfo(void)
{
	return (void *) &module_info;
}


int module_stop(int args, void *argp)
{
	ModuleLog(LOG_INFO, "TEXTUI: module_stop() called, setting g_ExitModule to true");
	g_ExitModule = true;
	sleep(2);
	//exit(0);
	return 0;
}
}
