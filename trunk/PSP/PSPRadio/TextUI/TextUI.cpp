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
#include <pspdebug.h>
#include <pspdisplay.h>
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
#include <stdarg.h>
#include <Logging.h>
#include "TextUI.h"

	
/* Define printf, just to make typing easier */
#define printf	pspDebugScreenPrintf

#define MAX_ROWS 34
#define MAX_COL  68

#define TEXT_UI_CFG_FILENAME "TextUI.cfg"

#define RGB2BGR(x) (((x>>16)&0xFF) | (x&0xFF00) | ((x<<16)&0xFF0000))

CTextUI::CTextUI()
{
	m_lockprint = NULL;
	m_lockclear = NULL;
	m_CurrentScreen = CScreenHandler::PSPRADIO_SCREEN_PLAYLIST;
	m_strTitle = strdup("PSPRadio by Raf");
	
	m_lockprint = new CLock("Print_Lock");
	m_lockclear = new CLock("Clear_Lock");

}

CTextUI::~CTextUI()
{
	Log(LOG_VERYLOW, "~CTextUI(): Start");
	if (m_lockprint)
	{
		delete(m_lockprint);
		m_lockprint = NULL;
	}
	if (m_lockclear)
	{
		delete(m_lockclear);
		m_lockclear = NULL;
	}
	Log(LOG_VERYLOW, "~CTextUI(): End");
}

int CTextUI::Initialize(char *strCWD)
{
	char *strCfgFile = NULL;

	strCfgFile = (char *)malloc(strlen(strCWD) + strlen(TEXT_UI_CFG_FILENAME) + 10);
	sprintf(strCfgFile, "%s/%s", strCWD, TEXT_UI_CFG_FILENAME);

	m_Config = new CIniParser(strCfgFile);
	
	free (strCfgFile), strCfgFile = NULL;
	
	pspDebugScreenInit();
	
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

void CTextUI::Initialize_Screen(CScreenHandler::Screen screen)
{
	int x,y,c;
	m_CurrentScreen = screen;
	switch (screen)
	{
		case CScreenHandler::PSPRADIO_SCREEN_SHOUTCAST_BROWSER:
		case CScreenHandler::PSPRADIO_SCREEN_PLAYLIST:
			pspDebugScreenSetBackColor(GetConfigColor("COLORS:BACKGROUND"));
			pspDebugScreenSetTextColor(GetConfigColor("COLORS:MAINTEXT"));
			pspDebugScreenClear(); 
			if (m_strTitle)
			{
				GetConfigPos("TEXT_POS:TITLE", &x, &y);
				c = GetConfigColor("COLORS:TITLE");
				uiPrintf(x,y, c, m_strTitle);
			}
			GetConfigPos("TEXT_POS:MAIN_COMMANDS", &x, &y);
			c = GetConfigColor("COLORS:MAIN_COMMANDS");
				
			ClearRows(y);
			uiPrintf(x, y, c, "X Play | [] Stop | ^ Cycle Screens | START Options");
			break;
		case CScreenHandler::PSPRADIO_SCREEN_OPTIONS:
			pspDebugScreenSetBackColor(GetConfigColor("COLORS:OPTIONS_SCREEN_BACKGROUND"));
			pspDebugScreenSetTextColor(GetConfigColor("COLORS:OPTIONS_SCREEN_MAINTEXT"));
			pspDebugScreenClear(); 
			uiPrintf(-1,0, GetConfigColor("COLORS:OPTIONS_SCREEN_MAINTEXT"), "PSPRadio OPTIONS:");
			break;
	
	}
}

void CTextUI::UpdateOptionsScreen(list<CScreenHandler::Options> &OptionsList, 
								list<CScreenHandler::Options>::iterator &CurrentOptionIterator)
{
	list<CScreenHandler::Options>::iterator OptionIterator;
	CScreenHandler::Options	Option;
	
	int x=-1,y=5,c=0xFFFFFF;
	
	if (OptionsList.size() > 0)
	{
		for (OptionIterator = OptionsList.begin() ; OptionIterator != OptionsList.end() ; OptionIterator++)
		{
			if (OptionIterator == CurrentOptionIterator)
			{
				c = GetConfigColor("COLORS:OPTIONS_SCREEN_OPTION_NAME_TEXT");//0xFFFFFF;
			}
			else
			{
				c = GetConfigColor("COLORS:OPTIONS_SCREEN_OPTION_SELECTED_NAME_TEXT");//0x888888;
			}
			
			Option = (*OptionIterator);
			
			ClearRows(y);
			PrintOption(x,y,c, Option.strName, Option.strStates, Option.iNumberOfStates, Option.iSelectedState, 
						Option.iActiveState);
			
			y+=2;
		}
	}
}

void CTextUI::PrintOption(int x, int y, int c, char *strName, char *strStates[], int iNumberOfStates, int iSelectedState,
						  int iActiveState)
{
	int iTextPos = 5;
	int color = 0xFFFFFF;
	
	//uiPrintf(x,y,c, "%s(%d): %s", strName, iSelectedState, strStates);
	uiPrintf(iTextPos,y,c, "%s: ", strName);
	if (iNumberOfStates > 0)
	{
		iTextPos += strlen(strName)+2;
		for (int iStates = 0; iStates < iNumberOfStates ; iStates++)
		{
			if (iStates+1 == iActiveState)
			{
				color = GetConfigColor("COLORS:OPTIONS_SCREEN_ACTIVE_STATE");//0x0000FF;
			}
			else if (iStates+1 == iSelectedState) /** 1-based */
			{
				color = GetConfigColor("COLORS:OPTIONS_SCREEN_SELECTED_STATE");//0xFFFFFF;
			}
			else
			{
				color = GetConfigColor("COLORS:OPTIONS_SCREEN_NOT_SELECTED_STATE");//0x888888;
			}
			
			if ((iStates+1 == iActiveState) && (iStates+1 == iSelectedState))
			{
				color =  GetConfigColor("COLORS:OPTIONS_SCREEN_ACTIVE_AND_SELECTED_STATE");//0x9090E3;
			}
			
			uiPrintf(iTextPos,y,color, "%s ", strStates[iStates]);
			iTextPos += strlen(strStates[iStates])+1;
		}
	}	
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
		printf("%67c", ' ');
	}
	m_lockclear->Unlock();
}

