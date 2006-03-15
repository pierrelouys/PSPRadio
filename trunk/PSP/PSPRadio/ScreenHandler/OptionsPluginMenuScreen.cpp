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
#include "PRXLoader.h"
#include "IPSPRadio_UI.h"
#include "SHOUTcastScreen.h"
#include "LocalFilesScreen.h"
#include "OptionsPluginMenuScreen.h"
#include <FSS_Exports.h>

#define ReportError pPSPApp->ReportError

enum OptionIDs
{
	OPTION_ID_UI,
	OPTION_ID_FSS,
};

OptionsPluginMenuScreen::Options OptionsPluginMenuData[] =
{
		/* ID						Option Name					Option State List			(active,selected,number-of)-states */
	{	OPTION_ID_UI,				"User Interface",			{"Text", "3D"},					1,1,2		},
	{	OPTION_ID_FSS,				"FileSystemServers",		{"Off", "FTPD"},				1,1,2		},

	{  -1,  						"",							{""},							0,0,0		}
};

OptionsPluginMenuScreen::OptionsPluginMenuScreen(int Id, CScreenHandler *ScreenHandler):OptionsScreen(Id, ScreenHandler)
{
	Log(LOG_LOWLEVEL, "OptionsPluginMenuScreen(): Id=%d", Id);
	m_FSSModuleLoader = NULL;
	m_FSSModuleLoader = new CPRXLoader();
	if (m_FSSModuleLoader == NULL)
	{
		Log(LOG_ERROR, "Memory error - Unable to create CPRXLoader.");
	}
	//LoadFromConfig();
}

/** Activate() is called on screen activation */
void OptionsPluginMenuScreen::Activate(IPSPRadio_UI *UI)
{
	IScreen::Activate(UI);

	// Update.  This is necesary the first time */
	UpdateOptionsData();
	m_UI->UpdateOptionsScreen(m_OptionsList, m_CurrentOptionIterator);
}

void OptionsPluginMenuScreen::LoadFromConfig()
{
	Log(LOG_INFO, "LoadFromConfig");
}

void OptionsPluginMenuScreen::SaveToConfigFile()
{
}

/** This populates and updates the option data */
void OptionsPluginMenuScreen::UpdateOptionsData()
{
	Options Option;

	list<Options>::iterator		OptionIterator;
	
	while(m_OptionsList.size())
	{
		// Release memory allocated for network profile names
		OptionIterator = m_OptionsList.begin();
// 		if ((*OptionIterator).Id == OPTION_ID_NETWORK_PROFILES)
// 		{
// 			for (int i = 0; i < (*OptionIterator).iNumberOfStates; i++)
// 			{
// 				free((*OptionIterator).strStates[i]);
// 			}
// 		}
		m_OptionsList.pop_front();
	}


	for (int iOptNo=0;; iOptNo++)
	{
		if (-1 == OptionsPluginMenuData[iOptNo].Id)
			break;

		/* Make a copy of the table entry */
		Option.Id = OptionsPluginMenuData[iOptNo].Id;
		sprintf(Option.strName, 	OptionsPluginMenuData[iOptNo].strName);
		memcpy(Option.strStates, OptionsPluginMenuData[iOptNo].strStates, 
				sizeof(char*)*OptionsPluginMenuData[iOptNo].iNumberOfStates);
		Option.iActiveState		= OptionsPluginMenuData[iOptNo].iActiveState;
		Option.iSelectedState	= OptionsPluginMenuData[iOptNo].iSelectedState;
		Option.iNumberOfStates	= OptionsPluginMenuData[iOptNo].iNumberOfStates;

		/** Modify from data table */
		switch(iOptNo)
		{
			case OPTION_ID_UI:
				Option.iActiveState = m_ScreenHandler->GetCurrentUI() + 1;
				Option.iSelectedState = Option.iActiveState;
				break;
			case OPTION_ID_FSS:
				break;
		}

		m_OptionsList.push_back(Option);
	}

	m_CurrentOptionIterator = m_OptionsList.begin();

}

void OptionsPluginMenuScreen::OnOptionActivation()
{
	bool fOptionActivated = false;
//	static time_t	timeLastTime = 0;
//	time_t timeNow = clock() / (1000*1000); /** clock is in microseconds */
	int iSelectionBase0 = (*m_CurrentOptionIterator).iSelectedState - 1;
//	int iSelectionBase1 = (*m_CurrentOptionIterator).iSelectedState; 

	switch ((*m_CurrentOptionIterator).Id)
	{
		case OPTION_ID_UI:
			m_ScreenHandler->StartUI((CScreenHandler::UIs)iSelectionBase0);
			fOptionActivated = true;
			break;
		case OPTION_ID_FSS:
			Log(LOG_INFO, "User selected an FSS Plugin.");
			if (iSelectionBase0 > 0)
			{
				m_UI->DisplayMessage("Starting Plugin . . .");
				//sprintf(m_UI->buff, "original data");
				//Log(LOG_INFO, "ui->buff before loading plugin='%s'", m_UI->buff);
				int res = LoadFSSPlugin(iSelectionBase0 - 1);
				//Log(LOG_INFO, "ui->buff after loading plugin='%s'", m_UI->buff);
				if (res == 0)
				{
					fOptionActivated = true;
					m_UI->DisplayMessage("Plugin Started");
				}
				else
				{
					m_UI->DisplayMessage("Error Starting Plugin . . .");
				}
			}
			break;

	}

	if (true == fOptionActivated)
	{
		(*m_CurrentOptionIterator).iActiveState = (*m_CurrentOptionIterator).iSelectedState;
		m_UI->UpdateOptionsScreen(m_OptionsList, m_CurrentOptionIterator);
	}
}

int OptionsPluginMenuScreen::LoadFSSPlugin(int iFSSIndex)
{
	char strModulePath[MAXPATHLEN+1];
	char cwd[MAXPATHLEN+1];
	
	if (m_FSSModuleLoader->IsLoaded() == true)
	{
		Log(LOG_INFO, "Unloading currently running plugin");
		m_FSSModuleLoader->Unload();
	}

	sprintf(strModulePath, "%s/%s", getcwd(cwd, MAXPATHLEN), "FSS_FTPD.prx");

	int id = m_FSSModuleLoader->Load(strModulePath);
	
	if (m_FSSModuleLoader->IsLoaded() == true)
	{
		SceKernelModuleInfo modinfo;
		memset(&modinfo, 0, sizeof(modinfo));
		modinfo.size = sizeof(modinfo);
		sceKernelQueryModuleInfo(id, &modinfo);
		Log(LOG_INFO, "'%s' Loaded at text_addr=0x%x",
			strModulePath, modinfo.text_addr);
	
		int iRet = m_FSSModuleLoader->Start();
		
		Log(LOG_INFO, "Module start returned: 0x%x", iRet);
		
		ModuleStartFSS();
		
		return 0;
		
	}
	else
	{
		Log(LOG_ERROR, "Error loading '%s' Module. Error=0x%x", strModulePath, m_FSSModuleLoader->GetError());
		return -1;
	}
}
