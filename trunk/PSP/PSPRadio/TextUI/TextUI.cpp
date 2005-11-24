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
#include <Screen.h>
#include "TextUI.h"

#define MAX_ROWS 34
#define MAX_COL  68

#define TEXT_UI_CFG_FILENAME "TextUI/TextUI.cfg"

#define RGB2BGR(x) (((x>>16)&0xFF) | (x&0xFF00) | ((x<<16)&0xFF0000))

CTextUI::CTextUI()
{
	m_lockprint = NULL;
	m_lockclear = NULL;
	m_CurrentScreen = CScreenHandler::PSPRADIO_SCREEN_PLAYLIST;
	m_Screen = new CScreen;
	m_strTitle = strdup("PSPRadio by Raf");
	
	m_lockprint = new CLock("Print_Lock");
	m_lockclear = new CLock("Clear_Lock");

	m_isdirty = false;
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
	if (m_Config)
	{
		delete(m_Config);
		m_Config = NULL;
	}
	
	delete(m_Screen), m_Screen = NULL;
	Log(LOG_VERYLOW, "~CTextUI(): End");
}

int CTextUI::Initialize(char *strCWD)
{
	char *strCfgFile = NULL;

	strCfgFile = (char *)malloc(strlen(strCWD) + strlen(TEXT_UI_CFG_FILENAME) + 10);
	sprintf(strCfgFile, "%s/%s", strCWD, TEXT_UI_CFG_FILENAME);

	m_Config = new CIniParser(strCfgFile);
	
	free (strCfgFile), strCfgFile = NULL;
	
	//m_Screen->Init();
	
	return 0;
}

int CTextUI::GetConfigColor(char *strKey)
{
	int iRet;
	sscanf(m_Config->GetStr(strKey), "%x", &iRet);
	
	return RGB2BGR(iRet);
}

void CTextUI::GetConfigPair(char *strKey, int *x, int *y)
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
	
	m_Screen->SetTextMode((enum textmode)m_Config->GetInteger("OTHER:TEXT_MODE", 0));
	
	switch (screen)
	{
		case CScreenHandler::PSPRADIO_SCREEN_SHOUTCAST_BROWSER:
			m_Screen->SetBackColor(GetConfigColor("COLORS:BACKGROUND"));
			m_Screen->SetTextColor(GetConfigColor("COLORS:MAINTEXT"));
			if (m_Config->GetString("IMAGES:SHOUTCAST_SCREEN_BACKGROUND", NULL))
			{
				m_Screen->SetBackgroundImage(m_Config->GetStr("IMAGES:SHOUTCAST_SCREEN_BACKGROUND"));
			}
			m_Screen->Clear(); 
			if (m_strTitle)
			{
				GetConfigPair("TEXT_POS:TITLE", &x, &y);
				c = GetConfigColor("COLORS:TITLE");
				uiPrintf(x,y, c, m_strTitle);
			}
			GetConfigPair("TEXT_POS:MAIN_COMMANDS", &x, &y);
			c = GetConfigColor("COLORS:MAIN_COMMANDS");
				
			ClearRows(y);
			uiPrintf(x, y, c, "X Play | [] Stop | ^ Cycle Screens | START Options");
			break;
		
		case CScreenHandler::PSPRADIO_SCREEN_PLAYLIST:
			m_Screen->SetBackColor(GetConfigColor("COLORS:BACKGROUND"));
			m_Screen->SetTextColor(GetConfigColor("COLORS:MAINTEXT"));
			if (m_Config->GetString("IMAGES:MAIN_SCREEN_BACKGROUND", NULL))
			{
				m_Screen->SetBackgroundImage(m_Config->GetStr("IMAGES:MAIN_SCREEN_BACKGROUND"));
			}
			m_Screen->Clear(); 
			if (m_strTitle)
			{
				GetConfigPair("TEXT_POS:TITLE", &x, &y);
				c = GetConfigColor("COLORS:TITLE");
				uiPrintf(x,y, c, m_strTitle);
			}
			GetConfigPair("TEXT_POS:MAIN_COMMANDS", &x, &y);
			c = GetConfigColor("COLORS:MAIN_COMMANDS");
				
			ClearRows(y);
			uiPrintf(x, y, c, "X Play | [] Stop | ^ Cycle Screens | START Options");
			break;
		
		case CScreenHandler::PSPRADIO_SCREEN_OPTIONS:
			m_Screen->SetBackColor(GetConfigColor("COLORS:OPTIONS_SCREEN_BACKGROUND"));
			m_Screen->SetTextColor(GetConfigColor("COLORS:OPTIONS_SCREEN_MAINTEXT"));
			if (m_Config->GetString("IMAGES:OPTIONS_SCREEN_BACKGROUND", NULL))
			{
				m_Screen->SetBackgroundImage(m_Config->GetStr("IMAGES:OPTIONS_SCREEN_BACKGROUND"));
			}
			m_Screen->Clear(); 
			uiPrintf(-1,0, GetConfigColor("COLORS:OPTIONS_SCREEN_MAINTEXT"), "PSPRadio OPTIONS:");
			break;
	
	}
}

