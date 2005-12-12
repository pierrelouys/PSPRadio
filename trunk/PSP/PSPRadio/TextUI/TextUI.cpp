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

//#define MAX_ROWS 34
//#define MAX_COL  68
#define MAX_ROWS 		m_Screen->GetNumberOfTextRows()
#define MAX_COL 		m_Screen->GetNumberOfTextColumns()
#define PIXEL_TO_ROW(y)	((y)/m_Screen->GetFontHeight())
#define PIXEL_TO_COL(x) ((x)/m_Screen->GetFontWidth())
#define COL_TO_PIXEL(c) ((c)*m_Screen->GetFontWidth())
#define ROW_TO_PIXEL(r) ((r)*m_Screen->GetFontHeight())


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
	m_LastBatteryPercentage = 0;
	sceRtcGetCurrentClockLocalTime(&m_LastLocalTime);
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
	
	memset(&m_ScreenConfig, 0, sizeof(m_ScreenConfig));
	
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

void CTextUI::LoadConfigSettings(CScreenHandler::Screen screen)
{
	switch (screen)
	{
		case CScreenHandler::PSPRADIO_SCREEN_SHOUTCAST_BROWSER:
			m_ScreenConfig.FontMode   = (CScreen::textmode)m_Config->GetInteger("SCREEN_SHOUTCAST:FONT_MODE", 0);
			m_ScreenConfig.FontWidth  = m_Config->GetInteger("SCREEN_SHOUTCAST:FONT_WIDTH", 7);
			m_ScreenConfig.FontHeight = m_Config->GetInteger("SCREEN_SHOUTCAST:FONT_HEIGHT", 8);
			m_ScreenConfig.strBackground = m_Config->GetString("SCREEN_SHOUTCAST:BACKGROUND", NULL);
			m_ScreenConfig.BgColor = GetConfigColor("SCREEN_SHOUTCAST:BG_COLOR");
			m_ScreenConfig.FgColor = GetConfigColor("SCREEN_SHOUTCAST:FG_COLOR");
			GetConfigPair("SCREEN_SHOUTCAST:CONTAINERLIST_X_RANGE", 
									&m_ScreenConfig.ContainerListRangeX1, &m_ScreenConfig.ContainerListRangeX2);
			GetConfigPair("SCREEN_SHOUTCAST:CONTAINERLIST_Y_RANGE", 
									&m_ScreenConfig.ContainerListRangeY1, &m_ScreenConfig.ContainerListRangeY2);
			GetConfigPair("SCREEN_SHOUTCAST:ENTRIESLIST_X_RANGE", 
									&m_ScreenConfig.EntriesListRangeX1, &m_ScreenConfig.EntriesListRangeX2);
			GetConfigPair("SCREEN_SHOUTCAST:ENTRIESLIST_Y_RANGE", 
									&m_ScreenConfig.EntriesListRangeY1, &m_ScreenConfig.EntriesListRangeY2);
			GetConfigPair("SCREEN_SHOUTCAST:BUFFER_PERCENTAGE_XY", 
									&m_ScreenConfig.BufferPercentageX, &m_ScreenConfig.BufferPercentageY);
			m_ScreenConfig.BufferPercentageColor = GetConfigColor("SCREEN_SHOUTCAST:BUFFER_PERCENTAGE_COLOR");
			m_ScreenConfig.MetadataX1 = m_Config->GetInteger("SCREEN_SHOUTCAST:METADATA_X", 7);
			GetConfigPair("SCREEN_SHOUTCAST:METADATA_Y_RANGE", 
									&m_ScreenConfig.MetadataRangeY1, &m_ScreenConfig.MetadataRangeY2);
			m_ScreenConfig.ListsTitleColor = GetConfigColor("SCREEN_SHOUTCAST:LISTS_TITLE_COLOR");
			m_ScreenConfig.EntriesListColor = GetConfigColor("SCREEN_SHOUTCAST:ENTRIESLIST_COLOR");
			m_ScreenConfig.SelectedEntryColor = GetConfigColor("SCREEN_SHOUTCAST:SELECTED_ENTRY_COLOR");
			GetConfigPair("SCREEN_SHOUTCAST:PROGRAM_VERSION_XY", 
									&m_ScreenConfig.ProgramVersionX, &m_ScreenConfig.ProgramVersionY);
			m_ScreenConfig.ProgramVersionColor = GetConfigColor("SCREEN_SHOUTCAST:PROGRAM_VERSION_COLOR");
			GetConfigPair("SCREEN_SHOUTCAST:STREAM_OPENING_XY", 
									&m_ScreenConfig.StreamOpeningX, &m_ScreenConfig.StreamOpeningY);
			GetConfigPair("SCREEN_SHOUTCAST:STREAM_OPENING_ERROR_XY", 
									&m_ScreenConfig.StreamOpeningErrorX, &m_ScreenConfig.StreamOpeningErrorY);
			GetConfigPair("SCREEN_SHOUTCAST:STREAM_OPENING_SUCCESS_XY", 
									&m_ScreenConfig.StreamOpeningSuccessX, &m_ScreenConfig.StreamOpeningSuccessY);
			m_ScreenConfig.StreamOpeningColor = GetConfigColor("SCREEN_SHOUTCAST:STREAM_OPENING_COLOR");
			m_ScreenConfig.StreamOpeningErrorColor = GetConfigColor("SCREEN_SHOUTCAST:STREAM_OPENING_ERROR_COLOR");
			m_ScreenConfig.StreamOpeningSuccessColor = GetConfigColor("SCREEN_SHOUTCAST:STREAM_OPENING_SUCCESS_COLOR");
			GetConfigPair("SCREEN_SHOUTCAST:CLEAN_ON_NEW_STREAM_Y_RANGE", 
									&m_ScreenConfig.CleanOnNewStreamRangeY1, &m_ScreenConfig.CleanOnNewStreamRangeY2);
			GetConfigPair("SCREEN_SHOUTCAST:ACTIVE_COMMAND_XY", 
									&m_ScreenConfig.ActiveCommandX, &m_ScreenConfig.ActiveCommandY);
			GetConfigPair("SCREEN_SHOUTCAST:ERROR_MESSAGE_XY", 
									&m_ScreenConfig.ErrorMessageX, &m_ScreenConfig.ErrorMessageY);
			m_ScreenConfig.ActiveCommandColor = GetConfigColor("SCREEN_SHOUTCAST:ACTIVE_COMMAND_COLOR");
			m_ScreenConfig.ErrorMessageColor = GetConfigColor("SCREEN_SHOUTCAST:ERROR_MESSAGE_COLOR");
			GetConfigPair("SCREEN_SHOUTCAST:NETWORK_ENABLING_XY", 
									&m_ScreenConfig.NetworkEnablingX, &m_ScreenConfig.NetworkEnablingY);
			GetConfigPair("SCREEN_SHOUTCAST:NETWORK_DISABLING_XY", 
									&m_ScreenConfig.NetworkDisablingX, &m_ScreenConfig.NetworkDisablingY);
			GetConfigPair("SCREEN_SHOUTCAST:NETWORK_READY_XY", 
									&m_ScreenConfig.NetworkReadyX, &m_ScreenConfig.NetworkReadyY);
			m_ScreenConfig.NetworkEnablingColor = GetConfigColor("SCREEN_SHOUTCAST:NETWORK_ENABLING_COLOR");
			m_ScreenConfig.NetworkDisablingColor = GetConfigColor("SCREEN_SHOUTCAST:NETWORK_DISABLING_COLOR");
			m_ScreenConfig.NetworkReadyColor = GetConfigColor("SCREEN_SHOUTCAST:NETWORK_READY_COLOR");
			GetConfigPair("SCREEN_SHOUTCAST:CLOCK_XY", 
									&m_ScreenConfig.ClockX, &m_ScreenConfig.ClockY);
			m_ScreenConfig.ClockColor = GetConfigColor("SCREEN_SHOUTCAST:CLOCK_COLOR");
			GetConfigPair("SCREEN_SHOUTCAST:BATTERY_XY", 
									&m_ScreenConfig.BatteryX, &m_ScreenConfig.BatteryY);
			m_ScreenConfig.BatteryColor = GetConfigColor("SCREEN_SHOUTCAST:BATTERY_COLOR");
			break;
			
		case CScreenHandler::PSPRADIO_SCREEN_PLAYLIST:
			m_ScreenConfig.FontMode   = (CScreen::textmode)m_Config->GetInteger("SCREEN_SHOUTCAST:FONT_MODE", 0);
			m_ScreenConfig.FontWidth  = m_Config->GetInteger("SCREEN_PLAYLIST:FONT_WIDTH", 7);
			m_ScreenConfig.FontHeight = m_Config->GetInteger("SCREEN_PLAYLIST:FONT_HEIGHT", 8);
			m_ScreenConfig.strBackground = m_Config->GetString("SCREEN_PLAYLIST:BACKGROUND", NULL);
			m_ScreenConfig.BgColor = GetConfigColor("SCREEN_PLAYLIST:BG_COLOR");
			m_ScreenConfig.FgColor = GetConfigColor("SCREEN_PLAYLIST:FG_COLOR");
			GetConfigPair("SCREEN_PLAYLIST:CONTAINERLIST_X_RANGE", 
									&m_ScreenConfig.ContainerListRangeX1, &m_ScreenConfig.ContainerListRangeX2);
			GetConfigPair("SCREEN_PLAYLIST:CONTAINERLIST_Y_RANGE", 
									&m_ScreenConfig.ContainerListRangeY1, &m_ScreenConfig.ContainerListRangeY2);
			GetConfigPair("SCREEN_PLAYLIST:ENTRIESLIST_X_RANGE", 
									&m_ScreenConfig.EntriesListRangeX1, &m_ScreenConfig.EntriesListRangeX2);
			GetConfigPair("SCREEN_PLAYLIST:ENTRIESLIST_Y_RANGE", 
									&m_ScreenConfig.EntriesListRangeY1, &m_ScreenConfig.EntriesListRangeY2);
			m_ScreenConfig.ListsTitleColor = GetConfigColor("SCREEN_PLAYLIST:LISTS_TITLE_COLOR");
			m_ScreenConfig.EntriesListColor = GetConfigColor("SCREEN_PLAYLIST:ENTRIESLIST_COLOR");
			m_ScreenConfig.SelectedEntryColor = GetConfigColor("SCREEN_PLAYLIST:SELECTED_ENTRY_COLOR");
			GetConfigPair("SCREEN_PLAYLIST:BUFFER_PERCENTAGE_XY", 
									&m_ScreenConfig.BufferPercentageX, &m_ScreenConfig.BufferPercentageY);
			m_ScreenConfig.BufferPercentageColor = GetConfigColor("SCREEN_PLAYLIST:BUFFER_PERCENTAGE_COLOR");
			m_ScreenConfig.MetadataX1 = m_Config->GetInteger("SCREEN_PLAYLIST:METADATA_X", 7);
			GetConfigPair("SCREEN_PLAYLIST:METADATA_Y_RANGE", 
									&m_ScreenConfig.MetadataRangeY1, &m_ScreenConfig.MetadataRangeY2);
			GetConfigPair("SCREEN_PLAYLIST:PROGRAM_VERSION_XY", 
									&m_ScreenConfig.ProgramVersionX, &m_ScreenConfig.ProgramVersionY);
			m_ScreenConfig.ProgramVersionColor = GetConfigColor("SCREEN_PLAYLIST:PROGRAM_VERSION_COLOR");
			GetConfigPair("SCREEN_PLAYLIST:STREAM_OPENING_XY", 
									&m_ScreenConfig.StreamOpeningX, &m_ScreenConfig.StreamOpeningY);
			GetConfigPair("SCREEN_PLAYLIST:STREAM_OPENING_ERROR_XY", 
									&m_ScreenConfig.StreamOpeningErrorX, &m_ScreenConfig.StreamOpeningErrorY);
			GetConfigPair("SCREEN_PLAYLIST:STREAM_OPENING_SUCCESS_XY", 
									&m_ScreenConfig.StreamOpeningSuccessX, &m_ScreenConfig.StreamOpeningSuccessY);
			m_ScreenConfig.StreamOpeningColor = GetConfigColor("SCREEN_PLAYLIST:STREAM_OPENING_COLOR");
			m_ScreenConfig.StreamOpeningErrorColor = GetConfigColor("SCREEN_PLAYLIST:STREAM_OPENING_ERROR_COLOR");
			m_ScreenConfig.StreamOpeningSuccessColor = GetConfigColor("SCREEN_PLAYLIST:STREAM_OPENING_SUCCESS_COLOR");
			GetConfigPair("SCREEN_PLAYLIST:CLEAN_ON_NEW_STREAM_Y_RANGE", 
									&m_ScreenConfig.CleanOnNewStreamRangeY1, &m_ScreenConfig.CleanOnNewStreamRangeY2);
			GetConfigPair("SCREEN_PLAYLIST:ACTIVE_COMMAND_XY", 
									&m_ScreenConfig.ActiveCommandX, &m_ScreenConfig.ActiveCommandY);
			GetConfigPair("SCREEN_PLAYLIST:ERROR_MESSAGE_XY", 
									&m_ScreenConfig.ErrorMessageX, &m_ScreenConfig.ErrorMessageY);
			m_ScreenConfig.ActiveCommandColor = GetConfigColor("SCREEN_PLAYLIST:ACTIVE_COMMAND_COLOR");
			m_ScreenConfig.ErrorMessageColor = GetConfigColor("SCREEN_PLAYLIST:ERROR_MESSAGE_COLOR");
			GetConfigPair("SCREEN_PLAYLIST:NETWORK_ENABLING_XY", 
									&m_ScreenConfig.NetworkEnablingX, &m_ScreenConfig.NetworkEnablingY);
			GetConfigPair("SCREEN_PLAYLIST:NETWORK_DISABLING_XY", 
									&m_ScreenConfig.NetworkDisablingX, &m_ScreenConfig.NetworkDisablingY);
			GetConfigPair("SCREEN_PLAYLIST:NETWORK_READY_XY", 
									&m_ScreenConfig.NetworkReadyX, &m_ScreenConfig.NetworkReadyY);
			m_ScreenConfig.NetworkEnablingColor = GetConfigColor("SCREEN_PLAYLIST:NETWORK_ENABLING_COLOR");
			m_ScreenConfig.NetworkDisablingColor = GetConfigColor("SCREEN_PLAYLIST:NETWORK_DISABLING_COLOR");
			m_ScreenConfig.NetworkReadyColor = GetConfigColor("SCREEN_PLAYLIST:NETWORK_READY_COLOR");
			GetConfigPair("SCREEN_PLAYLIST:CLOCK_XY", 
									&m_ScreenConfig.ClockX, &m_ScreenConfig.ClockY);
			m_ScreenConfig.ClockColor = GetConfigColor("SCREEN_PLAYLIST:CLOCK_COLOR");
			GetConfigPair("SCREEN_PLAYLIST:BATTERY_XY", 
									&m_ScreenConfig.BatteryX, &m_ScreenConfig.BatteryY);
			m_ScreenConfig.BatteryColor = GetConfigColor("SCREEN_PLAYLIST:BATTERY_COLOR");
			break;
			
		case CScreenHandler::PSPRADIO_SCREEN_OPTIONS:
			m_ScreenConfig.FontMode   = (CScreen::textmode)m_Config->GetInteger("SCREEN_SHOUTCAST:FONT_MODE", 0);
			m_ScreenConfig.FontWidth  = m_Config->GetInteger("SCREEN_OPTIONS:FONT_WIDTH", 7);
			m_ScreenConfig.FontHeight = m_Config->GetInteger("SCREEN_OPTIONS:FONT_HEIGHT", 8);
			m_ScreenConfig.strBackground = m_Config->GetString("SCREEN_OPTIONS:BACKGROUND", NULL);
			m_ScreenConfig.BgColor = GetConfigColor("SCREEN_OPTIONS:BG_COLOR");
			m_ScreenConfig.FgColor = GetConfigColor("SCREEN_OPTIONS:FG_COLOR");
			GetConfigPair("SCREEN_OPTIONS:BUFFER_PERCENTAGE_XY", 
									&m_ScreenConfig.BufferPercentageX, &m_ScreenConfig.BufferPercentageY);
			m_ScreenConfig.BufferPercentageColor = GetConfigColor("SCREEN_OPTIONS:BUFFER_PERCENTAGE_COLOR");
			m_ScreenConfig.MetadataX1 = m_Config->GetInteger("SCREEN_OPTIONS:METADATA_X", 7);
			GetConfigPair("SCREEN_OPTIONS:METADATA_Y_RANGE", 
									&m_ScreenConfig.MetadataRangeY1, &m_ScreenConfig.MetadataRangeY2);
			GetConfigPair("SCREEN_OPTIONS:PROGRAM_VERSION_XY", 
									&m_ScreenConfig.ProgramVersionX, &m_ScreenConfig.ProgramVersionY);
			m_ScreenConfig.ProgramVersionColor = GetConfigColor("SCREEN_OPTIONS:PROGRAM_VERSION_COLOR");
			GetConfigPair("SCREEN_OPTIONS:STREAM_OPENING_XY", 
									&m_ScreenConfig.StreamOpeningX, &m_ScreenConfig.StreamOpeningY);
			GetConfigPair("SCREEN_OPTIONS:STREAM_OPENING_ERROR_XY", 
									&m_ScreenConfig.StreamOpeningErrorX, &m_ScreenConfig.StreamOpeningErrorY);
			GetConfigPair("SCREEN_OPTIONS:STREAM_OPENING_SUCCESS_XY", 
									&m_ScreenConfig.StreamOpeningSuccessX, &m_ScreenConfig.StreamOpeningSuccessY);
			m_ScreenConfig.StreamOpeningColor = GetConfigColor("SCREEN_OPTIONS:STREAM_OPENING_COLOR");
			m_ScreenConfig.StreamOpeningErrorColor = GetConfigColor("SCREEN_OPTIONS:STREAM_OPENING_ERROR_COLOR");
			m_ScreenConfig.StreamOpeningSuccessColor = GetConfigColor("SCREEN_OPTIONS:STREAM_OPENING_SUCCESS_COLOR");
			GetConfigPair("SCREEN_OPTIONS:CLEAN_ON_NEW_STREAM_Y_RANGE", 
									&m_ScreenConfig.CleanOnNewStreamRangeY1, &m_ScreenConfig.CleanOnNewStreamRangeY2);
			GetConfigPair("SCREEN_OPTIONS:ACTIVE_COMMAND_XY", 
									&m_ScreenConfig.ActiveCommandX, &m_ScreenConfig.ActiveCommandY);
			GetConfigPair("SCREEN_OPTIONS:ERROR_MESSAGE_XY", 
									&m_ScreenConfig.ErrorMessageX, &m_ScreenConfig.ErrorMessageY);
			m_ScreenConfig.ActiveCommandColor = GetConfigColor("SCREEN_OPTIONS:ACTIVE_COMMAND_COLOR");
			m_ScreenConfig.ErrorMessageColor = GetConfigColor("SCREEN_OPTIONS:ERROR_MESSAGE_COLOR");
			GetConfigPair("SCREEN_OPTIONS:NETWORK_ENABLING_XY", 
									&m_ScreenConfig.NetworkEnablingX, &m_ScreenConfig.NetworkEnablingY);
			GetConfigPair("SCREEN_OPTIONS:NETWORK_DISABLING_XY", 
									&m_ScreenConfig.NetworkDisablingX, &m_ScreenConfig.NetworkDisablingY);
			GetConfigPair("SCREEN_OPTIONS:NETWORK_READY_XY", 
									&m_ScreenConfig.NetworkReadyX, &m_ScreenConfig.NetworkReadyY);
			m_ScreenConfig.NetworkEnablingColor = GetConfigColor("SCREEN_OPTIONS:NETWORK_ENABLING_COLOR");
			m_ScreenConfig.NetworkDisablingColor = GetConfigColor("SCREEN_OPTIONS:NETWORK_DISABLING_COLOR");
			m_ScreenConfig.NetworkReadyColor = GetConfigColor("SCREEN_OPTIONS:NETWORK_READY_COLOR");
			GetConfigPair("SCREEN_OPTIONS:CLOCK_XY", 
									&m_ScreenConfig.ClockX, &m_ScreenConfig.ClockY);
			m_ScreenConfig.ClockColor = GetConfigColor("SCREEN_OPTIONS:CLOCK_COLOR");
			GetConfigPair("SCREEN_OPTIONS:BATTERY_XY", 
									&m_ScreenConfig.BatteryX, &m_ScreenConfig.BatteryY);
			m_ScreenConfig.BatteryColor = GetConfigColor("SCREEN_OPTIONS:BATTERY_COLOR");
			break;
	}
}

