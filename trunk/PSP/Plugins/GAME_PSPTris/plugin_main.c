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
#include <unistd.h>
#include <PSPRadio_Exports.h>
#include <GAME_Exports.h>
#include <Common.h>

void game_plugin_main();

PSP_MODULE_INFO("PSPTris", 0, 1, 1);
PSP_HEAP_SIZE_KB(4096);

int ModuleStartGAME()
{
	sleep(1);

	SceSize am = sceKernelTotalFreeMemSize();
	ModuleLog(LOG_INFO, "ModuleStartGAME(): Available memory: %dbytes (%dKB or %dMB)", am, am/1024, am/1024/1024);

	int thid = 0;

	thid = sceKernelCreateThread("game_thread", (void*) game_plugin_main, 0x25, 0xFA0*2, PSP_THREAD_ATTR_USER, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return 0;
}

int main_loop(int argc, const char** argv);

void game_plugin_main()
{
	PSPRadioExport_RequestExclusiveAccess(PLUGIN_GAME);
	main_loop(0, NULL);
	///PSPRadioExport_GiveUpExclusiveAccess(); /** PluginExits gives up access too */
	PSPRadioExport_PluginExits(PLUGIN_GAME); /** Notify PSPRadio, so it can unload the plugin */
}
