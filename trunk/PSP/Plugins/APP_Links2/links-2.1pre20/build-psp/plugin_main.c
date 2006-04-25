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
void wait_for_triangle(char *str);

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
	}
	#endif
	
	thid = sceKernelCreateThread("app_thread", (void*) app_plugin_main, 80, 0xFA0*2, PSP_THREAD_ATTR_USER, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return 0;
}

int ModuleContinueApp()
{
	return 0;
}

int stderr_handler(char *data, int len)
{
	wait_for_triangle(data);
}

int main_loop(int argc, char** argv);

int connect_to_apctl(int config);

static char *argv[] = { "APP_Links2", "-g", "-driver", "sdl", "-mode", "480x272", "file://ms0:/psp/game/__SCE__PSPRadio/APP_Links2/pspupdates.html", NULL };	
void app_plugin_main()
{
	static int argc = sizeof(argv)/sizeof(char *)-1; 	/* idea from scummvm psp port */
	char str[128];
	int ret;
		
	PSPRadioExport_RequestExclusiveAccess(PLUGIN_APP);
	
	pspDebugScreenInit();
	
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

#ifdef STAND_ALONE_APP

#include <png.h>
#include <pspdisplay.h>
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272
char *ScreenshotName(char *path);
void ScreenshotStore(char *filename);

void TakeScreenShot()
{
	char	path[MAXPATHLEN];
	char	*filename;
	char    m_strCWD[MAXPATHLEN+1];
	
	getcwd(m_strCWD, MAXPATHLEN);

	sprintf(path, "%s/Screenshots/", m_strCWD);

	filename = ScreenshotName(path);

	if  (filename)
	{
		ScreenshotStore(filename);
		ModuleLog(LOG_INFO, "Screenshot stored as : %s", filename);
		free(filename);
	}
	else
	{
		ModuleLog(LOG_INFO, "No screenshot taken..");
	}
}

char *ScreenshotName(char *path)
{
	char	*filename;
	int		image_number;
	FILE	*temp_handle;

	filename = (char *) malloc(MAXPATHLEN);
	if (filename)
	{
		for (image_number = 0 ; image_number < 1000 ; image_number++)
		{
			sprintf(filename, "%sPSPRadio_Screen%03d.png", path, image_number);
			temp_handle = fopen(filename, "r");
			// If the file didn't exist we can use this current filename for the screenshot
			if (!temp_handle)
			{
				break;
			}
			fclose(temp_handle);
		}
	}
	return filename;
}

//The code below is take from an example for libpng.
void ScreenshotStore(char *filename)
{
	u32* vram32;
	u16* vram16;
	int bufferwidth;
	int pixelformat;
	int unknown;
	int i, x, y;
	png_structp png_ptr;
	png_infop info_ptr;
	FILE* fp;
	u8* line;
	fp = fopen(filename, "wb");
	if (!fp) return;
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) return;
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		fclose(fp);
		return;
	}
	png_init_io(png_ptr, fp);
	png_set_IHDR(png_ptr, info_ptr, SCREEN_WIDTH, SCREEN_HEIGHT,
		8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	line = (u8*) malloc(SCREEN_WIDTH * 3);
	sceDisplayWaitVblankStart();  // if framebuf was set with PSP_DISPLAY_SETBUF_NEXTFRAME, wait until it is changed
	sceDisplayGetFrameBuf((void**)&vram32, &bufferwidth, &pixelformat, &unknown);
	vram16 = (u16*) vram32;
	for (y = 0; y < SCREEN_HEIGHT; y++) {
		for (i = 0, x = 0; x < SCREEN_WIDTH; x++) {
			u32 color = 0;
			u8 r = 0, g = 0, b = 0;
			switch (pixelformat) {
				case PSP_DISPLAY_PIXEL_FORMAT_565:
								color = vram16[x + y * bufferwidth];
								r = (color & 0x1f) << 3;
								g = ((color >> 5) & 0x3f) << 2 ;
								b = ((color >> 11) & 0x1f) << 3 ;
								break;
				case PSP_DISPLAY_PIXEL_FORMAT_5551:
								color = vram16[x + y * bufferwidth];
								r = (color & 0x1f) << 3;
								g = ((color >> 5) & 0x1f) << 3 ;
								b = ((color >> 10) & 0x1f) << 3 ;
								break;
				case PSP_DISPLAY_PIXEL_FORMAT_4444:
								color = vram16[x + y * bufferwidth];
								r = (color & 0xf) << 4;
								g = ((color >> 4) & 0xf) << 4 ;
								b = ((color >> 8) & 0xf) << 4 ;
								break;
				case PSP_DISPLAY_PIXEL_FORMAT_8888:
								color = vram32[x + y * bufferwidth];
								r = color & 0xff;
								g = (color >> 8) & 0xff;
								b = (color >> 16) & 0xff;
								break;
			}
			line[i++] = r;
			line[i++] = g;
			line[i++] = b;
		}
		png_write_row(png_ptr, line);
	}
	free(line);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	fclose(fp);
}
#endif