void CTextUI::Initialize_Screen(CScreenHandler::Screen screen)
{
	m_CurrentScreen = screen;

	LoadConfigSettings(screen);
	m_Screen->SetTextMode(m_ScreenConfig.FontMode);
	m_Screen->SetFontSize(m_ScreenConfig.FontWidth, m_ScreenConfig.FontHeight);
	m_Screen->SetBackColor(m_ScreenConfig.BgColor);
	m_Screen->SetTextColor(m_ScreenConfig.FgColor);
	if (m_ScreenConfig.strBackground)
	{
		m_Screen->SetBackgroundImage(m_ScreenConfig.strBackground);
	}
	m_Screen->Clear(); 
	if (pPSPApp->GetProgramVersion())
	{
		uiPrintf(m_ScreenConfig.ProgramVersionX, m_ScreenConfig.ProgramVersionY, m_ScreenConfig.ProgramVersionColor, pPSPApp->GetProgramVersion());
	}
	OnBatteryChange(m_LastBatteryPercentage);
	OnTimeChange(&m_LastLocalTime);
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
	
	int x=-1,y=m_Config->GetInteger("SCREEN_OPTIONS:FIRST_ENTRY_Y",40),c=0xFFFFFF;
	
	if (OptionsList.size() > 0)
	{
		for (OptionIterator = OptionsList.begin() ; OptionIterator != OptionsList.end() ; OptionIterator++)
		{
			if (OptionIterator == CurrentOptionIterator)
			{
				c = GetConfigColor("SCREEN_OPTIONS:COLOR_OPTION_NAME_TEXT");//0xFFFFFF;
			}
			else
			{
				c = GetConfigColor("SCREEN_OPTIONS:COLOR_OPTION_SELECTED_NAME_TEXT");//0x888888;
			}
			
			Option = (*OptionIterator);
			
			//ClearRows(y);
			PrintOption(x,y,c, Option.strName, Option.strStates, Option.iNumberOfStates, Option.iSelectedState, 
						Option.iActiveState);
			
			y+=m_Config->GetInteger("SCREEN_OPTIONS:Y_INCREMENT",16); //was 2*
		}
	}
}