int CTextUI::OnVBlank()
{
	if (m_isdirty)
	{
		//flip
		m_isdirty = false;
	}
	return 0;
}

void CTextUI::UpdateOptionsScreen(list<OptionsScreen::Options> &OptionsList, 
										 list<OptionsScreen::Options>::iterator &CurrentOptionIterator)
{
	list<OptionsScreen::Options>::iterator OptionIterator;
	OptionsScreen::Options	Option;
	
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
	m_Screen->SetXY(x,y);
	m_Screen->SetTextColor(color);
	m_Screen->Printf(msg);
	
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
		m_Screen->ClearLine(iRow);
	}
	m_lockclear->Unlock();
}

void CTextUI::ClearHalfRows(int iColStart, int iColEnd, int iRowStart, int iRowEnd)
{
	if (iRowEnd == -1)
		iRowEnd = iRowStart;
		
	m_lockclear->Lock();
	for (int iRow = iRowStart ; (iRow < MAX_ROWS) && (iRow <= iRowEnd); iRow++)
	{
		m_Screen->ClearNChars(iColStart, iRow, iColEnd - iColStart);
	}
	m_lockclear->Unlock();
}
	
	
int CTextUI::SetTitle(char *strTitle)
{
//	int x,y;
//	int c;
//	GetConfigPair("TEXT_POS:TITLE", &x, &y);
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
	GetConfigPair("TEXT_POS:NETWORK_ENABLING", &x, &y);
	c = GetConfigColor("COLORS:NETWORK_ENABLING");
	
	//ClearErrorMessage();
	ClearRows(y);
	uiPrintf(x, y, c, "Enabling Network");
	
	return 0;
}

int CTextUI::DisplayMessage_DisablingNetwork()
{
	int x,y,c;
	GetConfigPair("TEXT_POS:NETWORK_DISABLING", &x, &y);
	c = GetConfigColor("COLORS:NETWORK_DISABLING");
	
	ClearRows(y);
	uiPrintf(x, y, c, "Disabling Network");
	
	return 0;
}

int CTextUI::DisplayMessage_NetworkReady(char *strIP)
{
	int x,y,c;
	GetConfigPair("TEXT_POS:NETWORK_READY", &x, &y);
	c = GetConfigColor("COLORS:NETWORK_READY");
	
	ClearRows(y);
	uiPrintf(x, y, c, "Ready, IP %s", strIP);
	
	return 0;
}

int CTextUI::DisplayMainCommands()
{
//	int x,y,c;
//	GetConfigPair("TEXT_POS:MAIN_COMMANDS", &x, &y);
//	c = GetConfigColor("COLORS:MAIN_COMMANDS");
	
//	ClearRows(y);
//	uiPrintf(x, y, c, "X Play/Pause | [] Stop | L / R To Browse");
		
	return 0;
}