void CTextUI::ClearHalfRows(int iColStart, int iRowStart, int iRowEnd)
{
	if (iRowEnd == -1)
		iRowEnd = iRowStart;
		
	m_lockclear->Lock();
	for (int iRow = iRowStart ; (iRow < MAX_ROWS) && (iRow <= iRowEnd); iRow++)
	{
		pspDebugScreenSetXY(iColStart,iRow);
		printf("%33c", ' ');
	}
	m_lockclear->Unlock();
}
	
	
int CTextUI::SetTitle(char *strTitle)
{
//	int x,y;
//	int c;
//	GetConfigPos("TEXT_POS:TITLE", &x, &y);
//	c = GetConfigColor("COLORS:TITLE");
//	uiPrintf(x,y, c, strTitle);
	
	if (m_strTitle)
	{
		free(m_strTitle);
	}
	
	m_strTitle = strdup(strTitle);
	
	return 0;
}

int CTextUI::DisplayMessage_EnablingNetwork()
{
	int x,y,c;
	GetConfigPos("TEXT_POS:NETWORK_ENABLING", &x, &y);
	c = GetConfigColor("COLORS:NETWORK_ENABLING");
	
	//ClearErrorMessage();
	ClearRows(y);
	uiPrintf(x, y, c, "Enabling Network");
	
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
//	int x,y,c;
//	GetConfigPos("TEXT_POS:MAIN_COMMANDS", &x, &y);
//	c = GetConfigColor("COLORS:MAIN_COMMANDS");
	
//	ClearRows(y);
//	uiPrintf(x, y, c, "X Play/Pause | [] Stop | L / R To Browse");
		
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
		{
			uiPrintf(x, y, c, "STOP");
			int r1,r2;
			GetConfigPos("TEXT_POS:METADATA_ROW_RANGE", &r1, &r2);
			ClearRows(r1, r2);
			int px,py;
			GetConfigPos("TEXT_POS:BUFFER_PERCENTAGE", &px, &py);
			ClearRows(py);
			break;
		}
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
	c = GetConfigColor("COLORS:ERROR_MESSAGE");
	switch (m_CurrentScreen)
	{
		case CScreenHandler::PSPRADIO_SCREEN_SHOUTCAST_BROWSER:
		case CScreenHandler::PSPRADIO_SCREEN_PLAYLIST:
			GetConfigPos("TEXT_POS:ERROR_MESSAGE", &x, &y);
			ClearErrorMessage();
			break;
		case CScreenHandler::PSPRADIO_SCREEN_OPTIONS:
			GetConfigPos("TEXT_POS:ERROR_MESSAGE_IN_OPTIONS", &x, &y);
			ClearRows(y);
			break;
	}
	
	/** If message is longer than 2 lines, then truncate;
	The -10 is to accomodate for the "Error: " plus a bit.
	*/
	if (strlen(strMsg)>(MAX_COL*2 - 10))
	{
		strMsg[MAX_COL*2 - 10] = 0;
	}
	uiPrintf(x, y, c, "Error: %s", strMsg);
	
	return 0;
}