void CTextUI::PrintOption(int x, int y, int c, char *strName, char *strStates[], int iNumberOfStates, int iSelectedState,
						  int iActiveState)
{
	int iTextPos = PIXEL_TO_COL(m_Config->GetInteger("SCREEN_OPTIONS:FIRST_ENTRY_X",40));
	int color = 0xFFFFFF;
	
	uiPrintf(COL_TO_PIXEL(iTextPos), y, c, "%s: ", strName);
	if (iNumberOfStates > 0)
	{
		iTextPos += strlen(strName)+2;
		for (int iStates = 0; iStates < iNumberOfStates ; iStates++)
		{
			if (iStates+1 == iActiveState)
			{
				color = GetConfigColor("SCREEN_OPTIONS:COLOR_ACTIVE_STATE");//0x0000FF;
			}
			else if (iStates+1 == iSelectedState) /** 1-based */
			{
				color = GetConfigColor("SCREEN_OPTIONS:COLOR_SELECTED_STATE");//0xFFFFFF;
			}
			else
			{
				color = GetConfigColor("SCREEN_OPTIONS:COLOR_NOT_SELECTED_STATE");//0x888888;
			}
			
			if ((iStates+1 == iActiveState) && (iStates+1 == iSelectedState))
			{
				color =  GetConfigColor("SCREEN_OPTIONS:COLOR_ACTIVE_AND_SELECTED_STATE");//0x9090E3;
			}
			
			uiPrintf(COL_TO_PIXEL(iTextPos),y,color, "%s ", strStates[iStates]);
			iTextPos += strlen(strStates[iStates])+1;
		}
	}	
}

