/*
 * Links2 Port for PSPRadio
 * -----------------------------------------------------------------------
 *
 * plugin_main.c - Based on parts of: Simple PRX example by James Forshaw <tyranid@gmail.com>
 * by: Raf 2006.
 *
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
#include <links.h>

#ifdef STAND_ALONE_APP
	PSP_MODULE_INFO("Links2", 0x1000, 1, 1);
	PSP_MAIN_THREAD_ATTR(0);
	int CallbackThread(SceSize args, void *argp);
#else
	PSP_MODULE_INFO("APP_Links2", 0, 1, 1);
	PSP_HEAP_SIZE_KB(1024*6);
#endif

#define printf pspDebugScreenPrintf
void app_plugin_main();
void wait_for_triangle(char *str);
int main_loop(int argc, char** argv);
int connect_to_apctl(int config);

/** Plugin code */
int ModuleStartAPP()
{
	int thid = 0;
	sleep(1);
	
	pspDebugScreenInit();
	
	SceSize am = sceKernelTotalFreeMemSize();
	ModuleLog(LOG_INFO, "ModuleStartApp(): Available memory: %dbytes (%dKB or %dMB)", am, am/1024, am/1024/1024);

	thid = sceKernelCreateThread("app_thread", (void*) app_plugin_main, 80, 0xFA0*2, PSP_THREAD_ATTR_USER, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return 0;
}

int ModuleContinueApp()
{
	PSPRadioExport_RequestExclusiveAccess(PLUGIN_APP);
	pspDebugScreenInit();

	g_PSPEnableInput = truE;
	g_PSPEnableRendering = truE;
	cls_redraw_all_terminals();
	return 0;
}


int CreateHomepage(char *file)
{
	FILE *fp = fopen(file, "w");
	
	if (fp)
	{
		fprintf(fp, "<html><head><title>Links2 On PSP</title></head><body bgcolor=\"white\"><h3 align=\"center\"><b>Links2 For PSPRadio</b></h3>\n");
	
		fprintf(fp, "<p>PSP Port by Raf. Thanks to Danzel for OSK!<br> Thanks to Sandberg for upcoming GU driver :)<br>");
#ifdef STAND_ALONE_APP
		fprintf(fp, "This port is a stand alone application.<br>", PSPRadioExport_GetVersion());
#else
		fprintf(fp, "This port is a plugin for PSPRadio Version %s.<br>", PSPRadioExport_GetVersion());
#endif
		fprintf(fp, "Visit us at <a href=\"http://pspradio.berlios.de\">PSPRadio Forums</a> Or <a href=\"http://rafpsp.blogspot.com\">PSPRadio HomePage</a>.</p>\n");
		
		/** Google search Start */
		fprintf(fp, "<center>");
		fprintf(fp, "<form method=\"get\" action=\"http://www.google.com/custom\" target=\"_top\">");
		fprintf(fp, "<table bgcolor=\"#ffffff\">");
		fprintf(fp, "<tr><td nowrap=\"nowrap\" valign=\"top\" align=\"left\" height=\"32\">");
		fprintf(fp, "<a href=\"http://www.google.com/\">");
		fprintf(fp, "<img src=\"http://www.google.com/logos/Logo_25wht.gif\" border=\"0\" alt=\"Google\" align=\"middle\"></img></a>");
		fprintf(fp, "<br/>");
		fprintf(fp, "<input type=\"text\" name=\"q\" size=\"31\" maxlength=\"255\" value=\"\"></input>");
		fprintf(fp, "</td></tr>");
		fprintf(fp, "<tr><td valign=\"top\" align=\"left\">");
		fprintf(fp, "<input type=\"submit\" name=\"sa\" value=\"Search\"></input>");
		fprintf(fp, "<input type=\"hidden\" name=\"client\" value=\"pub-3916941649621652\"></input>");
		fprintf(fp, "<input type=\"hidden\" name=\"forid\" value=\"1\"></input>");
		fprintf(fp, "<input type=\"hidden\" name=\"channel\" value=\"5433159913\"></input>");
		fprintf(fp, "<input type=\"hidden\" name=\"ie\" value=\"ISO-8859-1\"></input>");
		fprintf(fp, "<input type=\"hidden\" name=\"oe\" value=\"ISO-8859-1\"></input>");
		fprintf(fp, "<input type=\"hidden\" name=\"cof\" value=\"GALT:#008000;GL:1;DIV:#336699;VLC:663399;AH:center;BGC:FFFFFF;LBGC:336699;ALC:0000FF;LC:0000FF;T:000000;GFNT:0000FF;GIMP:0000FF;FORID:1;\"></input>");
		fprintf(fp, "<input type=\"hidden\" name=\"hl\" value=\"en\"></input>");
		fprintf(fp, "</td></tr></table>");
		fprintf(fp, "</form>");
		fprintf(fp, "</center><br><br>");
		/** Google Search end */
		
		/** Load tips from tips file */
		{
			FILE *fTips = NULL;
			char strLine[512];
			#ifdef STAND_ALONE_APP
				fTips = fopen(".links/tips.html", "r");
			#else
				fTips = fopen("APP_Links2/tips.html", "r");
			#endif
			if (fTips)
			{
				while (!feof(fTips))
				{
					if (fgets(strLine, 512, fTips) != NULL)
					{
						fprintf(fp, strLine);
					}
					else
					{
						break;
					}
				}
				fclose(fTips), fTips = NULL;
			}
			else
			{
				fprintf(fp, "<p><br><br>PSP Port Tips: <br>Input mode: <b>START</b> = Enter. <b>START</b> = Exit.<br><b>L+UP</b> = Page up <b>L+Down</b> = Page Down.<br><b>CIRCLE</b> = Yes / OK / Enter<br><b>SQUARE</b> = No / Cancel<br><b>CROSS</b> = Right Mouse Click. <b>TRIANGLE</b> = Left Mouse Click.<br><b>SELECT</b> = Take Screenshot. <b>CROSS+SELECT</b> = Network Reconnect.</p><br><br>\n");
			}
		}
		
		fprintf(fp, "\n</body></html>\n");
		
		fclose(fp), fp = NULL;
		return 0;
	}
	
	return -1;
}

static char *argv[] = { "APP_Links2", "-g", "-driver", "pspsdl", "-mode", "480x272", "http://pspradio.berlios.de/Links2/APP_Links2.html", NULL };	

void app_plugin_main()
{
	static int argc = sizeof(argv)/sizeof(char *)-1; 	/* idea from scummvm psp port */
#if 0	
	char strhp[128];
#endif
	char str[128];
	int ret;
	
	PSPRadioExport_RequestExclusiveAccess(PLUGIN_APP);

#if 0
	getcwd(str, 100);
#ifdef STAND_ALONE_APP
	sprintf(strhp, "%s/.%s/%s.html", str, argv[0], argv[0]);
	printf("Creating '%s'\n", strhp);
	//wait_for_triangle(strhp);
#else // plugin
	sprintf(strhp, "%s/%s/%s.html", str, argv[0], argv[0]);
#endif

	if (CreateHomepage(strhp) == 0)
	{
#ifdef STAND_ALONE_APP
		sprintf(strhp, "file://%s/.%s/%s.html", str, argv[0], argv[0]);
#else // plugin
		sprintf(strhp, "file://%s/%s/%s.html", str, argv[0], argv[0]);
#endif
		argv[6] = strhp;
	}
	else
	{
		printf("Could not create '%s'\n", strhp);
		sceKernelSleepThreadCB();
		argv[6] = NULL;
		argc--;
	}
#endif

	g_PSPEnableInput = truE;
	g_PSPEnableRendering = truE;

	ret = main_loop(argc, (char **)&argv);

	if (ret != 0) 
	{
		sprintf(str, "Application returns %d", ret);
		wait_for_triangle(str);
	}

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



/** Stand alone code: */
#ifdef STAND_ALONE_APP
int StartNetworkThread(SceSize args, void *argp);
int main(int argc, char **argv)
{
	int thid;
	pspDebugScreenInit();
	pspDebugScreenPrintf("Links2 For PSP\n\n");
	pspDebugScreenPrintf("-Loading networking modules...\n");
	
	sceDisplayWaitVblankStart();
		
		//pspDebugInstallErrorHandler(MyExceptionHandler);
		
	pspSdkInstallNoDeviceCheckPatch();
	pspSdkInstallNoPlainModuleCheckPatch();
		
	if(pspSdkLoadInetModules() < 0)
	{
		printf("** Error, could not load inet modules\n");
		sceKernelSleepThreadCB();
	}
	
	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, PSP_THREAD_ATTR_USER, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}
	
	thid = sceKernelCreateThread("network_start_thread", StartNetworkThread, 0x11, 0xFA0, PSP_THREAD_ATTR_USER, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}
	
	thid = sceKernelCreateThread("app_thread", (void*) app_plugin_main, 80, 0xFA0*2, PSP_THREAD_ATTR_USER, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}
	
	sceKernelSleepThreadCB();
	return 0;
}

/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
{
	sceKernelExitGame();
	return 0;
}

/* Callback thread */
int StartNetworkThread(SceSize args, void *argp)
{
	int cbid;
	static int ResolverId;
	static char resolver_buffer[1024];

	pspSdkInetInit();
	
	connect_to_apctl(1); /* Just connect to the first profile for now */
	
	sceNetResolverCreate(&ResolverId, resolver_buffer, 1024);

	sceKernelSleepThreadCB();

	return 0;
}

int CallbackThread(SceSize args, void *argp)
{
	int cbid;
	
	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	
	sceKernelSleepThreadCB();

	return 0;
}

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
#else
void TakeScreenShot()
{
	PSPRadioExport_TakeScreenShot();
}
#endif
