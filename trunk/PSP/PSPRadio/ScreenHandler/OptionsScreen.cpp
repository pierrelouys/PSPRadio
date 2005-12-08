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
#include <psppower.h>
#include "OptionsScreen.h"
#include "TextUI.h"
#include "GraphicsUI.h"
#include "SandbergUI.h" 
#include "SHOUTcastScreen.h"

#define ReportError pPSPApp->ReportError

enum OptionIDs
{
	OPTION_ID_NETWORK_PROFILES,
	OPTION_ID_WIFI_AUTOSTART,
	OPTION_ID_USB_ENABLE,
	OPTION_ID_USB_AUTOSTART,
	OPTION_ID_PLAYMODE,
	OPTION_ID_CPU_SPEED,
	OPTION_ID_LOG_LEVEL,
	OPTION_ID_UI,
	OPTION_ID_INITIAL_SCREEN,
	OPTION_ID_REFRESH_PLAYLISTS,
	OPTION_ID_SHOUTCAST_DN,
	OPTION_ID_SAVE_CONFIG,
	OPTION_ID_EXIT,
};

OptionsScreen::Options OptionsData[] = 
{
		/* ID						Option Name					Option State List			(active,selected,number-of)-states */
	{	OPTION_ID_NETWORK_PROFILES,	"WiFi",						{"Off","1","2","3","4"},		1,1,5		},
	{	OPTION_ID_WIFI_AUTOSTART,	"WiFi AutoStart",			{"No", "Yes"},					1,1,2		},
	{	OPTION_ID_USB_ENABLE,		"USB",						{"OFF","ON"},					1,1,2		},
	{	OPTION_ID_USB_AUTOSTART,	"USB AutoStart",			{"No", "Yes"},					1,1,2		},
	{	OPTION_ID_PLAYMODE,			"Play Mode",				{"Normal", "Single", "Repeat"},	1,1,3		},
	{	OPTION_ID_CPU_SPEED,		"CPU Speed",				{"111","222","266","333"},		2,2,4		},
	{	OPTION_ID_LOG_LEVEL,		"Log Level",				{"All","Verbose","Info","Errors","Off"},	1,1,5		},
	{	OPTION_ID_UI,				"User Interface",			{"Text","Graphics","3D"},		1,1,3		},
	{	OPTION_ID_INITIAL_SCREEN,	"Initial Screen",			{"PlayList","SHOUTcast.com","Options"}, 1,1,3 },
	{	OPTION_ID_REFRESH_PLAYLISTS,"Refresh Playlists",		{"Yes"},						0,1,1		},
	{	OPTION_ID_SHOUTCAST_DN,		"Get Latest SHOUTcast DB",	{"Yes"},						0,1,1		},
	{	OPTION_ID_SAVE_CONFIG,		"Save Options",				{"Yes"},						0,1,1		},
	{	OPTION_ID_EXIT,				"Exit PSPRadio",			{"Yes"},						0,1,1		},
	
	{  -1,  						"",							{""},							0,0,0		}
};

OptionsScreen::OptionsScreen(int Id, CScreenHandler *ScreenHandler):IScreen(Id, ScreenHandler)
{
	m_iNetworkProfile = 1;
	m_WifiAutoStart = false;
	m_USBAutoStart = false;
	LoadFromConfig();
}

/** Activate() is called on screen activation */
void OptionsScreen::Activate(IPSPRadio_UI *UI)
{
	IScreen::Activate(UI);

	// Update network and UI options.  This is necesary the first time */
	UpdateOptionsData();
	m_UI->UpdateOptionsScreen(m_OptionsList, m_CurrentOptionIterator);
}

