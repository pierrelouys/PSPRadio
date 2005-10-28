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
#include "../libpspnet/pspnet.h"
#include "ScreenHandler.h"
#include "DirList.h"
#include "PlayList.h"
#include "TextUI.h"
#include "GraphicsUI.h"
#include "SandbergUI.h" 

#define ReportError pPSPApp->ReportError

CScreenHandler::CScreenHandler(char *strCWD, CIniParser *Config, CPSPSound *Sound)
{
	m_CurrentScreen = PSPRADIO_SCREEN_PLAYLIST;
	m_PreviousScreen = PSPRADIO_SCREEN_PLAYLIST;
	m_iNetworkProfile = 1;
	m_NetworkStarted  = false;
	m_RequestOnPlayOrStop = NOTHING;
	m_CurrentUI = UI_TEXT;
	m_UI = NULL;
	m_strCWD = strdup(strCWD);

	SetUp(Config, Sound, NULL, NULL);
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
}

void CScreenHandler::SetUp(CIniParser *Config, CPSPSound *Sound,
							CPlayList *CurrentPlayList, CDirList  *CurrentPlayListDir)
{	
	m_Config = Config;
	m_Sound = Sound;
	m_CurrentPlayList = CurrentPlayList;
	m_CurrentPlayListDir = CurrentPlayListDir;
	m_CurrentScreen = PSPRADIO_SCREEN_PLAYLIST;
	
	PopulateOptionsData();
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
	StartScreen(m_CurrentScreen);

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

int CScreenHandler::Start_Network(int iProfile)
{
	if (-1 != iProfile)
	{
		m_iNetworkProfile = abs(iProfile);
	}

	if (0 == iProfile)
	{
		m_iNetworkProfile = 1;
		Log(LOG_ERROR, "Network Profile in is invalid. Network profiles start from 1.");
	}
	if (sceWlanGetSwitchState() != 0)
	{
		if (m_NetworkStarted)
		{
			m_UI->DisplayMessage_DisablingNetwork();

			Log(LOG_INFO, "Triangle Pressed. Restarting networking...");
			pPSPApp->DisableNetwork();
			sceKernelDelayThread(500000);  
		}
		
		m_UI->DisplayMessage_EnablingNetwork();

		if (pPSPApp->EnableNetwork(abs(m_iNetworkProfile)) == 0)
		{
			m_UI->DisplayMessage_NetworkReady(pPSPApp->GetMyIP());
		}
		else
		{
			m_UI->DisplayMessage_DisablingNetwork();
		}

		m_UI->DisplayMessage_NetworkReady(pPSPApp->GetMyIP());
		Log(LOG_INFO, "Networking Enabled, IP='%s'...", pPSPApp->GetMyIP());
		
		m_NetworkStarted = true;
		Log(LOG_INFO, "Enabling Network: Done. IP='%s'", pPSPApp->GetMyIP());
		
	}
	else
	{
		ReportError("The Network Switch is OFF, Cannot Start Network.");
	}
	
	return 0;
}	

int CScreenHandler::Stop_Network()
{
	if (m_NetworkStarted)
	{
		m_UI->DisplayMessage_DisablingNetwork();

		Log(LOG_INFO, "Disabling network...");
		pPSPApp->DisableNetwork();
		sceKernelDelayThread(500000);  
	}
	return 0;
}

void CScreenHandler::GetNetworkProfileName(int iProfile, char *buf, size_t size)
{
	netData data;
	memset(&data, 0, sizeof(netData));
	data.asUint = 0xBADF00D;
	memset(&data.asString[4], 0, 124);
	sceUtilityGetNetParam(iProfile, 0/**Profile Name*/, &data);
	
	strncpy(buf, data.asString, size);
}
	

void CScreenHandler::StartScreen(Screen screen)
{
	m_UI->Initialize_Screen(m_CurrentScreen);
	
	switch(screen)
	{
		case PSPRADIO_SCREEN_PLAYLIST:
			if (m_UI && m_CurrentPlayListDir && m_CurrentPlayList && m_Sound)
			{
				Log(LOG_LOWLEVEL, "Displaying current playlist");
				m_CurrentPlayListDir->Clear();
				char *strFileName = NULL;
				strFileName = (char *)malloc(strlen(m_strCWD) + strlen("PlayLists") + 10);
				sprintf(strFileName, "%s/PlayLists", m_strCWD);
				m_CurrentPlayListDir->LoadDirectory(strFileName); //**//
				free(strFileName),strFileName = NULL;
				//m_CurrentPlayListDir->LoadDirectory(); //**//
				if (m_CurrentPlayListDir->Size() > 0)
				{
					Log(LOG_LOWLEVEL, "Loading Playlist file '%s'.", m_CurrentPlayListDir->GetCurrentURI());
					m_CurrentPlayList->Clear();
					m_CurrentPlayList->LoadPlayListURI(m_CurrentPlayListDir->GetCurrentURI());
				}

				m_UI->DisplayPLList(m_CurrentPlayListDir);
				m_CurrentPlayListSideSelection = PLAYLIST_LIST;
				/** tell ui of m_CurrentPlayListSideSelection change. */
				m_UI->OnCurrentPlayListSideSelectionChange(m_CurrentPlayListSideSelection); 

				if(m_CurrentPlayList->GetNumberOfSongs() > 0)
				{
					m_UI->DisplayPLEntries(m_CurrentPlayList);
					
					if (CPSPSound::PLAY == m_Sound->GetPlayState())
					{
						/** Populate m_CurrentMetaData */
						//don't until user starts it!
						//m_CurrentPlayList->GetCurrentSong(m_CurrentMetaData);
						m_UI->OnNewSongData(m_Sound->GetCurrentStream()->GetMetaData());
					}
				}
			}
			break;
			
		case PSPRADIO_SCREEN_SHOUTCAST_BROWSER:
			if (m_UI && m_CurrentPlayListDir && m_CurrentPlayList && m_Sound)
			{
				Log(LOG_LOWLEVEL, "StartScreen::PSPRADIO_SCREEN_SHOUTCAST_BROWSER");
				m_CurrentPlayListDir->Clear();
				char *strFileName = NULL;
				strFileName = (char *)malloc(strlen(m_strCWD) + strlen("SHOUTcast") + 10);
				sprintf(strFileName, "%s/SHOUTcast", m_strCWD);
				m_CurrentPlayListDir->LoadDirectory(strFileName); //**//
				free(strFileName),strFileName = NULL;
				if (m_CurrentPlayListDir->Size() > 0)
				{
					Log(LOG_LOWLEVEL, "Loading xml file '%s'.", m_CurrentPlayListDir->GetCurrentURI());
					m_CurrentPlayList->Clear();
					m_CurrentPlayList->LoadPlayListFromSHOUTcastXML(m_CurrentPlayListDir->GetCurrentURI());
				}
				
				m_UI->DisplayPLList(m_CurrentPlayListDir);
				m_CurrentPlayListSideSelection = PLAYLIST_LIST;
				/** tell ui of m_CurrentPlayListSideSelection change. */
				m_UI->OnCurrentPlayListSideSelectionChange(m_CurrentPlayListSideSelection); 

				if(m_CurrentPlayList->GetNumberOfSongs() > 0)
				{
					m_UI->DisplayPLEntries(m_CurrentPlayList);
					
					if (CPSPSound::PLAY == m_Sound->GetPlayState())
					{
						/** Populate m_CurrentMetaData */
						//don't until user starts it!
						//m_CurrentPlayList->GetCurrentSong(m_CurrentMetaData);
						m_UI->OnNewSongData(m_Sound->GetCurrentStream()->GetMetaData());
					}
				}
			}
			break;
			
		case PSPRADIO_SCREEN_OPTIONS:
			// Update network and UI options.  This is necesary the first time */
			PopulateOptionsData();
			m_UI->UpdateOptionsScreen(m_OptionsList, m_CurrentOptionIterator);
			break;
	}
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
			switch (m_CurrentScreen)
			{
				case CScreenHandler::PSPRADIO_SCREEN_PLAYLIST:
				case CScreenHandler::PSPRADIO_SCREEN_SHOUTCAST_BROWSER:
					if (iButtonMask & PSP_CTRL_START)		/** Go to Options screen */
					{
						// Enter option menu and store the current screen
						m_PreviousScreen = m_CurrentScreen;
						m_CurrentScreen  = PSPRADIO_SCREEN_OPTIONS;
						StartScreen(m_CurrentScreen);
					}
					else if (iButtonMask & PSP_CTRL_TRIANGLE) /** Cycle through screens (except options) */
					{
						m_CurrentScreen = (Screen)(m_CurrentScreen+1);
						/** Don't go to options screen with triangle;
							Also, as options screen is the last one in the list
							we cycle back to the first one */
						if (m_CurrentScreen == PSPRADIO_SCREEN_OPTIONS)
						{
							m_CurrentScreen = PSPRADIO_SCREEN_LIST_BEGIN;
						}
						StartScreen(m_CurrentScreen);
					}
					else
					{
						PlayListScreenInputHandler(iButtonMask);
					}
					break;
				case CScreenHandler::PSPRADIO_SCREEN_OPTIONS:
					if (iButtonMask & PSP_CTRL_START)	/** Get out of Options screen */
					{
						// Go back to where we were before entering the options menu
						m_CurrentScreen = m_PreviousScreen;
						StartScreen(m_CurrentScreen);
					}
					else
					{
						OptionsScreenInputHandler(iButtonMask);
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
	Log(LOG_VERYLOW, "OnHPRMReleased(): iHPRMMask=0x%x", iHPRMMask);
	if (m_Sound)
	{
		CPSPSound::pspsound_state playingstate = m_Sound->GetPlayState();

		if (iHPRMMask & PSP_HPRM_BACK)
		{
			m_CurrentPlayList->Prev();
			m_UI->DisplayPLEntries(m_CurrentPlayList);
		}
		else if (iHPRMMask & PSP_HPRM_FORWARD)
		{
			m_CurrentPlayList->Next();
			m_UI->DisplayPLEntries(m_CurrentPlayList);
		}

		else if (iHPRMMask & PSP_HPRM_PLAYPAUSE) 
		{
			switch(playingstate)
			{
				case CPSPSound::STOP:
				case CPSPSound::PAUSE:
					m_Sound->GetCurrentStream()->SetURI(m_CurrentPlayList->GetCurrentURI());
					m_UI->DisplayActiveCommand(CPSPSound::PLAY);
					m_Sound->Play();
					/** Populate m_CurrentMetaData */
					m_CurrentPlayList->GetCurrentSong(m_Sound->GetCurrentStream()->GetMetaData());
					//m_Sound->GetCurrentStream()->SetURI(m_CurrentPlayList->GetURI());
					m_UI->OnNewSongData(m_Sound->GetCurrentStream()->GetMetaData());
					break;
				case CPSPSound::PLAY:
					m_UI->DisplayActiveCommand(CPSPSound::STOP);
					Log(LOG_VERYLOW, "Calling Stop() on HPRM PLAY/PAUSE pressed; currently Playing.");
					m_Sound->Stop();
					break;
			}
		}
	}
};
