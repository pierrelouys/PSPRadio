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

#define SHOUTCAST_DB_REQUEST_STRING 	"http://www.shoutcast.com/sbin/xmllister.phtml?service=pspradio&no_compress=1"

void CScreenHandler::DownloadSHOUTcastDB()
{
	CPSPSoundStream *connection = new CPSPSoundStream();
	connection->SetURI(SHOUTCAST_DB_REQUEST_STRING);
	connection->Open();

	if (true == connection->IsOpen())
	{
		Log(LOG_LOWLEVEL, "DownloadSHOUTcastDB(): Connection open, downloading...");
		FILE *fOut = fopen("SHOUTcast/db.xml", "w");
		if (fOut)
		{
			int iRet = 0;
			int iByteCnt = 0;
			char *buffer = (char*)malloc(8192);
			memset(buffer, 0, 8192);
			for(;;)
			{
				iRet = recv(connection->GetSocketDescriptor(), buffer, 8192, 0);
				if (0 == iRet)
				{
					connection->Close();
					delete connection, connection = NULL;
					fclose(fOut), fOut = NULL;
					Log(LOG_LOWLEVEL, "DownloadSHOUTcastDB(): DB Retrieved. (%dbytes)", iByteCnt);
					m_UI->DisplayMessage("DB Retrieved");
					break;
				}
				if (iRet > 0)
				{
					iByteCnt+= iRet;
					fwrite(buffer, iRet, 1, fOut);
				}
			}
			free(buffer), buffer = NULL;
		}
		else
		{
			Log(LOG_ERROR, "DownloadSHOUTcastDB(): Error- Couldn't open file for write.");
			m_UI->DisplayErrorMessage("Couldn't write file");
		}
	}
	else
	{
		Log(LOG_ERROR, "DownloadSHOUTcastDB(): Error- Couldn't connect to SHOUTcast.");
		m_UI->DisplayErrorMessage("Couldn't connect...");
	}
}
