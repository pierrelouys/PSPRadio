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
#include "ScreenHandler.h"
#include "DirList.h"
#include "PlayList.h"
#include "TextUI.h"
#include "GraphicsUI.h"
#include "SandbergUI.h" 


#define ReportError pPSPApp->ReportError

enum OptionIDs
{
	OPTION_ID_NETWORK_PROFILES,
	OPTION_ID_NETWORK_ENABLE,
	OPTION_ID_USB_ENABLE,
	OPTION_ID_CPU_SPEED
};

CScreenHandler::Options OptionsData[] = 
{
		/* ID						Option Name					Option State List			(active,selected,number-of)-states */
	{	OPTION_ID_NETWORK_PROFILES,	"Select Network Profile",	{"1","2","3","4","5"},			1,1,5		},
	{	OPTION_ID_NETWORK_ENABLE,	"Start Network",			{"OFF","ON"},					1,1,2		},
	{	OPTION_ID_USB_ENABLE,		"USB",						{"OFF","ON"},					1,1,2		},
	{	OPTION_ID_CPU_SPEED,		"CPU Speed",				{"222","266","333"},			1,1,3		},
	
	{  -1,  						"",							{""},							0,0,0		}
};

CScreenHandler::CScreenHandler(IPSPRadio_UI *UI, CIniParser *Config, CPSPSound *Sound)
{
	m_CurrentScreen = PSPRADIO_SCREEN_PLAYLIST;
	m_iNetworkProfile = 1;
	m_NetworkStarted  = false;
	m_RequestOnPlayOrStop = NOTHING;
	m_CurrentMetaData = &CurrentSoundStream.m_CurrentMetaData;
	
	SetUp(UI, Config, Sound, NULL, NULL);
}

void CScreenHandler::SetUp(IPSPRadio_UI *UI, CIniParser *Config, CPSPSound *Sound,
							CPlayList *CurrentPlayList, CDirList  *CurrentPlayListDir)
{	
	m_UI = UI;
	m_Config = Config;
	m_Sound = Sound;
	m_CurrentPlayList = CurrentPlayList;
	m_CurrentPlayListDir = CurrentPlayListDir;
	
	PopulateOptionsData();
	StartScreen(m_CurrentScreen);
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
		pPSPApp->CantExit();

		//m_UI->DisplayActiveCommand(CPSPSound::STOP);
		//m_Sound->Stop();
		//sceKernelDelayThread(50000);  
		
		if (m_NetworkStarted)
		{
			m_UI->DisplayMessage_DisablingNetwork();

			Log(LOG_INFO, "Triangle Pressed. Restarting networking...");
			pPSPApp->DisableNetwork();
			sceKernelDelayThread(500000);  
		}
		
		m_UI->DisplayMessage_EnablingNetwork();

		pPSPApp->EnableNetwork(abs(m_iNetworkProfile));
		
		m_UI->DisplayMessage_NetworkReady(pPSPApp->GetMyIP());
		Log(LOG_INFO, "Networking Enabled, IP='%s'...", pPSPApp->GetMyIP());
		
		m_NetworkStarted = true;
		Log(LOG_INFO, "Enabling Network: Done. IP='%s'", pPSPApp->GetMyIP());
		
		pPSPApp->CanExit();
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
	//m_UI->DisplayMessage_NetworkSelection(m_iNetworkProfile, data.asString);
	//Log(LOG_INFO, "Current Network Profile Selection: %d Name: '%s'", m_iNetworkProfile, data.asString);
}
	

void CScreenHandler::StartScreen(Screen screen)
{
	m_UI->Initialize_Screen(m_CurrentScreen);
	
	switch(screen)
	{
		case PSPRADIO_SCREEN_PLAYLIST:
			if (m_UI && m_CurrentPlayListDir && m_CurrentPlayList && m_Sound && m_CurrentMetaData)
			{
				Log(LOG_LOWLEVEL, "Displaying current playlist");
				m_UI->DisplayPLList(m_CurrentPlayListDir);
				
				if(m_CurrentPlayList->GetNumberOfSongs() > 0)
				{
					m_UI->DisplayPLEntries(m_CurrentPlayList);
					
					if (CPSPSound::PLAY == m_Sound->GetPlayState())
					{
						/** Populate m_CurrentMetaData */
						//don't until user starts it!
						//m_CurrentPlayList->GetCurrentSong(m_CurrentMetaData);
						m_UI->OnNewSongData(m_CurrentMetaData);
					}
				}
			}
			break;
			
		case PSPRADIO_SCREEN_OPTIONS:
			m_UI->UpdateOptionsScreen(m_OptionsList, m_CurrentOptionIterator);
			break;
	}
}