void OptionsScreen::LoadFromConfig()
{
	CIniParser *pConfig = m_ScreenHandler->GetConfig();
	Log(LOG_INFO, "LoadFromConfig(): Loading Options from configuration file");
	
	if (pConfig)
	{
		/** CPU FREQ */
		int iRet = 0;
		switch(pConfig->GetInteger("SYSTEM:CPUFREQ"))
		{
			case 111:
				iRet = scePowerSetClockFrequency(111, 111, 55);
				break;
			case 222:
				iRet = scePowerSetClockFrequency(222, 222, 111);
				break;
			case 265:
			case 266:
				iRet = scePowerSetClockFrequency(266, 266, 133);
				break;
			case 333:
				iRet = scePowerSetClockFrequency(333, 333, 166);
				break;
			default:
				iRet = -1;
				break;
		}
		if (0 != iRet) 
		{
			Log(LOG_ERROR, "LoadFromConfig(): Unable to change CPU/BUS Speed to selection %d",
					pConfig->GetInteger("SYSTEM:CPUFREQ"));
		}
		/** CPU FREQ */
		
		/** LOGLEVEL */
		pLogging->SetLevel((loglevel_enum)pConfig->GetInteger("DEBUGGING:LOGLEVEL", 100));
		/** LOGLEVEL */
		
		/** WIFI PROFILE */
		m_iNetworkProfile = pConfig->GetInteger("WIFI:PROFILE", 1);
		/** WIFI PROFILE */
		
		/** OPTION_ID_WIFI_AUTOSTART */
		if (1 == pConfig->GetInteger("WIFI:AUTOSTART", 0))
		{
			m_WifiAutoStart = true;
			Log(LOG_INFO, "LoadFromConfig(): WIFI AUTOSTART SET: Enabling Network; using profile: %d", 	
				m_iNetworkProfile);
			//m_ScreenHandler->GetScreen(CScreenHandler::PSPRADIO_SCREEN_OPTIONS)->Activate(m_UI);
			//((OptionsScreen *)m_ScreenHandler->GetScreen(CScreenHandler::PSPRADIO_SCREEN_OPTIONS))->Start_Network(m_Config->GetInteger("WIFI:PROFILE", 1));
			Start_Network();
			/** Go back after starting network */
			//m_ScreenHandler->GetScreen(iInitialScreen)->Activate(m_UI);
		}
		else
		{
			m_WifiAutoStart = false;
			Log(LOG_INFO, "LoadFromConfig(): WIFI AUTOSTART Not Set, Not starting network");
		}
		/** OPTION_ID_WIFI_AUTOSTART */
		
		/** OPTION_ID_INITIAL_SCREEN */
		// This is loaded in PSPRadio.cpp/ScreenHandler.cpp 
		/** OPTION_ID_INITIAL_SCREEN */
		
		/** OPTION_ID_USB_AUTOSTART */
		if (1 == pConfig->GetInteger("SYSTEM:USB_AUTOSTART", 0))
		{
			m_USBAutoStart = true;
			Log(LOG_INFO, "LoadFromConfig(): USB_AUTOSTART SET: Enabling USB");
			pPSPApp->EnableUSB();
		}
		else
		{
			m_USBAutoStart = false;
			Log(LOG_INFO, "LoadFromConfig(): USB_AUTOSTART Not Set");
		}
		/** OPTION_ID_USB_AUTOSTART */
		


	}
	else
	{
		Log(LOG_ERROR, "LoadFromConfig(): Error: no config object.");
	}
}

