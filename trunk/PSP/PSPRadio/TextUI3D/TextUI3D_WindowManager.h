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

#ifndef _TEXTUI3D_WINDOWMANAGER_
#define _TEXTUI3D_WINDOWMANAGER_

#include "TextUI3D_WindowManager_HSM.h"

using namespace std;

	enum user_events
	{
	WM_EVENT_NONE,
	/* Lists */
	WM_EVENT_PLAYLIST,
	WM_EVENT_SHOUTCAST,
	WM_EVENT_LOCALFILES,
	WM_EVENT_SELECT_LIST,
	WM_EVENT_SELECT_ENTRIES,
	WM_EVENT_LIST_TEXT,
	WM_EVENT_ENTRY_TEXT,
	WM_EVENT_LIST_CLEAR,
	WM_EVENT_ENTRY_CLEAR,

	/* Option events */
	WM_EVENT_OPTIONS,
	WM_EVENT_OPTIONS_TEXT,
	WM_EVENT_OPTIONS_CLEAR,

	/* Misc events. */
	WM_EVENT_VBLANK,
	WM_EVENT_TIME,
	WM_EVENT_BATTERY,
	WM_EVENT_BUFFER,
	WM_EVENT_BITRATE,
	WM_EVENT_USB_ENABLE,
	WM_EVENT_USB_DISABLE,

	/* Static text events */
	WM_EVENT_TEXT_ERROR,
	WM_EVENT_TEXT_MESSAGE,
	WM_EVENT_TEXT_SONGTITLE,

	/* Network events */
	WM_EVENT_NETWORK_ENABLE,
	WM_EVENT_NETWORK_DISABLE,
	WM_EVENT_NETWORK_IP,

	/* Player events */
	WM_EVENT_PLAYER_STOP,
	WM_EVENT_PLAYER_START,
	WM_EVENT_PLAYER_PAUSE,

	/* Stream events */
	WM_EVENT_STREAM_START,
	WM_EVENT_STREAM_OPEN,
	WM_EVENT_STREAM_CONNECTING,
	WM_EVENT_STREAM_ERROR,
	WM_EVENT_STREAM_SUCCESS,

	/* GU Events */
	WM_EVENT_GU_INIT,
	};


class CTextUI3D_WindowManager
{

public:
	CTextUI3D_WindowManager();
	~CTextUI3D_WindowManager();

	void WM_SendEvent(int event, void *data);
	void Initialize(char *cwd);
	void AddOptionText(int x, int y, unsigned int color, char *text);
	void AddListText(int x, int y, unsigned int color, char *text);
	void AddEntryText(int x, int y, unsigned int color, char *text);
	void AddTitleText(int x, int y, unsigned int color, char *text);

private:
	WindowHandlerHSM	m_wm_hsm;

};

#endif
