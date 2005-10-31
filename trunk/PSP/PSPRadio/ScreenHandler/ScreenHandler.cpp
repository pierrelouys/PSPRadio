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
#include "ScreenHandler.h"
#include "PlayListScreen.h"
#include "OptionsScreen.h"
#include "SHOUTcastScreen.h"
#include "DirList.h"
#include "PlayList.h"
#include "TextUI.h"
#include "GraphicsUI.h"
#include "SandbergUI.h" 

#define ReportError pPSPApp->ReportError

CScreenHandler::CScreenHandler(char *strCWD, CIniParser *Config, CPSPSound *Sound, CPlayList *CurrentPlayList, CDirList  *CurrentPlayListDir)
{
	//m_CurrentScreen = PSPRADIO_SCREEN_PLAYLIST;
	//m_PreviousScreen = PSPRADIO_SCREEN_PLAYLIST;
	m_RequestOnPlayOrStop = NOTHING;
	m_CurrentUI = UI_TEXT;
	m_UI = NULL;
	m_strCWD = strdup(strCWD);
	m_Config = Config;
	m_Sound = Sound;
//	m_CurrentPlayList = CurrentPlayList;
//	m_CurrentPlayListDir = CurrentPlayListDir;

	/** Create Screens... */
	Screens[PSPRADIO_SCREEN_PLAYLIST] = 
		new PlayListScreen(PSPRADIO_SCREEN_PLAYLIST, this, CurrentPlayList, CurrentPlayListDir);

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
	static bool fOnExitMenu = false;
	
	if (false == fOnExitMenu) /** Not in home menu */
	{
		if (iButtonMask & PSP_CTRL_HOME)
		{
			Log(LOG_VERYLOW, "Entering HOME menu, ignoring buttons..");
			fOnExitMenu = true;
			return;
		}
		else
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
	}
	else /** In the home menu */
	{
		if ( (iButtonMask & PSP_CTRL_HOME)		||
				(iButtonMask & PSP_CTRL_CROSS)	||
				(iButtonMask & PSP_CTRL_CIRCLE)
			)
		{
			fOnExitMenu = false;
			Log(LOG_VERYLOW, "Exiting HOME menu");
		}
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
}

