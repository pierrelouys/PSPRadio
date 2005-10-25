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
#include <psppower.h>


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
	OPTION_ID_CPU_SPEED,
	OPTION_ID_LOG_LEVEL,
	OPTION_ID_UI,
	OPTION_ID_SHOUTCAST_DN,
};

CScreenHandler::Options OptionsData[] = 
{
		/* ID						Option Name					Option State List			(active,selected,number-of)-states */
	{	OPTION_ID_NETWORK_PROFILES,	"Select Network Profile",	{"1","2","3","4","5"},			1,1,5		},
	{	OPTION_ID_NETWORK_ENABLE,	"Start Network",			{"OFF","ON"},					1,1,2		},
	{	OPTION_ID_USB_ENABLE,		"USB",						{"OFF","ON"},					1,1,2		},
	{	OPTION_ID_CPU_SPEED,		"CPU Speed",				{"111","222","266","333"},		2,2,4		},
	{	OPTION_ID_LOG_LEVEL,		"Log Level",				{"All","Verbose","Info","Errors","Off"},	1,1,5		},
	{	OPTION_ID_UI,				"User Interface",			{"Text","Graphics","3D"},		1,1,3		},
	{	OPTION_ID_SHOUTCAST_DN,		"Get Latest SHOUTcast DB",	{"Download"},					0,1,1		},
	
	{  -1,  						"",							{""},							0,0,0		}
};

void CScreenHandler::PopulateOptionsData()
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
				Option.iNumberOfStates = pPSPApp->GetNumberOfNetworkProfiles() - 1;
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
				switch(scePowerGetCpuClockFrequency())
				{
					case 111:
						Option.iActiveState = 1;
						break;
					case 222:
						Option.iActiveState = 2;
						break;
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
				Option.iActiveState = m_CurrentUI + 1;
				break;
		}
		
		m_OptionsList.push_back(Option);
	}
	
	m_CurrentOptionIterator = m_OptionsList.begin();

}

void CScreenHandler::OptionsScreenInputHandler(int iButtonMask)
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
void CScreenHandler::OnOptionActivation()
{
	bool fOptionActivated = false;
	static time_t	timeLastTime = 0;
	time_t timeNow = clock() / (1000*1000); /** clock is in microseconds */
	
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
			StartUI((UIs)((*m_CurrentOptionIterator).iSelectedState - 1));
			fOptionActivated = true;
			break;
		case OPTION_ID_SHOUTCAST_DN:
			if ( (timeNow - timeLastTime) > 60 ) /** Only allow to refresh shoutcast once a minute max! */
			{
				m_UI->DisplayMessage("downloading latest...");
				DownloadSHOUTcastDB();
				timeLastTime = timeNow;
			}
			else
			{
				m_UI->DisplayErrorMessage("Only once a minute!");
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
