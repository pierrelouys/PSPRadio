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
#include "LocalFilesScreen.h"
#include "TextUI.h"
#include "GraphicsUI.h"
#include "TextUI3D.h" 


LocalFilesScreen::LocalFilesScreen(int Id, CScreenHandler *ScreenHandler)
	:PlayListScreen(Id, ScreenHandler)
{
	Log(LOG_VERYLOW,"LocalFilesScreen Ctor.");
	m_Lists = NULL;

	//m_Lists = new CMetaDataContainer();

	m_strPath = strdup("ms0:/PSP/MUSIC");
	
	//LoadLists();

}


LocalFilesScreen::~LocalFilesScreen()
{
	if (m_strPath)
	{
		free(m_strPath), m_strPath = NULL;
	}

}

void LocalFilesScreen::LoadLists()
{
	if (!m_Lists)
	{
		m_Lists = new CMetaDataContainer();
	}

	if (m_Lists)
	{
		Log(LOG_LOWLEVEL, "LoadLists::PSPRADIO_SCREEN_LOCALFILES");
		m_Lists->Clear();
		
		//Log(LOG_LOWLEVEL, "Loading xml file '%s'.", SHOUTCAST_DB_FILENAME);
		//m_Lists->LoadSHOUTcastXML(SHOUTCAST_DB_FILENAME);
		Log(LOG_LOWLEVEL, "Loading local files from '%s'", m_strPath);
		m_Lists->LoadDirectory(m_strPath);

		m_Lists->SetCurrentSide(CMetaDataContainer::CONTAINER_SIDE_CONTAINERS);

	}
}
