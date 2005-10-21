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
	OPTION_ID_CPU_SPEED,
	OPTION_ID_LOG_LEVEL,
	OPTION_ID_UI,
};

CScreenHandler::Options OptionsData[] = 
{
		/* ID						Option Name					Option State List			(active,selected,number-of)-states */
	{	OPTION_ID_NETWORK_PROFILES,	"Select Network Profile",	{"1","2","3","4","5"},			1,1,5		},
	{	OPTION_ID_NETWORK_ENABLE,	"Start Network",			{"OFF","ON"},					1,1,2		},
	{	OPTION_ID_USB_ENABLE,		"USB",						{"OFF","ON"},					1,1,2		},
	{	OPTION_ID_CPU_SPEED,		"CPU Speed",				{"222","266","333"},			1,1,3		},
	{	OPTION_ID_LOG_LEVEL,		"Log Level",				{"10","20","50","80","100"},	1,1,5		},
	{	OPTION_ID_UI,				"User Interface",			{"Text","Graphic","3D"},		1,1,3		},
	
	{  -1,  						"",							{""},							0,0,0		}
};

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