void CTextUI::uiPrintf(int x, int y, int color, char *strFormat, ...)
{
	va_list args;
	char msg[70*5/** 5 lines worth of text...*/];

	/** -2  = Don't Print */
	if (x != -2)
	{
		va_start (args, strFormat);         /* Initialize the argument list. */
		
		vsprintf(msg, strFormat, args);
	
		if (msg[strlen(msg)-1] == 0x0A)
			msg[strlen(msg)-1] = 0; /** Remove LF 0D*/
		if (msg[strlen(msg)-1] == 0x0D) 
			msg[strlen(msg)-1] = 0; /** Remove CR 0A*/
	
		if (x == -1) /** CENTER */
		{
			x = PSP_SCREEN_WIDTH/2 - ((strlen(msg)/2)*m_Screen->GetFontWidth());
		}
		m_Screen->PrintText(x, y, color, msg);
		
		va_end (args);                  /* Clean up. */
	}
}

void CTextUI::ClearRows(int iRowStart, int iRowEnd)
{
	if (iRowEnd == -1)
		iRowEnd = iRowStart;
		
	m_lockclear->Lock();
	for (int iRow = iRowStart ; (iRow < PSP_SCREEN_HEIGHT) && (iRow <= iRowEnd); iRow+=m_Screen->GetFontHeight())
	{
		//m_Screen->ClearLine(PIXEL_TO_ROW(iRow));///m_Screen->GetFontHeight());
		m_Screen->ClearCharsAtYFromX1ToX2(iRow, 0, PSP_SCREEN_WIDTH);
	}
	m_lockclear->Unlock();
}

