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


CScreenHandler::CScreenHandler(IPSPRadio_UI *UI, CIniParser *Config, CPSPSound *Sound)
{
	m_CurrentScreen = PSPRADIO_SCREEN_PLAYLIST;
	m_iNetworkProfile = 1;
	m_NetworkStarted  = false;

	SetUp(UI, Config, Sound, NULL, NULL, NULL);
}

void CScreenHandler::SetUp(IPSPRadio_UI *UI, CIniParser *Config, CPSPSound *Sound,
							CPlayList *CurrentPlayList, CDirList  *CurrentPlayListDir, CPlayList::songmetadata *CurrentMetaData)
{	
	m_UI = UI;
	m_Config = Config;
	m_Sound = Sound;
	m_CurrentPlayList = CurrentPlayList;
	m_CurrentPlayListDir = CurrentPlayListDir;
	m_CurrentMetaData = CurrentMetaData;
}

int CScreenHandler::Setup_Network()
{
	m_iNetworkProfile = m_Config->GetInteger("WIFI:PROFILE", 1);
	
	if (m_iNetworkProfile < 1)
	{
		m_iNetworkProfile = 1;
		Log(LOG_ERROR, "Network Profile in m_Config file is invalid. Network profiles start from 1.");
	}
	
	if (1 == m_Config->GetInteger("WIFI:AUTOSTART", 0))
	{
		m_UI->DisplayMessage_EnablingNetwork();
		Log(LOG_INFO, "WIFI AUTOSTART SET: Enabling Network; using profile: %d", m_iNetworkProfile);
		pPSPApp->EnableNetwork(m_iNetworkProfile);
		m_UI->DisplayMessage_NetworkReady(pPSPApp->GetMyIP());
		Log(LOG_INFO, "Enabling Network: Done. IP='%s'", pPSPApp->GetMyIP());
		m_NetworkStarted = 1;
	}
	else
	{
		Log(LOG_INFO, "WIFI AUTOSTART Not Set, Not starting network");
		DisplayCurrentNetworkSelection();
	}
	
	return 0;
}	

void CScreenHandler::DisplayCurrentNetworkSelection()
{
	netData data;
	memset(&data, 0, sizeof(netData));
	data.asUint = 0xBADF00D;
	memset(&data.asString[4], 0, 124);
	sceUtilityGetNetParam(m_iNetworkProfile, 0/**Profile Name*/, &data);
	
	m_UI->DisplayMessage_NetworkSelection(m_iNetworkProfile, data.asString);
	Log(LOG_INFO, "Current Network Profile Selection: %d Name: '%s'", m_iNetworkProfile, data.asString);
}
	
void CScreenHandler::PlayListScreenInputHandler(int iButtonMask)
{
	Log(LOG_VERYLOW, "OnButtonReleased(): iButtonMask=0x%x", iButtonMask);
	if (m_Sound)
	{
		CPSPSound::pspsound_state playingstate = m_Sound->GetPlayState();
		
		
		if (iButtonMask & PSP_CTRL_LEFT && !m_NetworkStarted)
		{
			m_iNetworkProfile --;
			m_iNetworkProfile %= (pPSPApp->GetNumberOfNetworkProfiles());
			if (m_iNetworkProfile < 1)
				m_iNetworkProfile = 1;
			
			DisplayCurrentNetworkSelection();
		}
		else if (iButtonMask & PSP_CTRL_RIGHT && !m_NetworkStarted)
		{
			m_iNetworkProfile ++;
			m_iNetworkProfile %= (pPSPApp->GetNumberOfNetworkProfiles());
			//if (m_iNetworkProfile < 1)
			//	m_iNetworkProfile = 1;
			
			DisplayCurrentNetworkSelection();
		}
		else if (iButtonMask & PSP_CTRL_LTRIGGER)
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
					m_Sound->GetStream()->SetFile(m_CurrentPlayList->GetCurrentFileName());
					m_Sound->Play();
					break;
				case CPSPSound::PLAY:
					/** No pausing for URLs, only for Files(local) */
					if (CPSPSoundStream::STREAM_TYPE_FILE == m_Sound->GetStream()->GetType())
					{
						m_Sound->GetStream()->SetFile(m_CurrentPlayList->GetCurrentFileName());
						m_UI->DisplayActiveCommand(CPSPSound::PAUSE);
						m_Sound->Pause();
					}
					else
					{
						/** If currently playing a stream, and the user presses play, then start the 
						currently selected stream! */
						if (CPSPSoundStream::STREAM_STATE_OPEN == m_Sound->GetStream()->GetState())
						{
							m_Sound->Stop();
						}
					}
					break;
			}
		}
		else if (iButtonMask & PSP_CTRL_SQUARE)
		{
			if (playingstate == CPSPSound::PLAY || playingstate == CPSPSound::PAUSE)
			{
				m_Sound->Stop();
			}
		}
		else if (iButtonMask & PSP_CTRL_TRIANGLE && !m_NetworkStarted)
		{
			if (sceWlanGetSwitchState() != 0)
			{
				pPSPApp->CantExit();

				m_UI->DisplayActiveCommand(CPSPSound::STOP);
				m_Sound->Stop();
				sceKernelDelayThread(50000);  
				
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
				Log(LOG_INFO, "Triangle Pressed. Networking Enabled, IP='%s'...", pPSPApp->GetMyIP());
				
				m_NetworkStarted = true;
				
				pPSPApp->CanExit();
			}
			else
			{
				ReportError("The Network Switch is OFF, Cannot Start Network.");
			}
			
		}
	}
};

void CScreenHandler::OnPlayStateChange(CPSPSound::pspsound_state NewPlayState)
{
	static CPSPSound::pspsound_state OldPlayState = CPSPSound::STOP;
	
	switch(OldPlayState)
	{
		case CPSPSound::STOP:
			switch(NewPlayState)
			{
				case CPSPSound::PLAY:
					m_UI->DisplayActiveCommand(CPSPSound::PLAY);
					/** Populate m_CurrentMetaData */
					m_CurrentPlayList->GetCurrentSong(m_CurrentMetaData);
					m_UI->OnNewSongData(m_CurrentMetaData);
					break;
				
				case CPSPSound::STOP:
				case CPSPSound::PAUSE:
				default:
					break;
			}
			break;
		
		case CPSPSound::PLAY:
			switch(NewPlayState)
			{
				case CPSPSound::STOP:
					m_UI->DisplayActiveCommand(CPSPSound::STOP);
					//m_Sound->Stop();
					m_Sound->GetStream()->SetFile(m_CurrentPlayList->GetCurrentFileName());
					/** Populate m_CurrentMetaData */
					m_CurrentPlayList->GetCurrentSong(m_CurrentMetaData);
					m_Sound->Play();
					break;
					
				case CPSPSound::PLAY:
				case CPSPSound::PAUSE:
				default:
					break;
			}
			break;
		
		case CPSPSound::PAUSE:
		default:
			break;
	}
}
