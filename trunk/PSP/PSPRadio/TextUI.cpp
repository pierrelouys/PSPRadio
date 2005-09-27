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

CTextUI::CTextUI()
{
}

CTextUI::~CTextUI()
{
}

int CTextUI::Initialize()
{
	m_lockprint = new CLock("Print_Lock");
	m_lockclear = new CLock("Clear_Lock");
	pspDebugScreenInit();
	pspDebugScreenSetBackColor(COLOR_BLUE);
	pspDebugScreenSetTextColor(0xFFFFFFFF);
	pspDebugScreenClear(); 
	
	return 0;
}

void CTextUI::Terminate()
{
	delete(m_lockprint);
	m_lockprint = NULL;
	delete(m_lockclear);
	m_lockclear = NULL;
}


void CTextUI::uiPrintf(int x, int y, uicolors color, char *strFormat, ...)
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

	pspDebugScreenSetXY(x,y);
	pspDebugScreenSetTextColor(color);
	printf(msg);
	
	va_end (args);                  /* Clean up. */

	m_lockprint->Unlock();
}

void CTextUI::ClearRows(int iRowStart, int iNumRows)
{
	m_lockclear->Lock();
	for (int iRow = 10 ; (iRow < MAX_ROWS) && (iRow < iNumRows+iRowStart); iRow++)
	{
		pspDebugScreenSetXY(0,iRow + 1);
		printf("% 68c", ' ');
	}
	m_lockclear->Unlock();
}

int CTextUI::SetTitle(char *strTitle)
{

	uiPrintf(0,0, COLOR_WHITE, strTitle);
	
	return 0;
}

int CTextUI::DisplayMessage_EnablingNetwork()
{
	uiPrintf(10, 26, COLOR_YELLOW, "Enabling Network");
	
	return 0;
}

int CTextUI::DisplayMessage_DisablingNetwork()
{
	uiPrintf(10, 26, COLOR_RED, "Disabling Network");
	
	return 0;
}

int CTextUI::DisplayMessage_NetworkReady(char *strIP)
{
	uiPrintf(10, 26, COLOR_YELLOW, "Ready, IP %s", strIP);
	
	return 0;
}

int CTextUI::DisplayMainCommands()
{
	uiPrintf(8, 25, COLOR_GREEN, "O or X = Play/Pause | [] = Stop | ^ = Reconnect");
		
	return 0;
}

int CTextUI::DisplayActiveCommand(CPSPSound::pspsound_state playingstate)
{
	uicolors color = COLOR_YELLOW;
	switch(playingstate)
	{
	case CPSPSound::STOP:
		uiPrintf(30, 20, color, "STOP   ");
		break;
	case CPSPSound::PLAY:
		uiPrintf(30, 20, color, "PLAY   ");
		break;
	case CPSPSound::PAUSE:
		uiPrintf(30, 20, color, "PAUSE  ");
		break;
	}
	
	return 0;
}

int CTextUI::DisplayErrorMessage(char *strMsg)
{
	//printf("% 70c", ' ');
	uiPrintf(0, 30, COLOR_RED, "Error: %s", strMsg);
	
	return 0;
}

int CTextUI::DisplayPlayBuffer(int a, int b)
{
	ClearRows(11);
	uiPrintf(0, 11, COLOR_WHITE, "Out Buffer: %03d/%03d   ", a, b);
	return 0;
}

int CTextUI::DisplayDecodeBuffer(int a, int b)
{
	ClearRows(10);
	uiPrintf(0, 10, COLOR_WHITE, "In Buffer: %03d/%03d   ", a, b);
	return 0;
}

int CTextUI::OnNewStreamStarted()
{
	ClearRows(4, 6);
	return 0;
}

int CTextUI::OnStreamOpening(char *StreamName)
{
	ClearRows(18);
	uiPrintf(0, 18, COLOR_WHITE, "Stream: %s (Opening)", StreamName);
	return 0;
}

int CTextUI::OnStreamOpeningError(char *StreamName)
{
	ClearRows(18);
	uiPrintf(0, 18, COLOR_WHITE, "Stream: %s (Error Opening)", StreamName);
	return 0;
}

int CTextUI::OnStreamOpeningSuccess(char *StreamName)
{
	ClearRows(18);
	uiPrintf(0, 18, COLOR_WHITE, "Stream: %s (Open)", StreamName);
	return 0;
}

int CTextUI::DisplayMetadata(char *bMetadata)
{
	ClearRows(12,3);
	uiPrintf(0,12, COLOR_WHITE, "%s", bMetadata);
	return 0;
}

int CTextUI::DisplaySampleRateAndKBPS(int samplerate, int bitrate)
{
	uiPrintf(0,9, COLOR_WHITE, "%lukbps %dHz  ",
			bitrate, 
			samplerate);
	return 0;
}

int CTextUI::DisplayMPEGLayerType(char *strType)
{
	uiPrintf(20, 9, COLOR_WHITE, "MPEG layer %s stream   ", 
		strType);
	return 0;
}

int CTextUI::OnConnectionProgress()
{
	printf(".");
	return 0;
}