void CTextUI::ClearHalfRows(int iColStart, int iColEnd, int iRowStart, int iRowEnd)
{
	if (iRowEnd == -1)
		iRowEnd = iRowStart;
		
	m_lockclear->Lock();
	for (int iRow = iRowStart ; (iRow < PSP_SCREEN_HEIGHT) && (iRow <= iRowEnd); iRow+=m_Screen->GetFontHeight())
	{
		m_Screen->ClearCharsAtYFromX1ToX2(iRow, iColStart, iColEnd);
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

void CTextUI::OnBatteryChange(int Percentage)
{
	uiPrintf(m_ScreenConfig.BatteryX, m_ScreenConfig.BatteryY, m_ScreenConfig.BatteryColor, "B:%03d%%", Percentage);
	m_LastBatteryPercentage = Percentage;
}

void CTextUI::OnTimeChange(pspTime *LocalTime)
{
	uiPrintf(m_ScreenConfig.ClockX, m_ScreenConfig.ClockY, m_ScreenConfig.ClockColor, "%02d:%02d", 
				LocalTime->hour, LocalTime->minutes);
	m_LastLocalTime = *LocalTime;
}

int CTextUI::DisplayMessage_EnablingNetwork()
{
	//int x,y,c;
	//GetConfigPair("TEXT_POS:NETWORK_ENABLING", &x, &y);
	//c = GetConfigColor("COLORS:NETWORK_ENABLING");
	
	//ClearErrorMessage();
	ClearRows(m_ScreenConfig.NetworkEnablingY);
	uiPrintf(m_ScreenConfig.NetworkEnablingX, m_ScreenConfig.NetworkEnablingY, m_ScreenConfig.NetworkEnablingColor, "Enabling Network");
	
	return 0;
}

int CTextUI::DisplayMessage_DisablingNetwork()
{
	//int x,y,c;
	//GetConfigPair("TEXT_POS:NETWORK_DISABLING", &x, &y);
	//c = GetConfigColor("COLORS:NETWORK_DISABLING");
	
	ClearRows(m_ScreenConfig.NetworkDisablingY);
	uiPrintf(m_ScreenConfig.NetworkDisablingX, m_ScreenConfig.NetworkDisablingY, m_ScreenConfig.NetworkDisablingColor, "Disabling Network");
	
	return 0;
}

int CTextUI::DisplayMessage_NetworkReady(char *strIP)
{
	//int x,y,c;
	//GetConfigPair("TEXT_POS:NETWORK_READY", &x, &y);
	//c = GetConfigColor("COLORS:NETWORK_READY");
	
	ClearRows(m_ScreenConfig.NetworkReadyY);
	uiPrintf(m_ScreenConfig.NetworkReadyX, m_ScreenConfig.NetworkReadyY, m_ScreenConfig.NetworkReadyColor, "Ready, IP %s", strIP);
	
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
	//int x,y,c;
	//GetConfigPair("TEXT_POS:ACTIVE_COMMAND", &x, &y);
	//c = GetConfigColor("COLORS:ACTIVE_COMMAND");
	
	ClearRows(m_ScreenConfig.ActiveCommandY);
	switch(playingstate)
	{
	case CPSPSound::STOP:
		{
			uiPrintf(m_ScreenConfig.ActiveCommandX, m_ScreenConfig.ActiveCommandY, m_ScreenConfig.ActiveCommandColor, "STOP");
			//int r1,r2;
			//GetConfigPair("TEXT_POS:METADATA_ROW_RANGE", &r1, &r2);
			ClearRows(m_ScreenConfig.MetadataRangeY1, m_ScreenConfig.MetadataRangeY2);
			//int px,py;
			//GetConfigPair("TEXT_POS:BUFFER_PERCENTAGE", &px, &py);
			ClearRows(m_ScreenConfig.BufferPercentageY);
			break;
		}
	case CPSPSound::PLAY:
		uiPrintf(m_ScreenConfig.ActiveCommandX, m_ScreenConfig.ActiveCommandY, m_ScreenConfig.ActiveCommandColor, "PLAY");
		break;
	case CPSPSound::PAUSE:
		uiPrintf(m_ScreenConfig.ActiveCommandX, m_ScreenConfig.ActiveCommandY, m_ScreenConfig.ActiveCommandColor, "PAUSE");
		break;
	}
	
	return 0;
}

int CTextUI::DisplayErrorMessage(char *strMsg)
{
	int x,y,c;
	//c = GetConfigColor("COLORS:ERROR_MESSAGE");
	x = m_ScreenConfig.ErrorMessageX;
	y = m_ScreenConfig.ErrorMessageY;
	c = m_ScreenConfig.ErrorMessageColor;
	
	#if 0
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
	#endif
	ClearErrorMessage();
	/** If message is longer than 1 lines, then truncate;
	The -10 is to accomodate for the "Error: " plus a bit.
	*/
	if (strlen(strMsg)>(size_t)(PIXEL_TO_COL(PSP_SCREEN_WIDTH) - 10))
	{
		strMsg[(PIXEL_TO_COL(PSP_SCREEN_WIDTH) - 10)] = 0;
	}
	uiPrintf(x, y, c, "Error: %s", strMsg);
	
	return 0;
}

int CTextUI::DisplayMessage(char *strMsg)
{
	int x,y,c;
	//c = GetConfigColor("COLORS:ERROR_MESSAGE");
	x = m_ScreenConfig.ErrorMessageX;
	y = m_ScreenConfig.ErrorMessageY;
	c = m_ScreenConfig.ErrorMessageColor;
	#if 0
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
	#endif
	ClearErrorMessage();
	/** If message is longer than 1 lines, then truncate;
	 *  The -3 is just in case.
	 */
	if (strlen(strMsg)>(size_t)(PIXEL_TO_COL(PSP_SCREEN_WIDTH) - 3))
	{
		strMsg[(PIXEL_TO_COL(PSP_SCREEN_WIDTH) - 3)] = 0;
	}
	uiPrintf(-1, y, c, "%s", strMsg);
	
	return 0;
}

int CTextUI::ClearErrorMessage()
{
	//int x,y;
	//GetConfigPair("TEXT_POS:ERROR_MESSAGE_ROW_RANGE", &x, &y);
	//ClearRows(x, y);
	ClearRows(m_ScreenConfig.ErrorMessageY);
	return 0;
}

int CTextUI::DisplayBufferPercentage(int iPerc)
{
	//int c;

	if (CScreenHandler::PSPRADIO_SCREEN_OPTIONS != m_CurrentScreen)
	{
		//GetConfigPair("TEXT_POS:BUFFER_PERCENTAGE", &x, &y);
		//c = GetConfigColor("COLORS:BUFFER_PERCENTAGE");
	
		if (iPerc >= 95)
			iPerc = 100;
		if (iPerc < 2)
			iPerc = 0;

		//uiPrintf(x, y, c, "Buffer: %03d%c%c", iPerc, 37, 37/* 37='%'*/);
		uiPrintf(m_ScreenConfig.BufferPercentageX, m_ScreenConfig.BufferPercentageY, m_ScreenConfig.BufferPercentageColor, "Buffer: %03d%c", iPerc, 37/* 37='%'*/);
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
	//GetConfigPair("TEXT_POS:STREAM_OPENING", &x, &y);
	//c = GetConfigColor("COLORS:STREAM_OPENING");
	x = m_ScreenConfig.StreamOpeningX;
	y = m_ScreenConfig.StreamOpeningY;
	c = m_ScreenConfig.StreamOpeningColor;
	
	ClearErrorMessage(); /** Clear any errors */
	ClearRows(y);
	uiPrintf(x, y, c, "Opening Stream");
	return 0;
}

int CTextUI::OnConnectionProgress()
{
	int x,y,c;
	//GetConfigPair("TEXT_POS:STREAM_OPENING", &x, &y);
	//c = GetConfigColor("COLORS:STREAM_OPENING");
	char *strIndicator[] = {"OpEnInG StReAm", "oPeNiNg sTrEaM"};
	static int sIndex = 0;
	x = m_ScreenConfig.StreamOpeningX;
	y = m_ScreenConfig.StreamOpeningY;
	c = m_ScreenConfig.StreamOpeningColor;

	uiPrintf(x, y, c, "%s", strIndicator[sIndex]);
	
	sIndex = (sIndex+1)%2;

	return 0;
}

int CTextUI::OnStreamOpeningError()
{
	int x,y,c;
	//GetConfigPair("TEXT_POS:STREAM_OPENING_ERROR", &x, &y);
	//c = GetConfigColor("COLORS:STREAM_OPENING_ERROR");
	x = m_ScreenConfig.StreamOpeningErrorX;
	y = m_ScreenConfig.StreamOpeningErrorY;
	c = m_ScreenConfig.StreamOpeningErrorColor;
	
	ClearRows(y);
	uiPrintf(x, y, c, "Error Opening Stream");
	return 0;
}

int CTextUI::OnStreamOpeningSuccess()
{
	int x,y,c;
	//GetConfigPair("TEXT_POS:STREAM_OPENING_SUCCESS", &x, &y);
	//c = GetConfigColor("COLORS:STREAM_OPENING_SUCCESS");
	x = m_ScreenConfig.StreamOpeningSuccessX;
	y = m_ScreenConfig.StreamOpeningSuccessY;
	c = m_ScreenConfig.StreamOpeningSuccessColor;
	
	ClearRows(y);
	//uiPrintf(x, y, c, "Stream Opened");
	return 0;
}

int CTextUI::OnNewSongData(MetaData *pData)
{
	//int r1,r2,x1;

	if (CScreenHandler::PSPRADIO_SCREEN_OPTIONS != m_CurrentScreen)
	{

		//GetConfigPair("TEXT_POS:METADATA_ROW_RANGE", &r1, &r2);
		//x1 = m_Config->GetInteger("TEXT_POS:METADATA_START_COLUMN", 0);
		int y = m_ScreenConfig.MetadataRangeY1;
		int x = m_ScreenConfig.MetadataX1;
		ClearRows(y, m_ScreenConfig.MetadataRangeY2);
		
		if (strlen(pData->strTitle) >= (size_t)(m_Screen->GetNumberOfTextColumns()-10-PIXEL_TO_COL(x)))
			pData->strTitle[m_Screen->GetNumberOfTextColumns()-10-PIXEL_TO_COL(x)] = 0;
			
		if (strlen(pData->strURL) >= (size_t)(m_Screen->GetNumberOfTextColumns()-10-PIXEL_TO_COL(x)))
			pData->strURL[m_Screen->GetNumberOfTextColumns()-10-PIXEL_TO_COL(x)] = 0;
		
		if (0 != pData->iSampleRate)
		{
			uiPrintf(x, y, COLOR_WHITE, "%lukbps %dHz (%d channels) stream",
					pData->iBitRate/1000, 
					pData->iSampleRate,
					pData->iNumberOfChannels);
					//pData->strMPEGLayer);
			y+=m_Screen->GetFontHeight();
		}
		if (pData->strURL && strlen(pData->strURL))
		{
			uiPrintf(x, y, COLOR_WHITE,	"URL   : ");
			uiPrintf(x+COL_TO_PIXEL(8), y, COLOR_CYAN,	"%s ", pData->strURL);
		}
		else
		{
			uiPrintf(x , y,	COLOR_WHITE,	"Stream: ");
			uiPrintf(x+COL_TO_PIXEL(8), y,	COLOR_CYAN,		"%s ", pData->strURI);
		}
		y+=m_Screen->GetFontHeight();
		
		uiPrintf(x , y,	COLOR_WHITE,	"Title : ");
		uiPrintf(x+COL_TO_PIXEL(8), y,	COLOR_CYAN, 	"%s ", pData->strTitle);
		y+=m_Screen->GetFontHeight();
		
		if (pData->strArtist && strlen(pData->strArtist))
		{
			uiPrintf(x , y,	COLOR_WHITE,	"Artist: ");
			uiPrintf(x+COL_TO_PIXEL(8), y,	COLOR_CYAN, 	"%s ", pData->strArtist);
		}
	}
	return 0;
}

void CTextUI::DisplayContainers(CMetaDataContainer *Container)
{
	int iColorNormal, iColorSelected, iColorTitle, iColor;
	int iNextRow = 0;
	char *strText = NULL;

	map< string, list<MetaData>* >::iterator ListIterator;
	map< string, list<MetaData>* >::iterator *CurrentElement = Container->GetCurrentContainerIterator();
	map< string, list<MetaData>* > *List = Container->GetContainerList();
	iColorNormal   = m_ScreenConfig.EntriesListColor;
	iColorTitle    = m_ScreenConfig.ListsTitleColor;
	iColorSelected = m_ScreenConfig.SelectedEntryColor;
	iColor = iColorNormal;
	
	bool bShowFileExtension = m_Config->GetInteger("GENERAL:SHOW_FILE_EXTENSION", 0);

	ClearHalfRows(m_ScreenConfig.ContainerListRangeX1, m_ScreenConfig.ContainerListRangeX2, m_ScreenConfig.ContainerListRangeY1+ROW_TO_PIXEL(1), m_ScreenConfig.ContainerListRangeY2); /** Don't clear title (+1) */

	iNextRow = m_ScreenConfig.ContainerListRangeY1 + ROW_TO_PIXEL(1); /** Start after the title */
	
	strText = (char *)malloc (MAXPATHLEN);
	
	//Log(LOG_VERYLOW, "DisplayContainers(): populating screen");
	if (List->size() > 0)
	{
		//Log(LOG_VERYLOW, "DisplayContainers(): Setting iterator to middle of the screen");
		ListIterator = *CurrentElement;
		for (int i = 0; i < PIXEL_TO_ROW(m_ScreenConfig.ContainerListRangeY2-m_ScreenConfig.ContainerListRangeY1)/2; i++)
		{
			if (ListIterator == List->begin())
				break;
			ListIterator--;
		
		}

		//Log(LOG_VERYLOW, "DisplayPLEntries(): elements: %d", List->size());
		//Log(LOG_VERYLOW, "DisplayContainers(): Populating Screen (total elements %d)", List->size());
		for (; ListIterator != List->end() ; ListIterator++)
		{
			if (iNextRow > m_ScreenConfig.ContainerListRangeY2)
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
				if (false == bShowFileExtension)
				{
					char *ext = strrchr(pText, '.');
					if(ext)
					{
						ext[0] = 0;
					}
				}
				pText[PIXEL_TO_COL(m_ScreenConfig.ContainerListRangeX2-m_ScreenConfig.ContainerListRangeX1)] = 0;
				uiPrintf(m_ScreenConfig.ContainerListRangeX1, iNextRow, iColor, pText);
			}
			else
			{
				strText[PIXEL_TO_COL(m_ScreenConfig.ContainerListRangeX2-m_ScreenConfig.ContainerListRangeX1)] = 0;
				uiPrintf(m_ScreenConfig.ContainerListRangeX1, iNextRow, iColor, strText);
			}
			iNextRow+=m_Screen->GetFontHeight();
		}
	}
	
	free(strText), strText = NULL;
}

void CTextUI::DisplayElements(CMetaDataContainer *Container)
{
	int iNextRow;
	int iColorNormal,iColorTitle,iColorSelected, iColor;
	char *strText = NULL;
	
	list<MetaData>::iterator ListIterator;
	list<MetaData>::iterator *CurrentElement = Container->GetCurrentElementIterator();
	list<MetaData> *List = Container->GetElementList();
	iColorNormal = m_ScreenConfig.EntriesListColor;
	iColorSelected = m_ScreenConfig.SelectedEntryColor;
	iColorTitle = m_ScreenConfig.ListsTitleColor;
	iColor = iColorNormal;

	bool bShowFileExtension = m_Config->GetInteger("GENERAL:SHOW_FILE_EXTENSION", 0);
	
	ClearHalfRows(m_ScreenConfig.EntriesListRangeX1,m_ScreenConfig.EntriesListRangeX2,
				  m_ScreenConfig.EntriesListRangeY1+ROW_TO_PIXEL(1),m_ScreenConfig.EntriesListRangeY2); /** Don't clear Entry title */
	
	//uiPrintf(33/2 + x - 3/*entry/2*/, y, ct, "Entry");
	iNextRow = m_ScreenConfig.EntriesListRangeY1 + ROW_TO_PIXEL(1);
	
	strText = (char *)malloc (MAXPATHLEN);
	
	//Log(LOG_VERYLOW, "DisplayPLEntries(): populating screen");
	if (List->size() > 0)
	{
		ListIterator = *CurrentElement;
		for (int i = 0; i < PIXEL_TO_ROW(m_ScreenConfig.EntriesListRangeY2-m_ScreenConfig.EntriesListRangeY1)/2; i++)
		{
			if (ListIterator == List->begin())
				break;
			ListIterator--;
		
		}
		//Log(LOG_VERYLOW, "DisplayPLEntries(): elements: %d", List->size());
		for (; ListIterator != List->end() ; ListIterator++)
		{
			if (iNextRow > m_ScreenConfig.EntriesListRangeY2)
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
			
			
			char *pText = strText;
			if (strlen((*ListIterator).strTitle))
			{
				//Log(LOG_VERYLOW, "DisplayPLEntries(): Using strTitle='%s'", (*ListIterator).strTitle);
				strncpy(strText, (*ListIterator).strTitle, PIXEL_TO_COL(m_ScreenConfig.EntriesListRangeX2-m_ScreenConfig.EntriesListRangeX1));
				strText[PIXEL_TO_COL(m_ScreenConfig.EntriesListRangeX2-m_ScreenConfig.EntriesListRangeX1)] = 0;
			}
			else
			{
				strncpy(strText, (*ListIterator).strURI, PIXEL_TO_COL(m_ScreenConfig.EntriesListRangeX2-m_ScreenConfig.EntriesListRangeX1));
				strText[PIXEL_TO_COL(m_ScreenConfig.EntriesListRangeX2-m_ScreenConfig.EntriesListRangeX1)] = 0;
				
				if (strlen(strText) > 4 && memcmp(strText, "ms0:", 4) == 0)
				{
					pText = basename(strText);
					if (false == bShowFileExtension)
					{
						char *ext = strrchr(pText, '.');
						if(ext)
						{
							ext[0] = 0;
						}
					}
				}
			}
		
			uiPrintf(m_ScreenConfig.EntriesListRangeX1, iNextRow, iColor, pText);
			iNextRow+=m_Screen->GetFontHeight();
		}
	}
	
	free(strText), strText = NULL;
}

void CTextUI::OnCurrentContainerSideChange(CMetaDataContainer *Container)
{
	int iColorTitle = m_ScreenConfig.ListsTitleColor;

	ClearRows(m_ScreenConfig.ContainerListRangeY1);
	if (m_ScreenConfig.ContainerListRangeY1 != m_ScreenConfig.EntriesListRangeY1)
		ClearRows(m_ScreenConfig.EntriesListRangeY1);
	
	
	switch (Container->GetCurrentSide())
	{
		case CMetaDataContainer::CONTAINER_SIDE_CONTAINERS:
			uiPrintf((m_ScreenConfig.ContainerListRangeX2 - m_ScreenConfig.ContainerListRangeX1)/2 + m_ScreenConfig.ContainerListRangeX1 - COL_TO_PIXEL(4)/*entry/2*/,  m_ScreenConfig.ContainerListRangeY1, iColorTitle, "*List*");
			uiPrintf((m_ScreenConfig.EntriesListRangeX2 - m_ScreenConfig.EntriesListRangeX1)/2 + m_ScreenConfig.EntriesListRangeX1 - COL_TO_PIXEL(4)/*entry/2*/, m_ScreenConfig.EntriesListRangeY1, iColorTitle, "Entries");
			break;
		
		case CMetaDataContainer::CONTAINER_SIDE_ELEMENTS:
			uiPrintf((m_ScreenConfig.ContainerListRangeX2 - m_ScreenConfig.ContainerListRangeX1)/2 + m_ScreenConfig.ContainerListRangeX1 - COL_TO_PIXEL(3)/*entry/2*/,  m_ScreenConfig.ContainerListRangeY1, iColorTitle, "List");
			uiPrintf((m_ScreenConfig.EntriesListRangeX2 - m_ScreenConfig.EntriesListRangeX1)/2 + m_ScreenConfig.EntriesListRangeX1 - COL_TO_PIXEL(5)/*entry/2*/, m_ScreenConfig.EntriesListRangeY1, iColorTitle, "*Entries*");
			break;
	
	}
}
