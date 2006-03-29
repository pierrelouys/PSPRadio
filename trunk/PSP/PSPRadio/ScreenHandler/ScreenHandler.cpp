/*
	PSPRadio / Music streaming client for the PSP. (Initial Release: Sept. 2005)
	Copyright (C) 2005  Rafael Cabezas a.k.a. Raf

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

// 	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include <PSPApp.h>
#include <PSPSound.h>
#include <Main.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <malloc.h>
#include <iniparser.h>
#include <Tools.h>
#include <Logging.h>
#include <pspwlan.h>
#include <psphprm.h>
#include <pspdisplay.h>
#include <png.h>
#include "ScreenHandler.h"
#include "PlayListScreen.h"
#include "OptionsScreen.h"
#include "OptionsPluginMenuScreen.h"
#include "SHOUTcastScreen.h"
#include "LocalFilesScreen.h"
#include <UI_Interface.h>
#include "PRXLoader.h"
#include "PSPRadio_Exports.h"

#define ReportError pPSPApp->ReportError

CScreenHandler::CScreenHandler(char *strCWD, CIniParser *Config, CPSPSound *Sound, Screen InitialScreen)
{
	m_RequestOnPlayOrStop = NOTHING;
#ifdef DYNAMIC_BUILD
	m_CurrentUI = strdup(DEFAULT_UI_MODULE);
#else
	m_CurrentUI = UI_TEXT;
#endif
	m_UI = NULL;
	m_strCWD = strdup(strCWD);
	m_Config = Config;
	m_Sound = Sound;
	m_PlayMode = PLAYMODE_NORMAL; //PLAYMODE_SINGLE;//

	Log(LOG_VERYLOW, "CScreenHandler Ctor");
	/** Create Screens... */
	Screens[PSPRADIO_SCREEN_LOCALFILES] =
		new LocalFilesScreen(PSPRADIO_SCREEN_LOCALFILES, this);
	((LocalFilesScreen*)Screens[PSPRADIO_SCREEN_LOCALFILES])->SetConfigFilename("LocalFilesScreen.cfg");
	((LocalFilesScreen*)Screens[PSPRADIO_SCREEN_LOCALFILES])->LoadLists();

	Screens[PSPRADIO_SCREEN_PLAYLIST] =
		new PlayListScreen(PSPRADIO_SCREEN_PLAYLIST, this);
	((PlayListScreen*)Screens[PSPRADIO_SCREEN_PLAYLIST])->SetConfigFilename("PlaylistScreen.cfg");
	((PlayListScreen*)Screens[PSPRADIO_SCREEN_PLAYLIST])->LoadLists();

	Screens[PSPRADIO_SCREEN_SHOUTCAST_BROWSER] =
		new SHOUTcastScreen(PSPRADIO_SCREEN_SHOUTCAST_BROWSER, this);
	((SHOUTcastScreen*)Screens[PSPRADIO_SCREEN_SHOUTCAST_BROWSER])->SetConfigFilename("SHOUTcastScreen.cfg");
	((SHOUTcastScreen*)Screens[PSPRADIO_SCREEN_SHOUTCAST_BROWSER])->LoadLists();

	Screens[PSPRADIO_SCREEN_OPTIONS] =
		new OptionsScreen(PSPRADIO_SCREEN_OPTIONS, this);
	((OptionsScreen*)Screens[PSPRADIO_SCREEN_OPTIONS])->SetConfigFilename("OptionsScreen.cfg");

#ifdef DYNAMIC_BUILD
	Screens[PSPRADIO_SCREEN_OPTIONS_PLUGIN_MENU] =
		new OptionsPluginMenuScreen(PSPRADIO_SCREEN_OPTIONS_PLUGIN_MENU, this);
	((OptionsScreen*)Screens[PSPRADIO_SCREEN_OPTIONS_PLUGIN_MENU])->SetConfigFilename("OptionsScreen.cfg");

