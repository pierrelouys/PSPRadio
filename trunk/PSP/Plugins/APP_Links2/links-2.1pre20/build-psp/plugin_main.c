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

PSP_MODULE_INFO("APP_Links2", 0, 1, 1);
#ifdef STAND_ALONE_APP
PSP_HEAP_SIZE_KB(1024*16); /*20MB*/
#else
PSP_HEAP_SIZE_KB(8192*2);
#endif

#define printf pspDebugScreenPrintf
void app_plugin_main();

/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
{
	sceKernelExitGame();
	return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp)
{
	int cbid;
	static int ResolverId;
	static char resolver_buffer[1024];

	pspSdkInetInit();
	connect_to_apctl(1);
	sceNetResolverCreate(&ResolverId, resolver_buffer, 1024);

	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();

	return 0;
}

int ModuleStartAPP()
{
	sleep(1);
	
	SceSize am = sceKernelTotalFreeMemSize();
	ModuleLog(LOG_INFO, "ModuleStartApp(): Available memory: %dbytes (%dKB or %dMB)", am, am/1024, am/1024/1024);

	int thid = 0;

	#ifdef STAND_ALONE_APP
	{
		
		int thid = 0;

		thid = sceKernelCreateThread("update_thread", CallbackThread,
						 0x11, 0xFA0, PSP_THREAD_ATTR_USER, 0);
		if(thid >= 0)
		{
			sceKernelStartThread(thid, 0, 0);
		}
		//sleep(10);
	}
	#endif
	
	thid = sceKernelCreateThread("app_thread", (void*) app_plugin_main, 80, 0xFA0*2, PSP_THREAD_ATTR_USER, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	//wait_for_button();
	
	return 0;
}

// #include "psp_curses.h"
// int ModuleContinueApp()
// {
// 	PSPRadioExport_RequestExclusiveAccess(PLUGIN_APP);
// 	pspDebugScreenInit();
// 
// 	g_PSPEnableInput = truE;
// 	g_PSPEnableRendering = truE;
// 	window_redraw_all();
// 	return 0;
// }

int ModuleContinueApp()
{
	return 0;
}

int stderr_handler(char *data, int len)
{
	wait_for_triangle(data);
}

int main_loop(int argc, const char** argv);

int connect_to_apctl(int config);

static char *argv[] = { "APP_Links2", "-g", "-driver", "sdl", "-mode", "480x272", "file://ms0:/psp/game/__SCE__PSPRadio/APP_Links2/pop.html", NULL };	
void app_plugin_main()
{
	/* idea from scummvm psp port */
	static int argc = sizeof(argv)/sizeof(char *)-1;
	char str[128];
	int ret;
		
	PSPRadioExport_RequestExclusiveAccess(PLUGIN_APP);
	
	pspDebugScreenInit();
	
	//pspDebugInstallStdoutHandler(stderr_handler);
	//pspDebugInstallStderrHandler(stderr_handler);
	
	//ret = sceKernelStdoutReopen("ms0:/links2.stdout", PSP_O_WRONLY, 0777);
	//ret = sceKernelStderrReopen("ms0:/links2.stderr", PSP_O_WRONLY, 0777);
	
	ret = main_loop(argc, (char **)&argv);
	
	sprintf(str, "Application returns %d", ret);
	wait_for_triangle(str);

	#ifdef STAND_ALONE_APP
		sceKernelExitGame();
	#endif
	
	pspDebugScreenInit();
	PSPRadioExport_GiveUpExclusiveAccess();
}

/* Connect to an access point */
int connect_to_apctl(int config)
{
	int err;
	int stateLast = -1;

	/* Connect using the first profile */
	err = sceNetApctlConnect(config);
	if (err != 0)
	{
		printf(": sceNetApctlConnect returns %08X\n", err);
		return 0;
	}

	printf(": Connecting...\n");
	while (1)
	{
		int state;
		err = sceNetApctlGetState(&state);
		if (err != 0)
		{
			printf(": sceNetApctlGetState returns $%x\n", err);
			break;
		}
		if (state > stateLast)
		{
			printf("  connection state %d of 4\n", state);
			stateLast = state;
		}
		if (state == 4)
			break;  // connected with static IP

		// wait a little before polling again
		sceKernelDelayThread(50*1000); // 50ms
	}
	printf(": Connected!\n");

	if(err != 0)
	{
		return 0;
	}

	return 1;
}

void wait_for_triangle(char *str)
{
	printf("%s\n", str);
	printf("** Press TRIANGLE **\n");
	SceCtrlData pad;
	SceCtrlLatch latch ; 
	int button = 0;
	for(;;) 
	{
		sceDisplayWaitVblankStart();
		//sceCtrlReadBufferPositive(&pad, 1);
		sceCtrlReadLatch(&latch);
		
		if (latch.uiMake)
		{
			// Button Pressed 
			button = latch.uiPress;
		}
		else if (latch.uiBreak) {/** Button Released */
			if (button & PSP_CTRL_TRIANGLE)
			{
				break;
			}
		}
	}
}

void app_init_progress(char *str)
{
	static int step = 0;
	
	printf("Init Step %d..%s\n", step, str);
	
	step++;
}
