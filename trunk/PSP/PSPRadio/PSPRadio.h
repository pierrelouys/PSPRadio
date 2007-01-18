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
#include "ScreenHandler/ScreenHandler.h"
#include "ScreenHandler/PlayListScreen.h"
#include "ScreenHandler/SHOUTcastScreen.h"
#include <UI_Interface.h>
#include "Screen.h"

#include "PSPRadio_Exports.h"

#define PSPRADIO_VERSION	"1.0"

#define CFG_FILENAME		"PSPRadio.cfg"

#define PLUGIN_OFF_STRING	"Off"


/** Sender IDs */
#define SID_PSPRADIO				0x00000001
#define SID_SCREENHANDLER			0x00000002

#define MID_GIVEUPEXCLISIVEACCESS	0x00000001
#define MID_PLUGINEXITED			0x00000002

#ifdef __cplusplus
	extern CScreen *rootScreen;
	
	class CPSPRadio : public CPSPApp
	{
	private:
		CIniParser		*m_Config;
		CPSPSound		*m_Sound;
		IPSPRadio_UI	*m_UI;
		CScreenHandler	*m_ScreenHandler;
		char 			*m_strCWD;
	
	public:
	#ifdef DYNAMIC_BUILD
		CPSPRadio(): CPSPApp("PSPRadio", PSPRADIO_VERSION)
	#else
	#ifdef DEVHOOK
		CPSPRadio(): CPSPApp("PSPRadio", PSPRADIO_VERSION, "dh")
	#else
		CPSPRadio(): CPSPApp("PSPRadio", PSPRADIO_VERSION, "Static")
	#endif
	#endif	
		{
			/** Initialize to some sensible defaults */
			m_Config = NULL;
			m_Sound = NULL;
			m_UI = NULL;
			m_ScreenHandler = NULL;
			m_strCWD = NULL;
			#ifdef DYNAMIC_BUILD
				m_ExclusiveAccessPluginType = (plugin_type)-1;
			#endif
		};
		
		int Main(int argc, char **argv);
		
		CIniParser *GetConfig(){return m_Config;}
		IPSPRadio_UI *GetUI() { return m_UI; }
		CScreenHandler *GetScreenHandler() { return m_ScreenHandler; }
	
		void TakeScreenShot();
	
	private:
		int Setup(int argc, char **argv);
		int ProcessEvents();
		
		char *ScreenshotName(char *path);
		void  ScreenshotStore(char *filename);
	
	#ifdef DYNAMIC_BUILD
	public:
		int LoadPlugin(char *strPlugin, plugin_type type);
		char *GetActivePluginName(plugin_type type);
		void SetExclusiveAccessPluginType(plugin_type type) { m_ExclusiveAccessPluginType = type; }

	private:
		CPRXLoader *m_ModuleLoader[NUMBER_OF_PLUGIN_TYPES];
		plugin_type m_ExclusiveAccessPluginType;
	#endif
	
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
#endif // __cplusplus


#endif
