/*
	PSPRadio / Music streaming client for the PSP. (Initial Release: Sept. 2005)
	PSPRadio Copyright (C) 2005 Rafael Cabezas a.k.a. Raf
	TextUI3D Copyright (C) 2005 Jesper Sandberg & Raf


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
#include <PSPSound.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <malloc.h>
#include <math.h>
#include <time.h>
#include <iniparser.h>
#include <Tools.h>
#include <stdarg.h>
#include <Logging.h>

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>

#include "TextUI3D.h"
#include "TextUI3D_WindowManager.h"
#include "TextUI3D_Panel.h"
#include "TextUI3D_WindowManager_HSM.h"

CTextUI3D_WindowManager::CTextUI3D_WindowManager()
{
	/* Initialize HSM */
}

CTextUI3D_WindowManager::~CTextUI3D_WindowManager()
{
}

void CTextUI3D_WindowManager::Initialize(char *cwd)
{
	m_wm_hsm.Initialize(cwd);
}

void CTextUI3D_WindowManager::AddOptionText(int x, int y, unsigned int color, char *text)
{
	WindowHandlerHSM::TextItem	*item;

	item = (WindowHandlerHSM::TextItem *) malloc(sizeof(WindowHandlerHSM::TextItem));
	item->x 	= x;
	item->y 	= y;
	item->color 	= color;
	strcpy(item->strText, text);
	strupr(item->strText);
	item->ID = 0;
//	Log(LOG_ERROR, "%d, %d, %08X, %s", x, y, color, text);
	m_wm_hsm.Dispatch(WM_EVENT_OPTIONS_TEXT, item);
}

void CTextUI3D_WindowManager::AddListText(int x, int y, unsigned int color, char *text)
{
	WindowHandlerHSM::TextItem	*item;

	item = (WindowHandlerHSM::TextItem *) malloc(sizeof(WindowHandlerHSM::TextItem));
	item->x 	= x;
	item->y 	= y;
	item->color 	= color;
	strcpy(item->strText, text);
	strupr(item->strText);
	item->ID = 0;
//	Log(LOG_ERROR, "%d, %d, %08X, %s", x, y, color, text);
	m_wm_hsm.Dispatch(WM_EVENT_LIST_TEXT, item);
}

void CTextUI3D_WindowManager::AddEntryText(int x, int y, unsigned int color, char *text)
{
	WindowHandlerHSM::TextItem	*item;

	item = (WindowHandlerHSM::TextItem *) malloc(sizeof(WindowHandlerHSM::TextItem));
	item->x 	= x;
	item->y 	= y;
	item->color 	= color;
	strcpy(item->strText, text);
	strupr(item->strText);
	item->ID = 0;
//	Log(LOG_ERROR, "%d, %d, %08X, %s", x, y, color, text);
	m_wm_hsm.Dispatch(WM_EVENT_ENTRY_TEXT, item);
}

void CTextUI3D_WindowManager::AddTitleText(int x, int y, unsigned int color, char *text)
{
	WindowHandlerHSM::TextItem	*item;

	item = (WindowHandlerHSM::TextItem *) malloc(sizeof(WindowHandlerHSM::TextItem));
	item->x 	= x;
	item->y 	= y;
	item->color 	= color;
	strcpy(item->strText, text);
	strupr(item->strText);
	item->ID = 0;
//	Log(LOG_ERROR, "%d, %d, %08X, %s", x, y, color, text);
	m_wm_hsm.Dispatch(WM_EVENT_TEXT_SONGTITLE, item);
}

/*
	HSM for the WindowManager.
	All events about changes in the UI are passed to this handler.
*/
void CTextUI3D_WindowManager::WM_SendEvent(int event, void *data)
{
	m_wm_hsm.Dispatch(event, data);
}