void OptionsScreen::SaveToConfigFile()
{
	CIniParser *pConfig = m_ScreenHandler->GetConfig();
	
	if (pConfig)
	{
		pConfig->SetInteger("DEBUGGING:LOGLEVEL", pLogging->GetLevel());
		pConfig->SetInteger("SYSTEM:CPUFREQ", scePowerGetCpuClockFrequency());
		pConfig->SetInteger("WIFI:PROFILE", m_iNetworkProfile);
		switch(m_ScreenHandler->GetCurrentUI())
		{
			case CScreenHandler::UI_TEXT:
				pConfig->SetString("UI:MODE", "Text");
				break;
			case CScreenHandler::UI_GRAPHICS:
				pConfig->SetString("UI:MODE", "Graphics");
				break;
			case CScreenHandler::UI_3D:
				pConfig->SetString("UI:MODE", "3D");
				break;
		}
		/** OPTION_ID_INITIAL_SCREEN */
		pConfig->SetInteger("SYSTEM:INITIAL_SCREEN", m_ScreenHandler->GetInitialScreen());
		/** OPTION_ID_INITIAL_SCREEN */

		/** OPTION_ID_WIFI_AUTOSTART */
		pConfig->SetInteger("WIFI:AUTOSTART", m_WifiAutoStart);
		/** OPTION_ID_WIFI_AUTOSTART */
		
		/** OPTION_ID_USB_AUTOSTART */
		pConfig->SetInteger("SYSTEM:USB_AUTOSTART", m_USBAutoStart);
		/** OPTION_ID_USB_AUTOSTART */

		pConfig->Save();
	}
	else
	{
		Log(LOG_ERROR, "SaveToConfigFile(): Error: no config object.");
	}
}

/** This populates and updates the option data */
void OptionsScreen::UpdateOptionsData()
{
	Options Option;
	
	list<Options>::iterator		OptionIterator;

	while(m_OptionsList.size())
	{
		// Release memory allocated for network profile names
		OptionIterator = m_OptionsList.begin();
		if ((*OptionIterator).Id == OPTION_ID_NETWORK_PROFILES)
		{
			for (int i = 0; i < (*OptionIterator).iNumberOfStates; i++)
			{
				free((*OptionIterator).strStates[i]);
			}
		}
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
				Option.iNumberOfStates = pPSPApp->GetNumberOfNetworkProfiles();
				char *NetworkName = NULL;
				Option.strStates[0] = strdup("Off");
				for (int i = 1; i <= Option.iNumberOfStates; i++)
				{
					NetworkName = (char*)malloc(128);
					pPSPApp->GetNetworkProfileName(i, NetworkName, 128);
					Option.strStates[i] = NetworkName;
				}
				Option.iActiveState = (pPSPApp->IsNetworkEnabled()==true)?(m_iNetworkProfile+1):1;
				break;
			}
				
			case OPTION_ID_USB_ENABLE:
				Option.iActiveState = (pPSPApp->IsUSBEnabled()==true)?2:1;
				break;
				
			case OPTION_ID_PLAYMODE:
				Option.iActiveState = (m_ScreenHandler->GetPlayMode())+1;
				break;
				
			case OPTION_ID_CPU_SPEED:
				switch(scePowerGetCpuClockFrequency())
				{
					case 111:
						Option.iActiveState = 1;
						break;
					case 222:
						Option.iActiveState = 2;
						break;
					case 265:
					case 266:
						Option.iActiveState = 3;
						break;
					case 333:
						Option.iActiveState = 4;
						break;
					default:
						Log(LOG_ERROR, "CPU speed unrecognized: %dMHz", 
							scePowerGetCpuClockFrequency());
						break;
				}
				break;
		
			case OPTION_ID_LOG_LEVEL:
				switch(pLogging->GetLevel())
				{
					case 0:
					case LOG_VERYLOW:
						Option.iActiveState = 1;
						break;
					case LOG_LOWLEVEL:
						Option.iActiveState = 2;
						break;
					case LOG_INFO:
						Option.iActiveState = 3;
						break;
					case LOG_ERROR:
						Option.iActiveState = 4;
						break;
					case LOG_ALWAYS:
					default:
						Option.iActiveState = 5;
						break;
				}
				break;
			
			case OPTION_ID_UI:
				Option.iActiveState = m_ScreenHandler->GetCurrentUI() + 1;
				break;
			
			case OPTION_ID_INITIAL_SCREEN:
				Option.iActiveState = m_ScreenHandler->GetInitialScreen() + 1;
				break;

			case OPTION_ID_WIFI_AUTOSTART:
				Option.iActiveState = (m_WifiAutoStart==true)?2:1;
				break;
				
			case OPTION_ID_USB_AUTOSTART:
				Option.iActiveState = (m_USBAutoStart==true)?2:1;
				break;

		}
		
		m_OptionsList.push_back(Option);
	}
	
	m_CurrentOptionIterator = m_OptionsList.begin();

}

