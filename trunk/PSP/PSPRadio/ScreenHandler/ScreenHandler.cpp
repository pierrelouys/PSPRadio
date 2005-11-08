/* 
	PSPRadio / Music streaming client for the PSP. (Initial Release: Sept. 2005)
	Copyright (C) 2005  Rafael Cabezas a.k.a. Raf
	
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
#include <PSPApp.h>
#include <PSPSound.h>
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
#include <PSPNet.h>
#include <pspdisplay.h>
#include <png.h>
#include "ScreenHandler.h"
#include "PlayListScreen.h"
#include "OptionsScreen.h"
#include "SHOUTcastScreen.h"
#include "TextUI.h"
#include "GraphicsUI.h"
#include "SandbergUI.h" 

#define ReportError pPSPApp->ReportError

CScreenHandler::CScreenHandler(char *strCWD, CIniParser *Config, CPSPSound *Sound)
{
	m_RequestOnPlayOrStop = NOTHING;
	m_CurrentUI = UI_TEXT;
	m_UI = NULL;
	m_strCWD = strdup(strCWD);
	m_Config = Config;
	m_Sound = Sound;

	/** Create Screens... */
	Screens[PSPRADIO_SCREEN_PLAYLIST] = 
		new PlayListScreen(PSPRADIO_SCREEN_PLAYLIST, this);

	Screens[PSPRADIO_SCREEN_SHOUTCAST_BROWSER] = 
		new SHOUTcastScreen(PSPRADIO_SCREEN_SHOUTCAST_BROWSER, this);

	Screens[PSPRADIO_SCREEN_OPTIONS] = 
		new OptionsScreen(PSPRADIO_SCREEN_OPTIONS, this);

	m_CurrentScreen = Screens[PSPRADIO_SCREEN_PLAYLIST];
	m_PreviousScreen = m_CurrentScreen;
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
	}
	if (m_strCWD)
	{
		free(m_strCWD), m_strCWD = NULL;
	}

	for (int i=PSPRADIO_SCREEN_LIST_BEGIN; i < PSPRADIO_SCREEN_LIST_END; i++)
	{
		delete(Screens[i]), Screens[i] = NULL;
	}
}

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
			break;
		case UI_GRAPHICS:
			Log(LOG_INFO, "StartUI: Starting Graphics UI");
			m_UI = new CGraphicsUI();
			break;
		case UI_3D:
			Log(LOG_INFO, "StartUI: Starting 3D UI");
			m_UI = new CSandbergUI();
			break;
	}
	m_CurrentUI = UI;
	m_UI->Initialize("./");//strCurrentDir); /* Initialize takes cwd */ ///FIX!!!
	//StartScreen(m_CurrentScreen);
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

void CScreenHandler::OnVBlank()
{
	m_UI->OnVBlank();
}

void CScreenHandler::CommonInputHandler(int iButtonMask)
{
	switch (m_CurrentScreen->GetId())
	{
		case CScreenHandler::PSPRADIO_SCREEN_PLAYLIST:
		case CScreenHandler::PSPRADIO_SCREEN_SHOUTCAST_BROWSER:
			if (iButtonMask & PSP_CTRL_START)		/** Go to Options screen */
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

void CScreenHandler::OnHPRMReleased(u32 iHPRMMask)
{
	((PlayListScreen*)Screens[PSPRADIO_SCREEN_PLAYLIST])->OnHPRMReleased(iHPRMMask);
};

void IScreen::Activate(IPSPRadio_UI *UI)
{
	m_UI = UI; 
	m_UI->Initialize_Screen((CScreenHandler::Screen)m_Id);
	m_ScreenHandler->SetCurrentScreen(this);
}

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