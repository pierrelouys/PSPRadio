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
#include <UI_Interface.h>
#include "PSPRadio.h"
#include "SHOUTcastScreen.h"
#include "LocalFilesScreen.h"
#include "OptionsScreen.h"
#include "Main.h"

#define ReportError pPSPApp->ReportError

enum OptionIDs
{
	OPTION_ID_NETWORK_PROFILES,
	OPTION_ID_WIFI_AUTOSTART,
	OPTION_ID_USB_ENABLE,
	OPTION_ID_PLAYMODE,
	OPTION_ID_CPU_SPEED,
	OPTION_ID_LOG_LEVEL,
	OPTION_ID_SKIN,
	OPTION_ID_INITIAL_SCREEN,
	OPTION_ID_REFRESH_PLAYLISTS,
	OPTION_ID_SHOUTCAST_DN,
	OPTION_ID_PLUGINS_MENU,
	OPTION_ID_SAVE_CONFIG,
	OPTION_ID_EXIT,
};

OptionsScreen::Options OptionsScreenData[] =
{
		/* ID						Option Name					Option State List			(active,selected,number-of)-states */
	{	OPTION_ID_NETWORK_PROFILES,	"WiFi",						{"Off","1","2","3","4"},		1,1,5		},
	{	OPTION_ID_WIFI_AUTOSTART,	"WiFi AutoStart",			{"No", "Yes"},					1,1,2		},
	
	{	OPTION_ID_USB_ENABLE,		"USB",						{"OFF","ON","AUTOSTART"},					1,1,3		},
	{	OPTION_ID_PLAYMODE,			"Play Mode",				{"Normal", "Single", "Repeat", "Global"},	1,1,4		},
	{	OPTION_ID_CPU_SPEED,		"CPU Speed",				{"111","222","266","333"},		2,2,4		},
	{	OPTION_ID_LOG_LEVEL,		"Log Level",				{"All","Verbose","Info","Errors","Off"},	1,1,5		},
	{	OPTION_ID_SKIN,				"Skin",						{""},							0,0,0		},
	{	OPTION_ID_INITIAL_SCREEN,	"Initial Screen",			{"Files", "Playlist","SHOUT","Options"}, 1,1,4 },
	{	OPTION_ID_REFRESH_PLAYLISTS,"Refresh Playlists",		{""},							0,0,0		},
	{	OPTION_ID_SHOUTCAST_DN,		"Get Latest SHOUTcast DB",	{""},							0,0,0		},
	{	OPTION_ID_PLUGINS_MENU,		"Plugins Menu",				{""},							0,0,0		},
	{	OPTION_ID_SAVE_CONFIG,		"Save Options",				{""},							0,0,0		},
	{	OPTION_ID_EXIT,				"Exit PSPRadio",			{""},							0,0,0		},

	{  -1,  						"",							{""},							0,0,0		}
};

