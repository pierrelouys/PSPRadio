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
PSP_HEAP_SIZE_KB(8192);

#define printf pspDebugScreenPrintf
void app_plugin_main();

int ModuleStartAPP()
{
	sleep(1);
	
	SceSize am = sceKernelTotalFreeMemSize();
	ModuleLog(LOG_INFO, "ModuleStartApp(): Available memory: %dbytes (%dKB or %dMB)", am, am/1024, am/1024/1024);

	int thid = 0;

	thid = sceKernelCreateThread("app_thread", (void*) app_plugin_main, 0x50, 0xFA0*2, PSP_THREAD_ATTR_USER, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	//wait_for_button();
	
	return 0;
}


void wait_for_triangle()
{
	printf("** Press TRIANGLE **");
	SceCtrlData pad;
	for(;;) 
	{
		sceDisplayWaitVblankStart();
		sceCtrlReadBufferPositive(&pad, 1);
		if (pad.Buttons & PSP_CTRL_TRIANGLE)
		{
			break;
		}
	}
}

int main_loop(int argc, const char** argv);

void app_plugin_main()
{
	int run = 1;

	PSPRadioExport_RequestExclusiveAccess();

	while (run == 1)
	{
		pspDebugScreenInit();
		printf(" Retawq Plugin for PSPRadio\n");
		printf("-----------------------------\n");
		printf("* CIRCLE: Start\n");
		printf("* CROSS:  Exit *\n");
		SceCtrlData pad;
		for(;;) 
		{
			sceDisplayWaitVblankStart();
			sceCtrlReadBufferPositive(&pad, 1);
			if (pad.Buttons & PSP_CTRL_CROSS)
			{
				run = 0;
				break;
			}
			else if (pad.Buttons & PSP_CTRL_CIRCLE)
			{
				pspDebugScreenInit();
				//char args[2][MAXPATHLEN+1];
				//getcwd(args[0], MAXPATHLEN);
				//strcpy(args[1], "http://google.com");
				//main_loop(2, args);
				main_loop(0, NULL);
				wait_for_triangle();
				break;
			}
		}
	}

	pspDebugScreenInit();
	PSPRadioExport_GiveUpExclusiveAccess();
}
