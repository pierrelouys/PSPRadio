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
#include <list>
#include <PSPApp.h>
#include <PSPSound_MP3.h>
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <malloc.h>
#include <iniparser.h>
#include <Tools.h>
#include <stdarg.h>
#include <Logging.h>
#include "TextUI.h"


#define MAX_ROWS 34
#define MAX_COL  68

#define TEXT_UI_CFG_FILENAME "TextUI.cfg"

#define RGB2BGR(x) (((x>>16)&0xFF) | (x&0xFF00) | ((x<<16)&0xFF0000))

CTextUI::CTextUI()
{
	m_lockprint = NULL;
	m_lockclear = NULL;
	
	m_lockprint = new CLock("Print_Lock");
	m_lockclear = new CLock("Clear_Lock");

}

CTextUI::~CTextUI()
{
	delete(m_lockprint);
	m_lockprint = NULL;
	delete(m_lockclear);
	m_lockclear = NULL;
}

int CTextUI::Initialize(char *strCWD)
{
	char strCfgFile[256];

	sprintf(strCfgFile, "%s/%s", strCWD, TEXT_UI_CFG_FILENAME);

	m_Config = new CIniParser(strCfgFile);
	
	pspDebugScreenInit();
	pspDebugScreenSetBackColor(GetConfigColor("COLORS:BACKGROUND"));
	pspDebugScreenSetTextColor(GetConfigColor("COLORS:MAINTEXT"));
	pspDebugScreenClear(); 
	
	return 0;
}

int CTextUI::GetConfigColor(char *strKey)
{
	int iRet;
	sscanf(m_Config->GetStr(strKey), "%x", &iRet);
	
	return RGB2BGR(iRet);
}

void CTextUI::GetConfigPos(char *strKey, int *x, int *y)
{
	sscanf(m_Config->GetStr(strKey), "%d,%d", x, y);
}

void CTextUI::Terminate()
{
}


void CTextUI::uiPrintf(int x, int y, int color, char *strFormat, ...)
{
	va_list args;
	char msg[70*5/** 5 lines worth of text...*/];
	
	m_lockprint->Lock();

	va_start (args, strFormat);         /* Initialize the argument list. */
	
	vsprintf(msg, strFormat, args);

	if (msg[strlen(msg)-1] == 0x0A)
		msg[strlen(msg)-1] = 0; /** Remove LF 0D*/
	if (msg[strlen(msg)-1] == 0x0D) 
		msg[strlen(msg)-1] = 0; /** Remove CR 0A*/

	if (x == -1) /** CENTER */
	{
		x = 67/2 - strlen(msg)/2;
	}
	pspDebugScreenSetXY(x,y);
	pspDebugScreenSetTextColor(color);
	printf(msg);
	
	va_end (args);                  /* Clean up. */

	m_lockprint->Unlock();
}

void CTextUI::ClearRows(int iRowStart, int iRowEnd)
{
	if (iRowEnd == -1)
		iRowEnd = iRowStart;
		
	m_lockclear->Lock();
	for (int iRow = iRowStart ; (iRow < MAX_ROWS) && (iRow <= iRowEnd); iRow++)
	{
		pspDebugScreenSetXY(0,iRow);
		printf("% 67c", ' ');
	}
	m_lockclear->Unlock();
}

int CTextUI::SetTitle(char *strTitle)
{
	int x,y;
	int c;
	GetConfigPos("TEXT_POS:TITLE", &x, &y);
	c = GetConfigColor("COLORS:TITLE");
	uiPrintf(x,y, c, strTitle);
	
	return 0;
}

int CTextUI::DisplayMessage_EnablingNetwork()
{
	int x,y,c;
	GetConfigPos("TEXT_POS:NETWORK_ENABLING", &x, &y);
	c = GetConfigColor("COLORS:NETWORK_ENABLING");
	
	ClearErrorMessage();
	ClearRows(y);
	uiPrintf(x, y, c, "Enabling Network");
	
	return 0;
}

int CTextUI::DisplayMessage_NetworkSelection(int iProfileID, char *strProfileName)
{
	int x,y,c;
	GetConfigPos("TEXT_POS:NETWORK_SELECTION", &x, &y);
	c = GetConfigColor("COLORS:NETWORK_SELECTION");
	
	ClearErrorMessage();
	ClearRows(y);
	uiPrintf(x, y, c, "Press TRIANGLE for Network Profile: %d '%s'", iProfileID, strProfileName);

	return 0;
}

int CTextUI::DisplayMessage_DisablingNetwork()
{
	int x,y,c;
	GetConfigPos("TEXT_POS:NETWORK_DISABLING", &x, &y);
	c = GetConfigColor("COLORS:NETWORK_DISABLING");
	
	ClearRows(y);
	uiPrintf(x, y, c, "Disabling Network");
	
	return 0;
}

int CTextUI::DisplayMessage_NetworkReady(char *strIP)
{
	int x,y,c;
	GetConfigPos("TEXT_POS:NETWORK_READY", &x, &y);
	c = GetConfigColor("COLORS:NETWORK_READY");
	
	ClearRows(y);
	uiPrintf(x, y, c, "Ready, IP %s", strIP);
	
	return 0;
}