OptionsScreen::OptionsScreen(int Id, CScreenHandler *ScreenHandler):IScreen(Id, ScreenHandler)
{
	if (Id == CScreenHandler::PSPRADIO_SCREEN_OPTIONS)
	{
		m_iNetworkProfile = 1;
		m_WifiAutoStart = false;
		m_USBAutoStart = false;
		m_USBStorage = new CPSPUSBStorage(pPSPApp);
		LoadFromConfig();
	}
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
	Log(LOG_LOWLEVEL, "LoadFromConfig(): pConfig=%p", pConfig);

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
			m_USBStorage->EnableUSB();
		}
		else
		{
			m_USBAutoStart = false;
			Log(LOG_INFO, "LoadFromConfig(): USB_AUTOSTART Not Set");
		}
		/** OPTION_ID_USB_AUTOSTART */

		/** OPTION_ID_PLAYMODE */
		playmodes playmode = (playmodes)pConfig->GetInteger("PLAYBACK:MODE", 0);
		m_ScreenHandler->SetPlayMode(playmode);
		Log(LOG_INFO, "LoadFromConfig(): PlayMode retrieved: %d", playmode);
		/** OPTION_ID_PLAYMODE */

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
		pConfig->SetString("PLUGINS:UI", m_ScreenHandler->GetCurrentUIName());
		pConfig->SetString("PLUGINS:DEFAULT_VISUALIZER", gPSPRadio->GetActivePluginName(PLUGIN_VIS));
		/** OPTION_ID_INITIAL_SCREEN */
		pConfig->SetInteger("SYSTEM:INITIAL_SCREEN", m_ScreenHandler->GetInitialScreen());
		/** OPTION_ID_INITIAL_SCREEN */

		/** OPTION_ID_WIFI_AUTOSTART */
		pConfig->SetInteger("WIFI:AUTOSTART", m_WifiAutoStart);
		/** OPTION_ID_WIFI_AUTOSTART */

		/** OPTION_ID_USB_AUTOSTART */
		pConfig->SetInteger("SYSTEM:USB_AUTOSTART", m_USBAutoStart);
		/** OPTION_ID_USB_AUTOSTART */

		/** OPTION_ID_PLAYMODE */
		pConfig->SetInteger("PLAYBACK:MODE",m_ScreenHandler->GetPlayMode());
		/** OPTION_ID_PLAYMODE */

		/** OPTION_ID_SKIN */
		pConfig->SetString("PLUGINS:UI_SKIN", m_ScreenHandler->GetCurrentSkin());
		/** OPTION_ID_SKIN */

		
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
		if (-1 == OptionsScreenData[iOptNo].Id)
			break;

		Option.Id = OptionsScreenData[iOptNo].Id;
		sprintf(Option.strName, 	OptionsScreenData[iOptNo].strName);
		memcpy(Option.strStates, OptionsScreenData[iOptNo].strStates,
			sizeof(char*)*OptionsScreenData[iOptNo].iNumberOfStates);
		Option.iActiveState		= OptionsScreenData[iOptNo].iActiveState;
		Option.iSelectedState	= OptionsScreenData[iOptNo].iSelectedState;
		Option.iNumberOfStates	= OptionsScreenData[iOptNo].iNumberOfStates;

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
				Option.iSelectedState = Option.iActiveState;
				break;
			}
			
			case OPTION_ID_USB_ENABLE:
				if (m_USBAutoStart==true)
				{
					Option.iActiveState = 3;
				}
				else
				{
					Option.iActiveState = (m_USBStorage->IsUSBEnabled()==true)?2:1;
				}
				Option.iSelectedState = Option.iActiveState;
				break;
			
			case OPTION_ID_PLAYMODE:
				Log(LOG_LOWLEVEL, "Table playmode=%d. New playmode = %d(+1).",
					Option.iActiveState, m_ScreenHandler->GetPlayMode());
				Option.iActiveState = (m_ScreenHandler->GetPlayMode())+1;
				Option.iSelectedState = Option.iActiveState;
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
				Option.iSelectedState = Option.iActiveState;
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
				Option.iSelectedState = Option.iActiveState;
				break;

			case OPTION_ID_SKIN:
				RetrieveSkins(/*option*/Option, /*currentUI*/m_ScreenHandler->GetCurrentUIName(), m_ScreenHandler->GetCurrentSkin());
				Option.iSelectedState = Option.iActiveState;
				break;
						
			case OPTION_ID_INITIAL_SCREEN:
				Option.iActiveState = m_ScreenHandler->GetInitialScreen() + 1;
				Option.iSelectedState = Option.iActiveState;
				break;

			case OPTION_ID_WIFI_AUTOSTART:
				Option.iActiveState = (m_WifiAutoStart==true)?2:1;
				Option.iSelectedState = Option.iActiveState;
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
	char *strSelection  = (*m_CurrentOptionIterator).strStates[iSelectionBase0];

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
			switch (iSelectionBase1)
			{
				case 1: /* OFF */
					int iRet = m_USBStorage->DisableUSB();
					if (-1 == iRet)
					{
						ReportError("USB Busy, try again later.");
					}
					if (false == m_USBStorage->IsUSBEnabled())
					{
						fOptionActivated = true;
					}
					m_USBAutoStart = false;
					break;
				case 2: /* ON  */
					m_USBStorage->EnableUSB();
					if (true == m_USBStorage->IsUSBEnabled())
					{
						fOptionActivated = true;
					}
					m_USBAutoStart = false;
					break;
				case 3: /* AUTOSTART */
					m_USBStorage->EnableUSB();
					if (true == m_USBStorage->IsUSBEnabled())
					{
						fOptionActivated = true;
					}
					m_USBAutoStart = true;
					break;
			}
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

		case OPTION_ID_SKIN:
			/* Have UI Stop updating */
			m_UI->OnScreenshot(CScreenHandler::PSPRADIO_SCREENSHOT_ACTIVE);
			m_ScreenHandler->StartUI(m_ScreenHandler->GetCurrentUIName(), strSelection, true);
			pPSPApp->SendEvent(MID_GIVEUPEXCLISIVEACCESS, NULL, SID_PSPRADIO);

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
			((LocalFilesScreen*)m_ScreenHandler->GetScreen(CScreenHandler::PSPRADIO_SCREEN_LOCALFILES))->LoadLists();
			m_UI->DisplayMessage("Done");
			fOptionActivated = true;
			break;

		case OPTION_ID_SHOUTCAST_DN:
			if ( (timeNow - timeLastTime) > 60 ) /** Only allow to refresh shoutcast once a minute max! */
			{
				m_UI->DisplayMessage("Downloading latest SHOUTcast Database. . .");
				m_ScreenHandler->GetSound()->Stop(); /** Stop stream if playing */
				if (true == m_ScreenHandler->DownloadNewSHOUTcastDB())
				{
					timeLastTime = timeNow; /** Only when successful */
					int iItemNo = ((SHOUTcastScreen*)m_ScreenHandler->GetScreen(CScreenHandler::PSPRADIO_SCREEN_SHOUTCAST_BROWSER))->LoadLists();
					char strMsg[45];
					sprintf(strMsg, "* %d Radio Stations Retrieved *", iItemNo);
					m_UI->DisplayMessage(strMsg);
				}
			}
			else
			{
				m_UI->DisplayErrorMessage("Wait a minute before re-downloading, thanks.");
			}
			fOptionActivated = false;
			break;

		case OPTION_ID_PLUGINS_MENU:
			//m_UI->DisplayMessage("Saving Configuration Options");
			//Log(LOG_INFO, "User selected to save config file.");
			// Enter option menu and store the current screen
			m_ScreenHandler->SetCurrentScreen(m_ScreenHandler->GetScreen(CScreenHandler::PSPRADIO_SCREEN_OPTIONS_PLUGIN_MENU));
			m_ScreenHandler->GetCurrentScreen()->Activate(m_UI);
			fOptionActivated = true;
			return;
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
	Log(LOG_LOWLEVEL, "Start_Network(%d)", iProfile);
	
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
			
			CIniParser *pConfig = m_ScreenHandler->GetConfig();
			if (pConfig->GetInteger("DEBUGGING:WIFI_LOG_ENABLE", 0))
				{
				char	*server, *port;
				Log(LOG_INFO, "Enabling WiFi logging.");
				server = pConfig->GetString("DEBUGGING:WIFI_LOG_SERVER", "192.168.2.2");
				port = pConfig->GetString("DEBUGGING:WIFI_LOG_PORT", "8000");
				pLogging->EnableWiFiLogging(server, port);
				}
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
	CIniParser *pConfig = m_ScreenHandler->GetConfig();

	if (pPSPApp->IsNetworkEnabled())
	{
		m_UI->DisplayMessage_DisablingNetwork();

		if (pConfig->GetInteger("DEBUGGING:WIFI_LOG_ENABLE", 0))
		{
			Log(LOG_INFO, "Disabling WiFi logging.");
			pLogging->DisableWiFiLogging();
		}

		Log(LOG_INFO, "Disabling network...");
		pPSPApp->DisableNetwork();
		sceKernelDelayThread(500000);
	}
	return 0;
}