void CScreenHandler::PopulateOptionsData()
{
	Options Option;
	
	while(m_OptionsList.size())
	{
		m_OptionsList.pop_front();
	}
	
	
	for (int iOptNo=0;; iOptNo++)
	{
		if (-1 == OptionsData[iOptNo].Id)
			break;
	
		Option.Id = OptionsData[iOptNo].Id;
		sprintf(Option.strName, 	OptionsData[iOptNo].strName);
		memcpy(Option.strStates, OptionsData[iOptNo].strStates, sizeof(char*)*OptionsData[iOptNo].iNumberOfStates);
		Option.iActiveState		= OptionsData[iOptNo].iActiveState;
		Option.iSelectedState	= OptionsData[iOptNo].iSelectedState;
		Option.iNumberOfStates	= OptionsData[iOptNo].iNumberOfStates;
		
		/** Modify from data table */
		switch(iOptNo)
		{
			case OPTION_ID_NETWORK_PROFILES:
			{
				Option.iNumberOfStates = pPSPApp->GetNumberOfNetworkProfiles() - 1;
				/** allocate memory and lose it forever.. */
				char *NetworkName = NULL;
				for (int i = 0; i < Option.iNumberOfStates; i++)
				{
					NetworkName = (char*)malloc(128);
					GetNetworkProfileName(i+1, NetworkName, 128);
					Option.strStates[i] = NetworkName;
				}
				break;
			}
				
			case OPTION_ID_NETWORK_ENABLE:
				Option.iActiveState = (pPSPApp->IsNetworkEnabled()==true)?2:1;
				break;
			
			case OPTION_ID_USB_ENABLE:
				Option.iActiveState = (pPSPApp->IsUSBEnabled()==true)?2:1;
				break;
				
			case OPTION_ID_CPU_SPEED:
			default:
				break;
		
		}
		
		
		m_OptionsList.push_back(Option);
	}
	
	m_CurrentOptionIterator = m_OptionsList.begin();
}

void CScreenHandler::OptionsScreenInputHandler(int iButtonMask)
{
//	Options Option;
	
	if (iButtonMask & PSP_CTRL_TRIANGLE)
	{
		m_CurrentScreen = (Screen)(m_CurrentScreen+1);
		if (m_CurrentScreen == PSPRADIO_SCREEN_LIST_END)
		{
			m_CurrentScreen = PSPRADIO_SCREEN_LIST_BEGIN;
		}
		StartScreen(m_CurrentScreen);
	}
	else if (iButtonMask & PSP_CTRL_UP)
	{
		if(m_CurrentOptionIterator == m_OptionsList.begin())
			m_CurrentOptionIterator = m_OptionsList.end();
		m_CurrentOptionIterator--;
		m_UI->UpdateOptionsScreen(m_OptionsList, m_CurrentOptionIterator);
	}
	else if (iButtonMask & PSP_CTRL_DOWN)
	{
		m_CurrentOptionIterator++;
		if(m_CurrentOptionIterator == m_OptionsList.end())
			m_CurrentOptionIterator = m_OptionsList.begin();
		
		m_UI->UpdateOptionsScreen(m_OptionsList, m_CurrentOptionIterator);
	}
	else if (iButtonMask & PSP_CTRL_LEFT)
	{
		if ((*m_CurrentOptionIterator).iSelectedState > 1)
		{
			(*m_CurrentOptionIterator).iSelectedState--;
		
			//OnOptionChange();
			m_UI->UpdateOptionsScreen(m_OptionsList, m_CurrentOptionIterator);
		}
	}
	else if (iButtonMask & PSP_CTRL_RIGHT)
	{
		if ((*m_CurrentOptionIterator).iSelectedState < (*m_CurrentOptionIterator).iNumberOfStates)
		{
			(*m_CurrentOptionIterator).iSelectedState++;
			
			//OnOptionChange();
			m_UI->UpdateOptionsScreen(m_OptionsList, m_CurrentOptionIterator);
		}
	}
	else if ( (iButtonMask & PSP_CTRL_CROSS) || (iButtonMask & PSP_CTRL_CIRCLE) )
	{
		OnOptionActivation();
	}
}