int CTextUI::DisplayMessage(char *strMsg)
{
	int x,y,c;
	c = GetConfigColor("COLORS:ERROR_MESSAGE");
	switch (m_CurrentScreen)
	{
		case CScreenHandler::PSPRADIO_SCREEN_SHOUTCAST_BROWSER:
		case CScreenHandler::PSPRADIO_SCREEN_PLAYLIST:
			GetConfigPos("TEXT_POS:ERROR_MESSAGE", &x, &y);
			ClearErrorMessage();
			break;
		case CScreenHandler::PSPRADIO_SCREEN_OPTIONS:
			GetConfigPos("TEXT_POS:ERROR_MESSAGE_IN_OPTIONS", &x, &y);
			ClearRows(y);
			break;
	}
	
	/** If message is longer than 2 lines, then truncate;
	 *  The -3 is just in case.
	 */
	if (strlen(strMsg)>(MAX_COL*2 - 10))
	{
		strMsg[MAX_COL*2 - 3] = 0;
	}
	uiPrintf(-1, y, c, "%s", strMsg);
	
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

	if (CScreenHandler::PSPRADIO_SCREEN_OPTIONS != m_CurrentScreen)
	{
		GetConfigPos("TEXT_POS:BUFFER_PERCENTAGE", &x, &y);
		c = GetConfigColor("COLORS:BUFFER_PERCENTAGE");
	
		if (iPerc > 97)
			iPerc = 100;
		if (iPerc < 2)
			iPerc = 0;

		uiPrintf(x, y, c, "Buffer: %03d%c%c", iPerc, 37, 37/* 37='%'*/);
	}
	return 0;
}