void OptionsScreen::InputHandler(int iButtonMask)
{
	if (IS_BUTTON_PRESSED(iButtonMask, PSPRadioButtonMap.BTN_OPT_NAMES_BACK))
	{
		if(m_CurrentOptionIterator == m_OptionsList.begin())
			m_CurrentOptionIterator = m_OptionsList.end();
		m_CurrentOptionIterator--;
		m_UI->UpdateOptionsScreen(m_OptionsList, m_CurrentOptionIterator);
	}
	else if (IS_BUTTON_PRESSED(iButtonMask, PSPRadioButtonMap.BTN_OPT_NAMES_FWD))
	{
		m_CurrentOptionIterator++;
		if(m_CurrentOptionIterator == m_OptionsList.end())
			m_CurrentOptionIterator = m_OptionsList.begin();

		m_UI->UpdateOptionsScreen(m_OptionsList, m_CurrentOptionIterator);
	}
	else if (IS_BUTTON_PRESSED(iButtonMask, PSPRadioButtonMap.BTN_OPT_OPTIONS_BACK))
	{
		if ((*m_CurrentOptionIterator).iSelectedState > 1)
		{
			(*m_CurrentOptionIterator).iSelectedState--;

			//OnOptionChange();
			m_UI->UpdateOptionsScreen(m_OptionsList, m_CurrentOptionIterator);
		}
	}
	else if (IS_BUTTON_PRESSED(iButtonMask, PSPRadioButtonMap.BTN_OPT_OPTIONS_FWD))
	{
		if ((*m_CurrentOptionIterator).iSelectedState < (*m_CurrentOptionIterator).iNumberOfStates)
		{
			(*m_CurrentOptionIterator).iSelectedState++;

			//OnOptionChange();
			m_UI->UpdateOptionsScreen(m_OptionsList, m_CurrentOptionIterator);
		}
	}
	else if (IS_BUTTON_PRESSED(iButtonMask, PSPRadioButtonMap.BTN_OPT_ACTIVATE))
	{
		OnOptionActivation();
	}
}