void OptionsScreen::OnOptionActivation()
{
	bool fOptionActivated = false;
	static time_t	timeLastTime = 0;
	time_t timeNow = clock() / (1000*1000); /** clock is in microseconds */
	int iSelectionBase0 = (*m_CurrentOptionIterator).iSelectedState - 1;
	int iSelectionBase1 = (*m_CurrentOptionIterator).iSelectedState;
	
	switch ((*m_CurrentOptionIterator).Id)
	{
		case OPTION_ID_NETWORK_PROFILES:
			m_iNetworkProfile = iSelectionBase0;
			if (m_iNetworkProfile > 0) /** Enable */
			{
				m_ScreenHandler->GetSound()->Stop(); /** Stop stream if playing */
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
			
		case OPTION_ID_WIFI_AUTOSTART:
			m_WifiAutoStart = (iSelectionBase1 == 2)?true:false;
			fOptionActivated = true;
			break;

		case OPTION_ID_USB_ENABLE:
			if (iSelectionBase1 == 2) /** Enable */
			{
				pPSPApp->EnableUSB();
				if (true == pPSPApp->IsUSBEnabled())
				{
					fOptionActivated = true;
				}
			}
			else /** Disable */
			{
				int iRet = pPSPApp->DisableUSB();
				if (-1 == iRet)
				{
					ReportError("USB Busy, try again later.");
				}
				if (false == pPSPApp->IsUSBEnabled())
				{
					fOptionActivated = true;
				}
			}
			break;
			
		case OPTION_ID_USB_AUTOSTART:
			m_USBAutoStart = (iSelectionBase1 == 2)?true:false;
			fOptionActivated = true;
			break;
		
		case OPTION_ID_PLAYMODE:
			m_ScreenHandler->SetPlayMode((playmodes)iSelectionBase0);
			fOptionActivated = true;
			break;
			
		case OPTION_ID_CPU_SPEED:
		{
			int iRet = -1;
			switch (iSelectionBase1)
			{
				case 1: /* 111 */
					iRet = scePowerSetClockFrequency(111, 111, 55);
					break;
				case 2: /* 222 */
					iRet = scePowerSetClockFrequency(222, 222, 111);
					break;
				case 3: /* 266 */
					iRet = scePowerSetClockFrequency(266, 266, 133);
					break;
				case 4: /* 333 */
					iRet = scePowerSetClockFrequency(333, 333, 166);
					break;
			}
		    if (0 == iRet) 
			{
				fOptionActivated = true;
			}
			else
			{
				Log(LOG_ERROR, "Unable to change CPU/BUS Speed to selection %d",
						(*m_CurrentOptionIterator).iSelectedState);
			}
			break;
		}
		
		case OPTION_ID_LOG_LEVEL:
			switch(iSelectionBase1)
			{
				case 1:
					pLogging->SetLevel(LOG_VERYLOW);
					break;
				case 2:
					pLogging->SetLevel(LOG_LOWLEVEL);
					break;
				case 3:
					pLogging->SetLevel(LOG_INFO);
					break;
				case 4:
					pLogging->SetLevel(LOG_ERROR);
					break;
				case 5:
					pLogging->SetLevel(LOG_ALWAYS);
					break;
			}
			Log(LOG_ALWAYS, "Log Level Changed to (%d)", pLogging->GetLevel());
			fOptionActivated = true;
			break;
			
		case OPTION_ID_UI:
			m_ScreenHandler->StartUI((CScreenHandler::UIs)iSelectionBase0);
			fOptionActivated = true;
			break;
			
		case OPTION_ID_INITIAL_SCREEN:
			m_ScreenHandler->SetInitialScreen((CScreenHandler::Screen)iSelectionBase0);
			fOptionActivated = true;
			break;
			
		case OPTION_ID_REFRESH_PLAYLISTS:
			m_UI->DisplayMessage("Refreshing Playlists");
			m_ScreenHandler->GetSound()->Stop(); /** Stop stream if playing */
			((PlayListScreen*)m_ScreenHandler->GetScreen(CScreenHandler::PSPRADIO_SCREEN_PLAYLIST))->LoadLists();
			m_UI->DisplayMessage("Done");
			fOptionActivated = true;
			break;
			
		case OPTION_ID_SHOUTCAST_DN:
			if ( (timeNow - timeLastTime) > 60 ) /** Only allow to refresh shoutcast once a minute max! */
			{
				m_UI->DisplayMessage("Downloading latest SHOUTcast Database. . .");
				m_ScreenHandler->GetSound()->Stop(); /** Stop stream if playing */
				if (true == m_ScreenHandler->DownloadSHOUTcastDB())
				{
					timeLastTime = timeNow; /** Only when successful */
					((SHOUTcastScreen*)m_ScreenHandler->GetScreen(CScreenHandler::PSPRADIO_SCREEN_SHOUTCAST_BROWSER))->LoadLists();
				}
			}
			else
			{
				m_UI->DisplayErrorMessage("Wait a minute before re-downloading, thanks.");
			}		
			fOptionActivated = false;
			break;	
			
		case OPTION_ID_SAVE_CONFIG:
			m_UI->DisplayMessage("Saving Configuration Options");
			Log(LOG_INFO, "User selected to save config file.");
			SaveToConfigFile();
			m_UI->DisplayMessage("Done");
			fOptionActivated = true;
			break;
			
		case OPTION_ID_EXIT:
			Log(LOG_ALWAYS, "User selected to Exit.");
			pPSPApp->SendEvent(EID_EXIT_SELECTED, NULL, SID_SCREENHANDLER);
			break;
			
	}
	
	if (true == fOptionActivated)	
	{
		(*m_CurrentOptionIterator).iActiveState = (*m_CurrentOptionIterator).iSelectedState;
		m_UI->UpdateOptionsScreen(m_OptionsList, m_CurrentOptionIterator);
	}
}

int OptionsScreen::Start_Network(int iProfile)
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
		if (pPSPApp->IsNetworkEnabled())
		{
			if (m_UI) 
				m_UI->DisplayMessage_DisablingNetwork();

			Log(LOG_INFO, "Triangle Pressed. Restarting networking...");
			pPSPApp->DisableNetwork();
			sceKernelDelayThread(500000);  
		}
		
		if (m_UI) 
			m_UI->DisplayMessage_EnablingNetwork();

		if (pPSPApp->EnableNetwork(abs(m_iNetworkProfile)) == 0)
		{
			if (m_UI) 
				m_UI->DisplayMessage_NetworkReady(pPSPApp->GetMyIP());
		}
		else
		{
			if (m_UI) 
				m_UI->DisplayMessage_DisablingNetwork();
		}

		if (m_UI) 
			m_UI->DisplayMessage_NetworkReady(pPSPApp->GetMyIP());
		Log(LOG_INFO, "Networking Enabled, IP='%s'...", pPSPApp->GetMyIP());
		
		Log(LOG_INFO, "Enabling Network: Done. IP='%s'", pPSPApp->GetMyIP());
		
	}
	else
	{
		ReportError("The Network Switch is OFF, Cannot Start Network.");
	}
	
	return 0;
}	

int OptionsScreen::Stop_Network()
{
	if (pPSPApp->IsNetworkEnabled())
	{
		m_UI->DisplayMessage_DisablingNetwork();

		Log(LOG_INFO, "Disabling network...");
		pPSPApp->DisableNetwork();
		sceKernelDelayThread(500000);  
	}
	return 0;
}

void OptionsScreen::InputHandler(int iButtonMask)
{
	if (iButtonMask & PSP_CTRL_UP)
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