#endif	
	m_CurrentScreen = Screens[InitialScreen];
	Log(LOG_LOWLEVEL, "Setting m_CurrentScreen to Screens[%d]=%p", InitialScreen, m_CurrentScreen);
	
	/* To avoid getting stuck when Options are the initial screen */
	if (InitialScreen == PSPRADIO_SCREEN_OPTIONS)
	{
		m_PreviousScreen = Screens[PSPRADIO_SCREEN_LOCALFILES];
	}
	else
	{
		m_PreviousScreen = m_CurrentScreen;
	}
	m_StreamOwnerScreen = NULL;

	m_UIModuleLoader = new CPRXLoader();
	if (m_UIModuleLoader == NULL)
	{
		Log(LOG_ERROR, "Memory error - Unable to create CPRXLoader.");
	}
	
	SetInitialScreen(InitialScreen);

	
}

CScreenHandler::~CScreenHandler()
{
	if (m_UI)
	{
		Log(LOG_LOWLEVEL, "Exiting. Calling UI->Terminate");
		m_UI->Terminate();
		Log(LOG_LOWLEVEL, "Exiting. Destroying UI object");
		delete(m_UI);
		m_UI = NULL;
		Log(LOG_LOWLEVEL, "Exiting. UI object deleted.");
	}
	if (m_strCWD)
	{
		Log(LOG_LOWLEVEL, "Exiting. deleting m_strCWD.");
		free(m_strCWD), m_strCWD = NULL;
	}

	for (int i=PSPRADIO_SCREEN_LIST_BEGIN; i < PSPRADIO_SCREEN_LIST_END; i++)
	{
		Log(LOG_LOWLEVEL, "Exiting. Deleting Screens.");
		delete(Screens[i]), Screens[i] = NULL;
	}

#ifdef DYNAMIC_BUILD
	if (m_CurrentUI)
	{
		Log(LOG_LOWLEVEL, "Exiting. Deleting m_CurrentUI string.");
		free(m_CurrentUI), m_CurrentUI = NULL;
	}
#endif
	Log(LOG_LOWLEVEL, "~CScreenHandler() End.");
}

#ifdef DYNAMIC_BUILD
	IPSPRadio_UI *CScreenHandler::StartUI(char *strUIModule)
	{
		bool wasPolling = pPSPApp->IsPolling();
		if (m_UI)
		{
			if (wasPolling)
			{
				/** If PSPRadio was running, then notify it that we're switching UIs */
				Log(LOG_LOWLEVEL, "Notifying PSPRadio That we're switching UIs");
				pPSPApp->SendEvent(EID_NEW_UI_POINTER, NULL, SID_SCREENHANDLER);
				pPSPApp->StopPolling();
			}
	
			Log(LOG_INFO, "StartUI: Destroying current UI");
			m_UI->Terminate();
			delete(m_UI), m_UI = NULL;
			Log(LOG_INFO, "Unloading Module");
			m_UIModuleLoader->Unload();
			Log(LOG_LOWLEVEL, "StartUI: Current UI destroyed.");
		}
		Log(LOG_LOWLEVEL, "StartUI: Starting UI '%s'", strUIModule);
		
		
		Log(LOG_LOWLEVEL, "In PSPRadioPRX: _global_impure_ptr=%p, _impure_ptr=%p", _global_impure_ptr, _impure_ptr);
		
		int id = m_UIModuleLoader->Load(strUIModule);
		
		if (m_UIModuleLoader->IsLoaded() == true)
		{
			SceKernelModuleInfo modinfo;
			memset(&modinfo, 0, sizeof(modinfo));
			modinfo.size = sizeof(modinfo);
			sceKernelQueryModuleInfo(id, &modinfo);
			Log(LOG_ALWAYS, "TEXT_ADDR: '%s' Loaded at text_addr=0x%x",
				strUIModule, modinfo.text_addr);
		
			int iRet = m_UIModuleLoader->Start();
			
			Log(LOG_LOWLEVEL, "Module start returned: 0x%x", iRet);
			
		}
		else
		{
			Log(LOG_ERROR, "Error loading '%s' Module. Error=0x%x", strUIModule, m_UIModuleLoader->GetError());
		}
		
		Log(LOG_INFO, "Calling ModuleStartUI() (at addr %p)", &ModuleStartUI);
		m_UI = ModuleStartUI();
		
		Log(LOG_INFO, "UI Started, m_UI = %p", m_UI);
		
		if (m_CurrentUI)
		{
			free(m_CurrentUI), m_CurrentUI = NULL;
		}
		m_CurrentUI = strdup(strUIModule);
		
		Log(LOG_LOWLEVEL, "Calling m_UI->Initialize");
		m_UI->Initialize(GetCWD(), strUIModule);
		
		Log(LOG_LOWLEVEL, "Calling currentscreen activate in (m_CurrentScreen=%p)", m_CurrentScreen);
		
		m_CurrentScreen->Activate(m_UI);
		Log(LOG_LOWLEVEL, "Activate called.");
		
		if (wasPolling)
		{
			/** If PSPRadio was running, then notify it of the new address of the UI */
			Log(LOG_LOWLEVEL, "Notifying PSPRadio of new UI's address (%p)", m_UI );
			pPSPApp->SendEvent(EID_NEW_UI_POINTER, m_UI, SID_SCREENHANDLER);
			pPSPApp->StartPolling();
		}
	
		return m_UI;
	}
