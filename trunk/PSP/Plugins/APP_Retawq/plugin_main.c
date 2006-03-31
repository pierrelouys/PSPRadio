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
#include <pspctrl.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <Tools.h>
#include <PSPRadio_Exports.h>
#include <APP_Exports.h>
#include <Common.h>

PSP_MODULE_INFO("APP_Retawq", 0, 1, 1);
PSP_HEAP_SIZE_KB(4096);

#define printf pspDebugScreenPrintf
void app_plugin_main();

int ModuleStartAPP()
{
	sleep(1);
	
	SceSize am = sceKernelTotalFreeMemSize();
	ModuleLog(LOG_INFO, "ModuleStartApp(): Available memory: %dbytes (%dKB or %dMB)", am, am/1024, am/1024/1024);

	int thid = 0;

	thid = sceKernelCreateThread("app_thread", (void*) app_plugin_main, 0x25, 0xFA0*2, PSP_THREAD_ATTR_USER, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	//wait_for_button();
	
	return 0;
}

#include "psp_curses.h"
int ModuleContinueApp()
{
	PSPRadioExport_RequestExclusiveAccess(PLUGIN_APP);
	pspDebugScreenInit();
	printf("Press SQUARE");
	g_PSPDisableInput = falsE;
	return 0;
}

int main_loop(int argc, const char** argv);

void app_plugin_main()
{
	PSPRadioExport_RequestExclusiveAccess(PLUGIN_APP);

	pspDebugScreenInit();

	main_loop(0, NULL);

	pspDebugScreenInit();
	PSPRadioExport_GiveUpExclusiveAccess();
}