int CTextUI::DisplayMainCommands()
{
	int x,y,c;
	GetConfigPos("TEXT_POS:MAIN_COMMANDS", &x, &y);
	c = GetConfigColor("COLORS:MAIN_COMMANDS");
	
	ClearRows(y);
	uiPrintf(x, y, c, "X Play/Pause | [] Stop | L / R To Browse");
		
	return 0;
}

int CTextUI::DisplayActiveCommand(CPSPSound::pspsound_state playingstate)
{
	int x,y,c;
	GetConfigPos("TEXT_POS:ACTIVE_COMMAND", &x, &y);
	c = GetConfigColor("COLORS:ACTIVE_COMMAND");
	
	ClearRows(y);
	switch(playingstate)
	{
	case CPSPSound::STOP:
		uiPrintf(x, y, c, "STOP");
		break;
	case CPSPSound::PLAY:
		uiPrintf(x, y, c, "PLAY");
		break;
	case CPSPSound::PAUSE:
		uiPrintf(x, y, c, "PAUSE");
		break;
	}
	
	return 0;
}

int CTextUI::DisplayErrorMessage(char *strMsg)
{
	int x,y,c;
	GetConfigPos("TEXT_POS:ERROR_MESSAGE", &x, &y);
	c = GetConfigColor("COLORS:ERROR_MESSAGE");
	
	ClearErrorMessage();
	uiPrintf(x, y, c, "Error: %s", strMsg);
	
	return 0;
}

int CTextUI::ClearErrorMessage()
{
	int x,y;
	GetConfigPos("TEXT_POS:ERROR_MESSAGE_ROW_RANGE", &x, &y);
	ClearRows(x, y);
	return 0;
}

int CTextUI::DisplayBufferPercentage(int iPerc)
{
	int x,y,c;
	GetConfigPos("TEXT_POS:BUFFER_PERCENTAGE", &x, &y);
	c = GetConfigColor("COLORS:ERROR_MESSAGE");

	if (iPerc > 97)
		iPerc = 100;
	if (iPerc < 2)
		iPerc = 0;
	uiPrintf(x, y, c, "Buffer: %03d%c%c", iPerc, 37, 37/* 37='%'*/);
	return 0;
}

int CTextUI::OnNewStreamStarted()
{
	int x,y;
	GetConfigPos("TEXT_POS:METADATA_ROW_RANGE", &x, &y);
	ClearRows(x, y);

	return 0;
}

int CTextUI::OnStreamOpening()
{
	int x,y,c;
	GetConfigPos("TEXT_POS:STREAM_OPENING", &x, &y);
	c = GetConfigColor("COLORS:STREAM_OPENING");
	
	ClearErrorMessage(); /** Clear any errors */
	ClearRows(y);
	uiPrintf(x, y, c, "Opening Stream");
	return 0;
}

int CTextUI::OnStreamOpeningError()
{
	int x,y,c;
	GetConfigPos("TEXT_POS:STREAM_OPENING_ERROR", &x, &y);
	c = GetConfigColor("COLORS:STREAM_OPENING_ERROR");
	
	ClearRows(y);
	uiPrintf(x, y, c, "Error Opening Stream");
	return 0;
}

int CTextUI::OnStreamOpeningSuccess()
{
	int x,y,c;
	GetConfigPos("TEXT_POS:STREAM_OPENING_SUCCESS", &x, &y);
	c = GetConfigColor("COLORS:STREAM_OPENING_SUCCESS");
	
	ClearRows(y);
	uiPrintf(x, y, c, "Stream Opened");
	return 0;
}

int CTextUI::OnVBlank()
{
	return 0;
}

int CTextUI::OnNewSongData(CPlayList::songmetadata *pData)
{
	int r1,r2;
	GetConfigPos("TEXT_POS:METADATA_ROW_RANGE", &r1, &r2);
	ClearRows(r1, r2);
	
	if (strlen(pData->strFileTitle) >= 59)
		pData->strFileTitle[59] = 0;
		
	if (strlen(pData->strURL) >= 59)
		pData->strURL[59] = 0;
	
	uiPrintf(0 , r1,	COLOR_WHITE,	"Stream: ");
	uiPrintf(8 , r1,	COLOR_CYAN,		"%s ", pData->strFileName);
	uiPrintf(0 , r1+1,	COLOR_WHITE,	"Title : ");
	uiPrintf(8 , r1+1,	COLOR_CYAN, 	"%s ", pData->strFileTitle);
	if (pData->strURL && strlen(pData->strURL))
	{
		uiPrintf(0, r1+2, COLOR_WHITE,	"URL   : ");
		uiPrintf(8, r1+2, COLOR_CYAN,	"%s ", pData->strURL);
	}
	uiPrintf(0,r1+3, COLOR_WHITE, "%lukbps %dHz MPEG layer %s stream",
			pData->BitRate/1000, 
			pData->SampleRate,
			pData->strMPEGLayer);
	return 0;
}

int CTextUI::OnConnectionProgress()
{
	printf(".");
	return 0;
}

int CTextUI::DisplayPLList(CDirList *plList)
{
	int x,y,c;
	GetConfigPos("TEXT_POS:PLAYLIST_DISPLAY", &x, &y);
	c = GetConfigColor("COLORS:PLAYLIST_DISPLAY");

	ClearRows(y);
	uiPrintf(x, y, c, "Current PlayList: %s", plList->GetCurrentURI());
	
	return 0;
}