#else
	#include "../../Plugins/UI_Text/TextUI.h"
	#include "../../Plugins/UI_Text3D/TextUI3D.h"
	IPSPRadio_UI *CScreenHandler::StartUI(UIs UI)
	{
		bool wasPolling = pPSPApp->IsPolling();
		if (m_UI)
		{
			if (wasPolling)
			{
				/** If PSPRadio was running, then notify it that we're switching UIs */
				Log(LOG_LOWLEVEL, "Notifying PSPRadio That we're switching UIs");
				pPSPApp->SendEvent(EID_NEW_UI_POINTER, NULL, SID_SCREENHANDLER);
				pPSPApp->StopPolling();
			}
	
			Log(LOG_INFO, "StartUI: Destroying current UI");
			m_UI->Terminate();
			delete(m_UI), m_UI = NULL;
			Log(LOG_LOWLEVEL, "StartUI: Current UI destroyed.");
		}
		Log(LOG_LOWLEVEL, "StartUI: Starting UI %d", UI);
		switch(UI)
		{
			default:
			case UI_TEXT:
				Log(LOG_INFO, "StartUI: Starting Text UI");
				m_UI = new CTextUI();
				m_UI->Initialize("./", "UI_Text");
				break;
			case UI_GRAPHICS:
				Log(LOG_INFO, "StartUI: Starting Graphics UI  -Error, graphicsUI not enabled for this build.");
				#ifdef GRAPHICS_UI
					Log(LOG_INFO, "StartUI: Starting Graphics UI");
					m_UI = new CGraphicsUI();
					m_UI->Initialize("./", "UI_Graphics");
				#endif
				break;
			case UI_3D:
				Log(LOG_INFO, "StartUI: Starting Text3D UI");
				m_UI = new CTextUI3D();
				m_UI->Initialize("./", "UI_Text3D");
				break;
		}
		m_CurrentUI = UI;
		//StartScreen(m_CurrentScreen);
		Log(LOG_LOWLEVEL, "Calling currentscreen activate");
		m_CurrentScreen->Activate(m_UI);
	
		if (wasPolling)
		{
			/** If PSPRadio was running, then notify it of the new address of the UI */
			Log(LOG_LOWLEVEL, "Notifying PSPRadio of new UI's address (0x%x)", m_UI );
			pPSPApp->SendEvent(EID_NEW_UI_POINTER, m_UI, SID_SCREENHANDLER);
			pPSPApp->StartPolling();
		}
	
		return m_UI;
	}
#endif

void CScreenHandler::PrepareShutdown()
{
	if(m_UI)
	{
		m_UI->PrepareShutdown();
	}
}

void CScreenHandler::OnVBlank()
{
	if (m_UI)
	{
		m_UI->OnVBlank();
	}
}