int CTextUI::OnNewStreamStarted()
{
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

int CTextUI::OnNewSongData(MetaData *pData)
{
	int r1,r2;
	GetConfigPos("TEXT_POS:METADATA_ROW_RANGE", &r1, &r2);
	ClearRows(r1, r2);
	
	if (strlen(pData->strTitle) >= 59)
		pData->strTitle[59] = 0;
		
	if (strlen(pData->strURL) >= 59)
		pData->strURL[59] = 0;
	
	if (0 != pData->iSampleRate)
	{
		uiPrintf(0,r1, COLOR_WHITE, "%lukbps %dHz (%d channels) stream",
				pData->iBitRate/1000, 
				pData->iSampleRate,
				pData->iNumberOfChannels);
				//pData->strMPEGLayer);
		r1++;
	}
	uiPrintf(0 , r1,	COLOR_WHITE,	"Stream: ");
	uiPrintf(8 , r1,	COLOR_CYAN,		"%s ", pData->strURI);
	r1++;
	uiPrintf(0 , r1,	COLOR_WHITE,	"Title : ");
	uiPrintf(8 , r1,	COLOR_CYAN, 	"%s ", pData->strTitle);
	r1++;
	if (pData->strArtist && strlen(pData->strArtist))
	{
		uiPrintf(0 , r1,	COLOR_WHITE,	"Artist: ");
		uiPrintf(8 , r1,	COLOR_CYAN, 	"%s ", pData->strArtist);
		r1++;
	}
	if (pData->strURL && strlen(pData->strURL))
	{
		uiPrintf(0, r1, COLOR_WHITE,	"URL   : ");
		uiPrintf(8, r1, COLOR_CYAN,	"%s ", pData->strURL);
	}
	return 0;
}

int CTextUI::OnConnectionProgress()
{
	printf(".");
	return 0;
}

int CTextUI::DisplayPLList(CDirList *plList)
{
	int x,y,c,r1,r2,ct,cs;
	list<CDirList::directorydata>::iterator ListIterator;
	list<CDirList::directorydata>::iterator *CurrentElement = plList->GetCurrentElementIterator();
	list<CDirList::directorydata> *List = plList->GetList();
	char *text = NULL;
	GetConfigPos("TEXT_POS:PLAYLIST_DIRS", &x, &y);
	GetConfigPos("TEXT_POS:PLAYLIST_ROW_RANGE", &r1, &r2);
	c = GetConfigColor("COLORS:PLAYLIST_ENTRIES");
	ct = GetConfigColor("COLORS:PLAYLIST_TITLE");
	cs = GetConfigColor("COLORS:PLAYLIST_SELECTED_ENTRY");
	int color = c;

	ClearHalfRows(x, r1+1,r2); /** Don't clear title (+1) */
	//uiPrintf(33/2 + x - 2/*list/2*/, y, ct, "List");
	y++;
	
	text = (char *)malloc (MAXPATHLEN);
	
	//Log(LOG_VERYLOW, "DisplayPLEntries(): populating screen");
	if (List->size() > 0)
	{
		ListIterator = *CurrentElement;
		for (int i = 0; i < (r2-r1)/2; i++)
		{
			if (ListIterator == List->begin())
				break;
			ListIterator--;
		
		}
		//Log(LOG_VERYLOW, "DisplayPLEntries(): elements: %d", List->size());
		for (; ListIterator != List->end() ; ListIterator++)
		{
			if (y > r2)
			{
				break;
			}
			
			if (ListIterator == *CurrentElement)
			{
				color = cs;
			}
			else
			{
				color = c;
			}
			
			//Log(LOG_VERYLOW, "DisplayPLEntries(): Using strURI='%s'", (*ListIterator).strURI);
			strncpy(text, basename((*ListIterator).strURI), 28);
			text[28] = 0;
		
			//Log(LOG_VERYLOW, "DisplayPLEntries(): Calling Print for text='%s'", text);
			uiPrintf(x, y, color, text);
			y+=1;
		}
	}
	
	free(text), text = NULL;
	
	return 0;
}

int CTextUI::DisplayPLEntries(CPlayList *PlayList)
{
	int x,y,c,r1,r2,ct,cs;
	list<MetaData>::iterator ListIterator;
	list<MetaData>::iterator *CurrentElement = PlayList->GetCurrentElementIterator();
	list<MetaData> *List = PlayList->GetList();
	char *text;
	GetConfigPos("TEXT_POS:PLAYLIST_ENTRIES", &x, &y);
	GetConfigPos("TEXT_POS:PLAYLIST_ROW_RANGE", &r1, &r2);
	c = GetConfigColor("COLORS:PLAYLIST_ENTRIES");
	cs = GetConfigColor("COLORS:PLAYLIST_SELECTED_ENTRY");
	ct = GetConfigColor("COLORS:PLAYLIST_TITLE");
	int color = c;

	ClearHalfRows(x, r1+1,r2); /** Don't clear Entry title */
	
	//uiPrintf(33/2 + x - 3/*entry/2*/, y, ct, "Entry");
	y++;
	
	text = (char *)malloc (MAXPATHLEN);
	
	
	
	//Log(LOG_VERYLOW, "DisplayPLEntries(): populating screen");
	if (List->size() > 0)
	{
		ListIterator = *CurrentElement;
		for (int i = 0; i < (r2-r1)/2; i++)
		{
			if (ListIterator == List->begin())
				break;
			ListIterator--;
		
		}
		//Log(LOG_VERYLOW, "DisplayPLEntries(): elements: %d", List->size());
		for (; ListIterator != List->end() ; ListIterator++)
		{
			if (y > r2)
			{
				break;
			}
			
			if (ListIterator == *CurrentElement)
			{
				color = cs;
			}
			else
			{
				color = c;
			}
			
			if (strlen((*ListIterator).strTitle))
			{
				//Log(LOG_VERYLOW, "DisplayPLEntries(): Using strTitle='%s'", (*ListIterator).strTitle);
				strncpy(text, (*ListIterator).strTitle, 28);
				text[28] = 0;
			}
			else
			{
				//Log(LOG_VERYLOW, "DisplayPLEntries(): Using strURI='%s'", (*ListIterator).strURI);
				strncpy(text, (*ListIterator).strURI, 28);
				text[28] = 0;
			}
		
			//Log(LOG_VERYLOW, "DisplayPLEntries(): Calling Print for text='%s'", text);
			uiPrintf(x, y, color, text);
			y+=1;
		}
	}
	
	free(text), text = NULL;
	
	return 0;
}

int CTextUI::OnCurrentPlayListSideSelectionChange(CScreenHandler::PlayListSide CurrentPlayListSideSelection)
{
	int r1,r2, ct, iListX, iEntryX, y;
	GetConfigPos("TEXT_POS:PLAYLIST_ENTRIES", &iEntryX, &y);
	GetConfigPos("TEXT_POS:PLAYLIST_DIRS", &iListX, &y);
	GetConfigPos("TEXT_POS:PLAYLIST_ROW_RANGE", &r1, &r2);
	ct = GetConfigColor("COLORS:PLAYLIST_TITLE");

	ClearRows(r1);
	
	switch (CurrentPlayListSideSelection)
	{
		case CScreenHandler::PLAYLIST_LIST:
			uiPrintf(33/2 + iListX - 4/*entry/2*/,  r1, ct, "*List*");
			uiPrintf(33/2 + iEntryX - 4/*entry/2*/, r1, ct, "Entries");
			break;
		
		case CScreenHandler::PLAYLIST_ENTRIES:
			uiPrintf(33/2 + iListX - 3/*entry/2*/,  r1, ct, "List");
			uiPrintf(33/2 + iEntryX - 5/*entry/2*/, r1, ct, "*Entries*");
			break;
	
	}
	
	return 0;
}

