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

PSP_MODULE_INFO("UI_TEXT", 0, 1, 1);
PSP_HEAP_SIZE_KB(4096);

IPSPRadio_UI *ModuleStartUI()
{
	sleep(1);

	ModuleLog(LOG_LOWLEVEL, "TextUI- ModuleStartUI()");

	SceSize am = sceKernelTotalFreeMemSize();
	ModuleLog(LOG_INFO, "TEXTUI: startui(): Available memory: %dbytes (%dKB or %dMB)", am, am/1024, am/1024/1024);


	ModuleLog(LOG_LOWLEVEL, "TextUI- _global_impure_ptr=%p, _impure_ptr=%p", _global_impure_ptr, _impure_ptr);

	return new CTextUI();
}

/** START of Plugin definitions setup */
UIPlugin textui_vtable = {
	PLUGIN_UI_VERSION, /* Interface version -- Don't change */
	NULL,//Populated by PSPRadio -- PSPRadio object.
	NULL,//	void *handle; /* Filled in by PSPRadio */
	NULL,//	char *filename; /* Filled in by PSPRadio */
	"TextUI",//	char *description; /* The description that is shown in the preferences box */
	NULL,//	void (*init)(void); /* Called when the plugin is enabled */
	NULL,//	void (*cleanup)(void); /* Called when the plugin is disabled */
	NULL,//	void (*about)(void); /* not used atm *//* Show the about box */
	NULL,//	void (*configure)(void); /* not used atm *//* Show the configure box */
	NULL,//	void (*disable_plugin)(struct _UIPlugin *); /* not used atm *//* Call this with a pointer to your plugin to disable the plugin */
};
/** START of Plugin definitions setup */

UIPlugin *get_uiplugin_info()
{
	return &textui_vtable;
}
