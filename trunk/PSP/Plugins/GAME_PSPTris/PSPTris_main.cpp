/*
	PSPTris - The game
	Copyright (C) 2006  Jesper Sandberg

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

#include <stdarg.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "PSPTris.h"
#include "PSPTris_audio.h"
#include "PSPTris_intro.h"
#include "PSPTris_menu.h"
#include "PSPKeyHandler.h"
#include "valloc.h"

#if !defined(DYNAMIC_BUILD)
PSP_MODULE_INFO("PSPTRIS", 0x0000, 0, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);
PSP_MAIN_THREAD_PRIORITY(80);
#endif /* !defined(DYNAMIC_BUILD) */

#define BUF_WIDTH	(512)
#define SCR_WIDTH	(480)
#define SCR_HEIGHT	(272)
#define PIXEL_SIZE	(4)
#define FRAME_SIZE	(BUF_WIDTH * SCR_HEIGHT * PIXEL_SIZE)
#define ZBUF_SIZE	(BUF_WIDTH * SCR_HEIGHT * 2) /* zbuffer seems to be 16-bit? */

/* global gu display list */
static unsigned int __attribute__((aligned(16))) gu_list[8192];

/* Local variables */
bool home_exit = false;
char *cwd = NULL;

#if !defined(DYNAMIC_BUILD)
/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
{
	home_exit = true;
	return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp)
{
    int cbid;
    cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();

	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
    int thid = 0;
    thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
    if (thid >= 0)
	sceKernelStartThread(thid, 0, 0);
    return thid;
}
#endif /* !defined(DYNAMIC_BUILD) */

void init_gu()
{
	sceGuStart(GU_DIRECT,::gu_list);
	sceGuDrawBuffer(GU_PSM_8888,(void*)0,BUF_WIDTH);
	sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,(void*)0x88000,BUF_WIDTH);
	sceGuDepthBuffer((void*)0x110000,BUF_WIDTH);
	sceGuOffset(2048 - (SCR_WIDTH/2),2048 - (SCR_HEIGHT/2));
	sceGuViewport(2048,2048,SCR_WIDTH,SCR_HEIGHT);
	sceGuDepthRange(0xc350,0x2710);
	sceGuScissor(0,0,SCR_WIDTH,SCR_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuDepthFunc(GU_GEQUAL);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuFrontFace(GU_CW);
	sceGuEnable(GU_CULL_FACE);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuTexFilter(GU_LINEAR, GU_LINEAR);

	sceGuClearColor(0x00000000);
	sceGuClearDepth(0);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);

	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);
}

/* main routine. Different name for stand-alone version and PSPRadio plugin version */
#if defined(DYNAMIC_BUILD)
	#ifdef __cplusplus
		extern "C"
		{
	#endif
int main_loop(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	CPSPKeyHandler				keyHandler;
	CPSPKeyHandler::KeyEvent	keyEvent;
	u32							keyDelay;
	bool						done = false;

	cwd = (char*)malloc(MAXPATHLEN);
	if (cwd)
		{
		getcwd(cwd, MAXPATHLEN);
		}
#if defined(DYNAMIC_BUILD)
	strcat(cwd, "/GAME_PSPTris");
#endif /* defined(DYNAMIC_BUILD) */

#if !defined(DYNAMIC_BUILD)
    //init screen and callbacks
    pspDebugScreenInit();
    pspDebugScreenClear();
    SetupCallbacks();
#endif /* !defined(DYNAMIC_BUILD) */

	/* Mark memory for screens */
	(void)valloc(2*FRAME_SIZE + ZBUF_SIZE);

	/* Init audio library */
	(void)PSPTris_audio_init();

	// setup GU
	sceGuInit();
	init_gu();

	/* Run intro sequence */
	PSPTris_intro_init(cwd);

	/* Run intro sequence until a key is pressed */
	while (!done && !home_exit)
	{
		PSPTris_intro();
		if(keyHandler.KeyHandler(keyEvent))
			{
			if (keyEvent.key_state & PSP_CTRL_START)
				{
				done = true;
				}
			}
	}
	PSPTris_intro_destroy();

	PSPTris_menu_init(cwd);

	while (!home_exit)
		{
		u32	key_state = 0;

		bool event_state;
		event_state = keyHandler.KeyHandler(keyEvent);

		if (event_state)
			{
			if ((keyEvent.event == KEY_EVENT_RELEASED) || (keyEvent.event == KEY_EVENT_REPEAT))
				{
				key_state = keyEvent.key_state;
				}
			}
		PSPTris_menu(key_state, &keyDelay);
		/* Set repeat delay reported by the menu / game */
		keyHandler.KeyHandler_Repeat(keyDelay);
		};

	PSPTris_menu_destroy();

	PSPTris_audio_shutdown();

	free(cwd);
	sceGuTerm();

#if !defined(DYNAMIC_BUILD)
    sceKernelExitGame();
#endif /* !defined(DYNAMIC_BUILD) */

    return 0;
}
#if defined(DYNAMIC_BUILD)
	#ifdef __cplusplus
		}
	#endif
#endif