//	OPTION_ID_NETWORK_PROFILES,
//	OPTION_ID_NETWORK_ENABLE,
//	OPTION_ID_USB_ENABLE,
//	OPTION_ID_CPU_SPEED
void CScreenHandler::OnOptionActivation()
{
	bool fOptionActivated = false;
	
	switch ((*m_CurrentOptionIterator).Id)
	{
		case OPTION_ID_NETWORK_PROFILES:
			m_iNetworkProfile = (*m_CurrentOptionIterator).iSelectedState;
			fOptionActivated = true;
			break;
		case OPTION_ID_NETWORK_ENABLE:
			if ((*m_CurrentOptionIterator).iSelectedState == 2) /** Enable */
			{
				Start_Network();
				if (true == pPSPApp->IsNetworkEnabled())
				{
					fOptionActivated = true;
				}
			}
			else /** Disable */
			{
				Stop_Network();
				fOptionActivated = true;
			}
			break;
		case OPTION_ID_USB_ENABLE:
			if ((*m_CurrentOptionIterator).iSelectedState == 2) /** Enable */
			{
				pPSPApp->EnableUSB();
				if (true == pPSPApp->IsUSBEnabled())
				{
					fOptionActivated = true;
				}
			}
			else /** Disable */
			{
				pPSPApp->DisableUSB();
				fOptionActivated = true;
			}
			break;
	}
	
	if (true == fOptionActivated)	
	{
		(*m_CurrentOptionIterator).iActiveState = (*m_CurrentOptionIterator).iSelectedState;
		m_UI->UpdateOptionsScreen(m_OptionsList, m_CurrentOptionIterator);
	}
}

void CScreenHandler::PlayListScreenInputHandler(int iButtonMask)
{
	CPSPSound::pspsound_state playingstate = m_Sound->GetPlayState();
	Log(LOG_VERYLOW, "OnButtonReleased(): iButtonMask=0x%x", iButtonMask);
	
		
	if (iButtonMask & PSP_CTRL_LTRIGGER)
	{
		m_CurrentPlayList->Prev();
		m_UI->DisplayPLEntries(m_CurrentPlayList);
	}
	else if (iButtonMask & PSP_CTRL_RTRIGGER)
	{
		m_CurrentPlayList->Next();
		m_UI->DisplayPLEntries(m_CurrentPlayList);
	}
	else if (iButtonMask & PSP_CTRL_UP)
	{
		m_CurrentPlayListDir->Prev();
		m_UI->DisplayPLList(m_CurrentPlayListDir);
		m_CurrentPlayList->Clear();
		m_CurrentPlayList->LoadPlayListURI(m_CurrentPlayListDir->GetCurrentURI());
		m_UI->DisplayPLEntries(m_CurrentPlayList);
	}
	else if (iButtonMask & PSP_CTRL_DOWN)
	{
		m_CurrentPlayListDir->Next();
		m_UI->DisplayPLList(m_CurrentPlayListDir);
		m_CurrentPlayList->Clear();
		m_CurrentPlayList->LoadPlayListURI(m_CurrentPlayListDir->GetCurrentURI());
		m_UI->DisplayPLEntries(m_CurrentPlayList);
	}
	else if (iButtonMask & PSP_CTRL_CROSS || iButtonMask & PSP_CTRL_CIRCLE) 
	{
		switch(playingstate)
		{
			case CPSPSound::STOP:
			case CPSPSound::PAUSE:
				CurrentSoundStream.SetURI(m_CurrentPlayList->GetCurrentURI());
				m_Sound->Play();
				break;
			case CPSPSound::PLAY:
				/** No pausing for URLs, only for Files(local) */
				if (CPSPSoundStream::STREAM_TYPE_FILE == CurrentSoundStream.GetType())
				{
					CurrentSoundStream.SetURI(m_CurrentPlayList->GetCurrentURI());
					m_UI->DisplayActiveCommand(CPSPSound::PAUSE);
					m_Sound->Pause();
				}
				else
				{
					/** If currently playing a stream, and the user presses play, then start the 
					currently selected stream! */
					/** We do this by stopping the stream, and asking the handler to start playing
					when the stream stops. */
					if (CPSPSoundStream::STREAM_STATE_OPEN == CurrentSoundStream.GetState())
					{
						/** If the new stream is different than the current, only then stop-"restart" */
						if (0 != strcmp(m_CurrentMetaData->strURI, m_CurrentPlayList->GetCurrentURI()))
						{
							Log(LOG_VERYLOW, "Calling Stop() at InputHandler, X or O pressed, and was playing. Also setting  request to play.");
							m_Sound->Stop();
							m_RequestOnPlayOrStop = PLAY;
						}
						else
						{
							Log(LOG_VERYLOW, "Not Stopping/Restarting, as the selected stream == current stream");
						}
					}
				}
				break;
		}
	}
	else if (iButtonMask & PSP_CTRL_SQUARE)
	{
		if (playingstate == CPSPSound::PLAY || playingstate == CPSPSound::PAUSE)
		{
			Log(LOG_VERYLOW, "Calling Stop() at InputHandler, [] pressed.");
			m_Sound->Stop();
		}
	}
	else if (iButtonMask & PSP_CTRL_TRIANGLE)
	{
		m_CurrentScreen = (Screen)(m_CurrentScreen+1);
		if (m_CurrentScreen == PSPRADIO_SCREEN_LIST_END)
		{
			m_CurrentScreen = PSPRADIO_SCREEN_LIST_BEGIN;
		}
		StartScreen(m_CurrentScreen);
	}
};
