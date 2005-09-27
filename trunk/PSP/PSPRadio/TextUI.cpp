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
#include <Logging.h>
#include "TextUI.h"


CTextUI::~CTextUI()
{
}

int CTextUI::Initialize()
{
	pspDebugScreenInit();
	pspDebugScreenSetBackColor(0x00FF0000);
	pspDebugScreenSetTextColor(0xFFFFFFFF);
	pspDebugScreenClear(); 
	
	return 0;
}


int CTextUI::SetTitle(char *strTitle)
{
	uiprint(strTitle, 0, 0, COLOR_WHITE);
	
	return 0;
}

int CTextUI::uiprint(char *strText, int x, int y, uicolors color)
{
	//lock();
	pspDebugScreenSetXY(x,y);
	pspDebugScreenSetTextColor(color);
	printf(strText);
	//unlock();
	
	return 0;
}

int CTextUI::uiprint(char *strFmt, char *strArg, int x, int y, uicolors color)
{
	char strOut[140];
	sprintf(strOut, strFmt, strArg);
	uiprint(strOut, x, y, color);
	
	return 0;
}

int CTextUI::DisplayMessage_EnablingNetwork()
{
	uiprint("Enabling Network", 10, 26, COLOR_YELLOW);
	
	return 0;
}

int CTextUI::DisplayMessage_DisablingNetwork()
{
	pspDebugScreenSetXY(10,26);
	uiprint("Disabling Network", 10, 26, COLOR_RED);
	
	return 0;
}

int CTextUI::DisplayMessage_NetworkReady(char *strIP)
{
	uiprint("Ready, IP %s", strIP, 10, 26, COLOR_YELLOW);
	
	return 0;
}

int CTextUI::DisplayMainCommands()
{
	uiprint("O or X = Play/Pause | [] = Stop | ^ = Reconnect", 8, 25, COLOR_GREEN);
		
	return 0;
}

int CTextUI::DisplayActiveCommand(CPSPSound::pspsound_state playingstate)
{
	uicolors color = COLOR_YELLOW;
	switch(playingstate)
	{
	case CPSPSound::STOP:
		uiprint("STOP   ", 30, 20, color);
		break;
	case CPSPSound::PLAY:
		uiprint("PLAY   ", 30, 20, color);
		break;
	case CPSPSound::PAUSE:
		uiprint("PAUSE  ", 30, 20, color);
		break;
	}
	
	return 0;
}

int CTextUI::DisplayErrorMessage(char *strMsg)
{
	//printf("% 70c", ' ');
	uiprint("Error: %s", strMsg, 0, 30, COLOR_RED);
	
	return 0;
}

int CTextUI::DisplayPlayBuffer(int a, int b)
{
	static char str[70];
	sprintf(str, "Out Buffer: %03d/%03d   ", a, b);
	uiprint(str, 0, 11, COLOR_WHITE);
		
	return 0;
}