int OptionsScreen::RetrieveSkins(Options &Option, const char *strCurrentUI, const char *strActiveSkin)
{
	int dfd = 0;
	char *strPath = m_ScreenHandler->GetCWD();
	int iNumberOfSkinsFound = 0;
	char *strSkin = NULL;
	SceIoDirent direntry;
	char strPrefix[MAXPATHLEN];
	
	strlcpy(strPrefix, strCurrentUI, MAXPATHLEN);
	strPrefix[MAXPATHLEN-1] = 0;
	if (strrchr(strPrefix, '.'))
	{
		*strrchr(strPrefix, '.') = 0;
	}
	strcat(strPrefix, ".");

	Option.strStates[0] = strdup(DEFAULT_SKIN);
	iNumberOfSkinsFound++;

	Log(LOG_LOWLEVEL, "RetrieveSkins: Reading '%s' Directory, looking for '%s' directories (strCurrentUI='%s', GetCurrentUIName='%s')...", strPath, strPrefix, strCurrentUI, m_ScreenHandler->GetCurrentUIName());
	
	dfd = sceIoDopen(strPath);

	Option.iActiveState = 1; /* Initial value */

	/** Get all files */
	if (dfd >= 0)
	{
		/** RC 10-10-2005: The direntry has to be memset! Or else the app will/may crash! */
		memset(&direntry, 0, sizeof(SceIoDirent));
		while(sceIoDread(dfd, &direntry) > 0)
		{
			if(direntry.d_stat.st_attr & FIO_SO_IFDIR) /** It's a directory */
			{
				if (strcmp(direntry.d_name, ".") == 0)
					continue;
				else if (strcmp(direntry.d_name, "..") == 0)
					continue;

				Log(LOG_LOWLEVEL, "RetrieveSkins: Processing '%s'", direntry.d_name);
				if (strncmp(direntry.d_name, strPrefix, strlen(strPrefix)) == 0)
				{
					Log(LOG_LOWLEVEL, "RetrieveSkins(): Adding '%s' to list. Found %d elements",
						direntry.d_name, iNumberOfSkinsFound+1);

					strSkin = (char*)malloc(128);
					if (strSkin)
					{
						char strFormat[64];
						sprintf(strFormat, "%s%%s", strPrefix);

						sscanf(direntry.d_name, strFormat, strSkin);

						Log(LOG_LOWLEVEL, "RetrieveSkins: direntry='%s' strSkin='%s'", direntry.d_name, strSkin, strActiveSkin);

						Option.strStates[iNumberOfSkinsFound] = strSkin;

						if (strcmp(strSkin, strActiveSkin) == 0)
						{
							Option.iActiveState = iNumberOfSkinsFound + 1;
						}

						iNumberOfSkinsFound++;
					}
					else
					{
						Log(LOG_ERROR, "RetrieveSkins: Memory error!");
						break;
					}
				}
			}
		}
		Option.iNumberOfStates = iNumberOfSkinsFound;
		sceIoDclose(dfd);
	}
	else
	{
		Log(LOG_ERROR, "RetrieveSkins: Unable to open '%s' Directory! (Error=0x%x)", strPath, dfd);
	}

	return iNumberOfSkinsFound;
}
