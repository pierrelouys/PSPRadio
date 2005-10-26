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

#define SHOUTCAST_DB_REQUEST_STRING				"http://www.shoutcast.com/sbin/xmllister.phtml?service=pspradio&no_compress=1"
#define SHOUTCAST_DB_COMPRESSED_REQUEST_STRING 	"http://www.shoutcast.com/sbin/xmllister.phtml?service=pspradio"
#define SHOUTCAST_DB_COMPRESSED_FILENAME		"SHOUTcast/db.xml.gz"
#define SHOUTCAST_DB_FILENAME					"SHOUTcast/db.xml"

bool UnCompress(char *strSourceFile, char *strDestFile);


bool CScreenHandler::DownloadSHOUTcastDB()
{
	bool success = false;
	CPSPSoundStream *WebConnection = new CPSPSoundStream();
	WebConnection->SetURI(SHOUTCAST_DB_COMPRESSED_REQUEST_STRING);
	WebConnection->Open();
	if (true == WebConnection->IsOpen())
	{
		Log(LOG_INFO, "DownloadSHOUTcastDB(): Connected - Downloading '%s'", SHOUTCAST_DB_COMPRESSED_FILENAME);
		bool bRet;
		size_t bytes;
		bRet = WebConnection->DownloadToFile(SHOUTCAST_DB_COMPRESSED_FILENAME, bytes);
		
		if (true == bRet)
		{
			WebConnection->Close();
			delete WebConnection, WebConnection = NULL;
			Log(LOG_INFO, "DownloadSHOUTcastDB(): DB Retrieved. (%dbytes)", bytes);
			m_UI->DisplayMessage("Uncompressing . . .");
			bRet = UnCompress(SHOUTCAST_DB_COMPRESSED_FILENAME, SHOUTCAST_DB_FILENAME);
			if (true == bRet)
			{
				m_UI->DisplayMessage("SHOUTcast DataBase Retrieved");
				Log(LOG_INFO, "SHOUTcast.com DB retrieved.");
				success = true;
			}
			else
			{
				Log(LOG_ERROR, "Error uncompressing '%s' to '%s'",SHOUTCAST_DB_COMPRESSED_FILENAME, SHOUTCAST_DB_FILENAME);
				m_UI->DisplayMessage("Error Uncompressing . . .");
			}
		}
	}
	else
	{
		Log(LOG_ERROR, "Error connecting to '%s'", SHOUTCAST_DB_COMPRESSED_REQUEST_STRING);
		m_UI->DisplayErrorMessage("Couldn't connect to SHOUTcast.com ...");
	}
	
	return success;
}

bool UnCompress(char *strSourceFile, char *strDestFile)
{
	bool success = false;
	
	return success;
}