int CTextUI::DisplayActiveCommand(CPSPSound::pspsound_state playingstate)
{
	int x,y,c;
	GetConfigPair("TEXT_POS:ACTIVE_COMMAND", &x, &y);
	c = GetConfigColor("COLORS:ACTIVE_COMMAND");
	
	ClearRows(y);
	switch(playingstate)
	{
	case CPSPSound::STOP:
		{
			uiPrintf(x, y, c, "STOP");
			int r1,r2;
			GetConfigPair("TEXT_POS:METADATA_ROW_RANGE", &r1, &r2);
			ClearRows(r1, r2);
			int px,py;
			GetConfigPair("TEXT_POS:BUFFER_PERCENTAGE", &px, &py);
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
			GetConfigPair("TEXT_POS:ERROR_MESSAGE", &x, &y);
			ClearErrorMessage();
			break;
		case CScreenHandler::PSPRADIO_SCREEN_OPTIONS:
			GetConfigPair("TEXT_POS:ERROR_MESSAGE_IN_OPTIONS", &x, &y);
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
			GetConfigPair("TEXT_POS:ERROR_MESSAGE", &x, &y);
			ClearErrorMessage();
			break;
		case CScreenHandler::PSPRADIO_SCREEN_OPTIONS:
			GetConfigPair("TEXT_POS:ERROR_MESSAGE_IN_OPTIONS", &x, &y);
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
	GetConfigPair("TEXT_POS:ERROR_MESSAGE_ROW_RANGE", &x, &y);
	ClearRows(x, y);
	return 0;
}

int CTextUI::DisplayBufferPercentage(int iPerc)
{
	int x,y,c;

	if (CScreenHandler::PSPRADIO_SCREEN_OPTIONS != m_CurrentScreen)
	{
		GetConfigPair("TEXT_POS:BUFFER_PERCENTAGE", &x, &y);
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
	GetConfigPair("TEXT_POS:STREAM_OPENING", &x, &y);
	c = GetConfigColor("COLORS:STREAM_OPENING");
	
	ClearErrorMessage(); /** Clear any errors */
	ClearRows(y);
	uiPrintf(x, y, c, "Opening Stream");
	return 0;
}

int CTextUI::OnStreamOpeningError()
{
	int x,y,c;
	GetConfigPair("TEXT_POS:STREAM_OPENING_ERROR", &x, &y);
	c = GetConfigColor("COLORS:STREAM_OPENING_ERROR");
	
	ClearRows(y);
	uiPrintf(x, y, c, "Error Opening Stream");
	return 0;
}

int CTextUI::OnStreamOpeningSuccess()
{
	int x,y,c;
	GetConfigPair("TEXT_POS:STREAM_OPENING_SUCCESS", &x, &y);
	c = GetConfigColor("COLORS:STREAM_OPENING_SUCCESS");
	
	ClearRows(y);
	uiPrintf(x, y, c, "Stream Opened");
	return 0;
}

int CTextUI::OnNewSongData(MetaData *pData)
{
	int r1,r2;

	if (CScreenHandler::PSPRADIO_SCREEN_OPTIONS != m_CurrentScreen)
	{

		GetConfigPair("TEXT_POS:METADATA_ROW_RANGE", &r1, &r2);
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
	}
	return 0;
}

int CTextUI::OnConnectionProgress()
{
	m_Screen->Printf(".");
	return 0;
}

/*
PLAYLIST_CONTAINERLIST_START_COLUMN=0,14;
PLAYLIST_CONTAINERLIST_END_COLUMN=17;
PLAYLIST_CONTAINERLIST_ROW_RANGE=14,28
*/
void CTextUI::DisplayContainers(CMetaDataContainer *Container)
{
	//int x,y,c,r1,r2,ct,cs,ex,ey;
	int iStartCol, iEndCol, iRowStart, iRowEnd;
	int iColorNormal, iColorSelected, iColorTitle, iColor;
	int iNextRow = 0;

	map< string, list<MetaData>* >::iterator ListIterator;
	map< string, list<MetaData>* >::iterator *CurrentElement = Container->GetCurrentContainerIterator();
	map< string, list<MetaData>* > *List = Container->GetContainerList();
	char *strText = NULL;
	//GetConfigPair("TEXT_POS:PLAYLIST_DIRS", &x, &y);
	//GetConfigPair("TEXT_POS:PLAYLIST_ENTRIES", &ex, &ey);
	//GetConfigPair("TEXT_POS:PLAYLIST_ROW_RANGE", &r1, &r2);
	iColorNormal   = GetConfigColor("COLORS:PLAYLIST_ENTRIES");
	iColorTitle    = GetConfigColor("COLORS:PLAYLIST_TITLE");
	iColorSelected = GetConfigColor("COLORS:PLAYLIST_SELECTED_ENTRY");
	iStartCol = m_Config->GetInteger("TEXT_POS:PLAYLIST_CONTAINERLIST_START_COLUMN", 0);
	iEndCol   = m_Config->GetInteger("TEXT_POS:PLAYLIST_CONTAINERLIST_END_COLUMN", 0);
	GetConfigPair("TEXT_POS:PLAYLIST_CONTAINERLIST_ROW_RANGE", &iRowStart, &iRowEnd);
	//int color = c;
	iColor = iColorNormal;

	ClearHalfRows(iStartCol, iEndCol, iRowStart+1, iRowEnd); /** Don't clear title (+1) */

	iNextRow = iRowStart + 1; /** Start after the title */
	
	strText = (char *)malloc (MAXPATHLEN);
	
	//Log(LOG_VERYLOW, "DisplayContainers(): populating screen");
	if (List->size() > 0)
	{
		//Log(LOG_VERYLOW, "DisplayContainers(): Setting iterator to middle of the screen");
		ListIterator = *CurrentElement;
		for (int i = 0; i < (iRowEnd-iRowStart)/2; i++)
		{
			if (ListIterator == List->begin())
				break;
			ListIterator--;
		
		}

		//Log(LOG_VERYLOW, "DisplayPLEntries(): elements: %d", List->size());
		//Log(LOG_VERYLOW, "DisplayContainers(): Populating Screen (total elements %d)", List->size());
		for (; ListIterator != List->end() ; ListIterator++)
		{
			if (iNextRow > iRowEnd)
			{
				break;
			}
			
			if (ListIterator == *CurrentElement)
			{
				iColor = iColorSelected;
			}
			else
			{
				iColor = iColorNormal;
			}
			
			strncpy(strText, ListIterator->first.c_str(), MAXPATHLEN - 1);
			strText[MAXPATHLEN - 1] = 0;

			if (strlen(strText) > 4 && memcmp(strText, "ms0:", 4) == 0)
			{
				char *pText = basename(strText);
				pText[iEndCol-iStartCol - 2] = 0;
				uiPrintf(iStartCol, iNextRow, iColor, pText);
			}
			else
			{
				strText[iEndCol-iStartCol - 2] = 0;
				uiPrintf(iStartCol, iNextRow, iColor, strText);
			}
			iNextRow++;
		}
	}
	
	free(strText), strText = NULL;
}

/*
PLAYLIST_ENTRIESLIST_START_COLUMN=18
PLAYLIST_ENTRIESLIST_END_COLUMN=67;
PLAYLIST_ENTRIESLIST_ROW_RANGE=14,28
*/
void CTextUI::DisplayElements(CMetaDataContainer *Container)
{
	int iStartCol,iEndCol,iRowStart,iRowEnd, iNextRow;
	int iColorNormal,iColorTitle,iColorSelected, iColor;
	char *strText = NULL;
	
	list<MetaData>::iterator ListIterator;
	list<MetaData>::iterator *CurrentElement = Container->GetCurrentElementIterator();
	list<MetaData> *List = Container->GetElementList();
	//GetConfigPair("TEXT_POS:PLAYLIST_ENTRIES", &x, &y);
	//GetConfigPair("TEXT_POS:PLAYLIST_ROW_RANGE", &iRowStart, &iRowEnd);
	iColorNormal = GetConfigColor("COLORS:PLAYLIST_ENTRIES");
	iColorSelected = GetConfigColor("COLORS:PLAYLIST_SELECTED_ENTRY");
	iColorTitle = GetConfigColor("COLORS:PLAYLIST_TITLE");
	iStartCol = m_Config->GetInteger("TEXT_POS:PLAYLIST_ENTRIESLIST_START_COLUMN", 0);
	iEndCol   = m_Config->GetInteger("TEXT_POS:PLAYLIST_ENTRIESLIST_END_COLUMN", 0);
	GetConfigPair("TEXT_POS:PLAYLIST_ENTRIESLIST_ROW_RANGE", &iRowStart, &iRowEnd);
	iColor = iColorNormal;

	ClearHalfRows(iStartCol,iEndCol, iRowStart+1,iRowEnd); /** Don't clear Entry title */
	
	//uiPrintf(33/2 + x - 3/*entry/2*/, y, ct, "Entry");
	iNextRow = iRowStart + 1;
	
	strText = (char *)malloc (MAXPATHLEN);
	
	//Log(LOG_VERYLOW, "DisplayPLEntries(): populating screen");
	if (List->size() > 0)
	{
		ListIterator = *CurrentElement;
		for (int i = 0; i < (iRowEnd-iRowStart)/2; i++)
		{
			if (ListIterator == List->begin())
				break;
			ListIterator--;
		
		}
		//Log(LOG_VERYLOW, "DisplayPLEntries(): elements: %d", List->size());
		for (; ListIterator != List->end() ; ListIterator++)
		{
			if (iNextRow > iRowEnd)
			{
				break;
			}
			
			if (ListIterator == *CurrentElement)
			{
				iColor = iColorSelected;
			}
			else
			{
				iColor = iColorNormal;
			}
			
			if (strlen((*ListIterator).strTitle))
			{
				//Log(LOG_VERYLOW, "DisplayPLEntries(): Using strTitle='%s'", (*ListIterator).strTitle);
				strncpy(strText, (*ListIterator).strTitle, (iEndCol-iStartCol));
				strText[iEndCol-iStartCol] = 0;
			}
			else
			{
				//Log(LOG_VERYLOW, "DisplayPLEntries(): Using strURI='%s'", (*ListIterator).strURI);
				strncpy(strText, (*ListIterator).strURI, (iEndCol-iStartCol));
				strText[iEndCol-iStartCol] = 0;
			}
		
			//Log(LOG_VERYLOW, "DisplayPLEntries(): Calling Print for strText='%s'", strText);
			uiPrintf(iStartCol, iNextRow, iColor, strText);
			iNextRow++;
		}
	}
	
	free(strText), strText = NULL;
}

void CTextUI::OnCurrentContainerSideChange(CMetaDataContainer *Container)
{
	//int r1,r2, ct, iListX, iEntryX, y;
	//GetConfigPair("TEXT_POS:PLAYLIST_ENTRIES", &iEntryX, &y);
	//GetConfigPair("TEXT_POS:PLAYLIST_DIRS", &iListX, &y);
	//GetConfigPair("TEXT_POS:PLAYLIST_ROW_RANGE", &r1, &r2);
	//ct = GetConfigColor("COLORS:PLAYLIST_TITLE");
	int iContainer_StartCol, iContainer_EndCol, iContainer_RowStart, iContainer_RowEnd;
	int iEntries_StartCol, iEntries_EndCol, iEntries_RowStart, iEntries_RowEnd;
	int iColorTitle;
	
	iContainer_StartCol = m_Config->GetInteger("TEXT_POS:PLAYLIST_CONTAINERLIST_START_COLUMN", 0);
	iContainer_EndCol   = m_Config->GetInteger("TEXT_POS:PLAYLIST_CONTAINERLIST_END_COLUMN", 0);
	GetConfigPair("TEXT_POS:PLAYLIST_CONTAINERLIST_ROW_RANGE", &iContainer_RowStart, &iContainer_RowEnd);
	
	iEntries_StartCol = m_Config->GetInteger("TEXT_POS:PLAYLIST_ENTRIESLIST_START_COLUMN", 0);
	iEntries_EndCol   = m_Config->GetInteger("TEXT_POS:PLAYLIST_ENTRIESLIST_END_COLUMN", 0);
	GetConfigPair("TEXT_POS:PLAYLIST_ENTRIESLIST_ROW_RANGE", &iEntries_RowStart, &iEntries_RowEnd);
	
	iColorTitle = GetConfigColor("COLORS:PLAYLIST_TITLE");

	ClearRows(iContainer_RowStart);
	if (iContainer_RowStart != iEntries_RowStart)
		ClearRows(iEntries_RowStart);
	
	
	switch (Container->GetCurrentSide())
	{
		case CMetaDataContainer::CONTAINER_SIDE_CONTAINERS:
// 			uiPrintf(33/2 + iListX - 4/*entry/2*/,  r1, ct, "*List*");
// 			uiPrintf(33/2 + iEntryX - 4/*entry/2*/, r1, ct, "Entries");
			uiPrintf((iContainer_EndCol - iContainer_StartCol)/2 - 4/*entry/2*/,  iContainer_RowStart, iColorTitle, "*List*");
			uiPrintf((iEntries_EndCol - iEntries_StartCol)/2+iEntries_StartCol - 4/*entry/2*/, iEntries_RowStart, iColorTitle, "Entries");
			break;
		
		case CMetaDataContainer::CONTAINER_SIDE_ELEMENTS:
//			uiPrintf(33/2 + iListX - 3/*entry/2*/,  r1, ct, "List");
//			uiPrintf(33/2 + iEntryX - 5/*entry/2*/, r1, ct, "*Entries*");
			uiPrintf((iContainer_EndCol - iContainer_StartCol)/2 - 3/*entry/2*/,  iContainer_RowStart, iColorTitle, "List");
			uiPrintf((iEntries_EndCol - iEntries_StartCol)/2+iEntries_StartCol - 5/*entry/2*/, iEntries_RowStart, iColorTitle, "*Entries*");
			break;
	
	}
}