void CScreenHandler::CommonInputHandler(int iButtonMask, u32 iEventType) /** Event Type is MID_ONBUTTON_RELEASED or MID_ONBUTTON_REPEAT */
{
	if (gPSPRadio->GetPluginExclisiveAccess() == true)
	{
		if ( (iButtonMask & PSP_CTRL_LTRIGGER) &&
			 (iButtonMask & PSP_CTRL_RTRIGGER) &&
			 (iButtonMask & PSP_CTRL_TRIANGLE) )
		{
			PSPRadioExport_GiveUpExclusiveAccess();
		}
	}
	else /** Normal */
	{
	
		/** Only do UP and DOWN repeats */
		if (MID_ONBUTTON_REPEAT == iEventType)
		{
			if (((iButtonMask & PSP_CTRL_UP) || (iButtonMask & PSP_CTRL_DOWN)) == false)
			{
				return;
			}
		}
		else if (MID_ONBUTTON_LONG_PRESS == iEventType)
		{
			if (iButtonMask & PSP_CTRL_SELECT)
			{
				GetSound()->Stop(); /** Stop stream if playing, else the event queue can back-up */
				// Generic screenshot method, which works for all UI classes
				Screenshot();
			}
		}
	
		switch (m_CurrentScreen->GetId())
		{
			case CScreenHandler::PSPRADIO_SCREEN_LOCALFILES:
			case CScreenHandler::PSPRADIO_SCREEN_PLAYLIST:
			case CScreenHandler::PSPRADIO_SCREEN_SHOUTCAST_BROWSER:
				if (iButtonMask & PSP_CTRL_HOME)
				{
					//pPSPApp->SendEvent(EID_EXIT_SELECTED, NULL, SID_SCREENHANDLER);
					Log(LOG_VERYLOW, "HOME BUTTON RECEIVED");
				}
				else if (iButtonMask & PSP_CTRL_START)		/** Go to Options screen */
				{
					// Enter option menu and store the current screen
					m_PreviousScreen = m_CurrentScreen;
					m_CurrentScreen  = Screens[PSPRADIO_SCREEN_OPTIONS];
					m_CurrentScreen->Activate(m_UI);
				}
				else if (iButtonMask & PSP_CTRL_TRIANGLE) /** Cycle through screens (except options) */
				{
					m_CurrentScreen = Screens[m_CurrentScreen->GetId()+1];
					/** Don't go to options screen with triangle;
						Also, as options screen is the last one in the list
						we cycle back to the first one */
					if (m_CurrentScreen->GetId() == PSPRADIO_SCREEN_OPTIONS)
					{
						m_CurrentScreen = Screens[PSPRADIO_SCREEN_LIST_BEGIN];
					}
					m_CurrentScreen->Activate(m_UI);
				}
				else
				{
					m_CurrentScreen->InputHandler(iButtonMask);
				}
				break;
			case CScreenHandler::PSPRADIO_SCREEN_OPTIONS:
	#ifdef DYNAMIC_BUILD
			case CScreenHandler::PSPRADIO_SCREEN_OPTIONS_PLUGIN_MENU:
	#endif
				if (iButtonMask & PSP_CTRL_START)	/** Get out of Options screen */
				{
					// Go back to where we were before entering the options menu
					m_CurrentScreen = m_PreviousScreen;
					m_CurrentScreen->Activate(m_UI);
				}
				else
				{
					m_CurrentScreen->InputHandler(iButtonMask);
				}
				break;
		}
	}
}

void CScreenHandler::OnHPRMReleased(u32 iHPRMMask)
{
	if (GetCurrentScreen())
		((PlayListScreen*)GetCurrentScreen())->OnHPRMReleased(iHPRMMask);
};

void CScreenHandler::Screenshot()
{
	char	path[MAXPATHLEN];
	char	*filename;

	sprintf(path, "%s/Screenshots/", m_strCWD);

	filename = ScreenshotName(path);

	if (m_UI)
	{
		m_UI->OnScreenshot(PSPRADIO_SCREENSHOT_ACTIVE);
	}

	if  (filename)
	{
		ScreenshotStore(filename);
		Log(LOG_INFO, "Screenshot stored as : %s", filename);
		free(filename);
	}
	else
	{
		Log(LOG_INFO, "No screenshot taken..");
	}

	if (m_UI)
	{
		m_UI->OnScreenshot(PSPRADIO_SCREENSHOT_NOT_ACTIVE);
	}
}

char *CScreenHandler::ScreenshotName(char *path)
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
void CScreenHandler::ScreenshotStore(char *filename)
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

/*----------------------------------*/
void IScreen::Activate(IPSPRadio_UI *UI)
{
	m_UI = UI;
	m_UI->Initialize_Screen(this);
	m_ScreenHandler->SetCurrentScreen(this);
}
