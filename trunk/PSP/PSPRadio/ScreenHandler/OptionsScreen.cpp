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
#include "DirList.h"
#include "PlayList.h"
#include "TextUI.h"
#include "GraphicsUI.h"
#include "SandbergUI.h" 


#define ReportError pPSPApp->ReportError

enum OptionIDs
{
	OPTION_ID_NETWORK_PROFILES,
	OPTION_ID_USB_ENABLE,
	OPTION_ID_CPU_SPEED,
	OPTION_ID_LOG_LEVEL,
	OPTION_ID_UI,
	OPTION_ID_SHOUTCAST_DN,
};

OptionsScreen::Options OptionsData[] = 
{
		/* ID						Option Name					Option State List			(active,selected,number-of)-states */
	{	OPTION_ID_NETWORK_PROFILES,	"WiFi",	{"Off","1","2","3","4"},			1,1,5		},
	{	OPTION_ID_USB_ENABLE,		"USB",						{"OFF","ON"},					1,1,2		},
	{	OPTION_ID_CPU_SPEED,		"CPU Speed",				{"111","222","266","333"},		2,2,4		},
	{	OPTION_ID_LOG_LEVEL,		"Log Level",				{"All","Verbose","Info","Errors","Off"},	1,1,5		},
	{	OPTION_ID_UI,				"User Interface",			{"Text","Graphics","3D"},		1,1,3		},
	{	OPTION_ID_SHOUTCAST_DN,		"Get Latest SHOUTcast DB",	{"Download"},					0,1,1		},
	
	{  -1,  						"",							{""},							0,0,0		}
};

void OptionsScreen::Activate(IPSPRadio_UI *UI)
{
	IScreen::Activate(UI);

	// Update network and UI options.  This is necesary the first time */
	UpdateOptionsData();
	m_UI->UpdateOptionsScreen(m_OptionsList, m_CurrentOptionIterator);
}

OptionsScreen::OptionsScreen(int Id, CScreenHandler *ScreenHandler):IScreen(Id, ScreenHandler)
{
	m_iNetworkProfile = 1;
//	m_NetworkStarted  = false;
}


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
				for (int i = 0; i < Option.iNumberOfStates; i++)
				{
					NetworkName = (char*)malloc(128);
					if (0 == i)
					{
						sprintf(NetworkName, "Off");
					}
					else
					{
						GetNetworkProfileName(i, NetworkName, 128);
					}
					Option.strStates[i] = NetworkName;
				}
				Option.iActiveState = (pPSPApp->IsNetworkEnabled()==true)?(m_iNetworkProfile+1):1;
				break;
			}
				
			case OPTION_ID_USB_ENABLE:
				Option.iActiveState = (pPSPApp->IsUSBEnabled()==true)?2:1;
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
				switch(Logging.GetLevel())
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
		}
		
		m_OptionsList.push_back(Option);
	}
	
	m_CurrentOptionIterator = m_OptionsList.begin();

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

//	OPTION_ID_NETWORK_PROFILES,
//	OPTION_ID_NETWORK_ENABLE,
//	OPTION_ID_USB_ENABLE,
//	OPTION_ID_CPU_SPEED
void OptionsScreen::OnOptionActivation()
{
	bool fOptionActivated = false;
	static time_t	timeLastTime = 0;
	time_t timeNow = clock() / (1000*1000); /** clock is in microseconds */
	
	switch ((*m_CurrentOptionIterator).Id)
	{
		case OPTION_ID_NETWORK_PROFILES:
			m_iNetworkProfile = (*m_CurrentOptionIterator).iSelectedState - 1;
			if (m_iNetworkProfile > 0) /** Enable */
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
		case OPTION_ID_CPU_SPEED:
		{
			int iRet = -1;
			switch ((*m_CurrentOptionIterator).iSelectedState)
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
			switch((*m_CurrentOptionIterator).iSelectedState)
			{
				case 1:
					Logging.SetLevel(LOG_VERYLOW);
					break;
				case 2:
					Logging.SetLevel(LOG_LOWLEVEL);
					break;
				case 3:
					Logging.SetLevel(LOG_INFO);
					break;
				case 4:
					Logging.SetLevel(LOG_ERROR);
					break;
				case 5:
					Logging.SetLevel(LOG_ALWAYS);
					break;
			}
			Log(LOG_ALWAYS, "Log Level Changed to (%d)", Logging.GetLevel());
			fOptionActivated = true;
			break;
		case OPTION_ID_UI:
			m_ScreenHandler->StartUI((CScreenHandler::UIs)((*m_CurrentOptionIterator).iSelectedState - 1));
			fOptionActivated = true;
			break;
		case OPTION_ID_SHOUTCAST_DN:
			if ( (timeNow - timeLastTime) > 60 ) /** Only allow to refresh shoutcast once a minute max! */
			{
				m_UI->DisplayMessage("Downloading latest SHOUTcast Database. . .");
				if (true == m_ScreenHandler->DownloadSHOUTcastDB())
				{
					timeLastTime = timeNow; /** Only when successful */
				}
			}
			else
			{
				m_UI->DisplayErrorMessage("Wait a minute before re-downloading, thanks.");
			}		
			fOptionActivated = false;
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
		
		//m_NetworkStarted = true;
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

void OptionsScreen::GetNetworkProfileName(int iProfile, char *buf, size_t size)
{
	netData data;
	memset(&data, 0, sizeof(netData));
	data.asUint = 0xBADF00D;
	memset(&data.asString[4], 0, 124);
	sceUtilityGetNetParam(iProfile, 0/**Profile Name*/, &data);
	
	strncpy(buf, data.asString, size);
}
