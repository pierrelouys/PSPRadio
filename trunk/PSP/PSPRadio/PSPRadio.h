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
#ifndef _PSPRADIO_H_
	#define _PSPRADIO_H_

#include <PSPApp.h>
#include <PSPSound.h>
#include "ScreenHandler.h"
#include "PlayListScreen.h"
#include "SHOUTcastScreen.h"
#include <UI_Interface.h>
#include "Screen.h"

#define PSPRADIO_VERSION	"0.38.073"

#define CFG_FILENAME		"PSPRadio.cfg"

extern CScreen *rootScreen;

class CPSPRadio : public CPSPApp
{
private:
	CIniParser *m_Config;
	CPSPSound *m_Sound;
	IPSPRadio_UI *m_UI;
	CScreenHandler *m_ScreenHandler;

public:
#ifdef DYNAMIC_BUILD
	CPSPRadio(): CPSPApp("PSPRadio", PSPRADIO_VERSION)
#else
	CPSPRadio(): CPSPApp("PSPRadio", PSPRADIO_VERSION, "Static")
#endif	
	{
		/** Initialize to some sensible defaults */
		m_Config = NULL;
		m_Sound = NULL;
		m_UI = NULL;
		m_ScreenHandler = NULL;
	};
	
	int Setup(int argc, char **argv);
	int ProcessEvents();
	
	CIniParser *GetConfig(){return m_Config;}

private:
	/** Setup */
	int Setup_OpenConfigFile(char *strCurrentDir);
	int Setup_Logging(char *strCurrentDir);
	int Setup_UI(char *strCurrentDir);
	int Setup_Sound();
	
	void OnExit();
	void OnVBlank();
	int OnPowerEvent(int pwrflags);
};

#endif
