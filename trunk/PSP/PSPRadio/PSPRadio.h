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
#include <VIS_Plugin.h>
#include "Screen.h"

#include "PSPRadio_Exports.h"

#define CFG_FILENAME		"PSPRadio.cfg"

#define PLUGIN_OFF_STRING	"Off"

extern _button_mappings_struct_ PSPRadioButtonMap;

/** Sender IDs */
#define SID_PSPRADIO				0x00000001
#define SID_SCREENHANDLER			0x00000002

#define MID_GIVEUPEXCLISIVEACCESS	0x00000001
#define MID_PLUGINEXITED			0x00000002

/* Power event data */

struct _PowerEventData
{
	bool bPlayAfterResume;
	bool bPauseAfterResume;
	bool bEnableNetworkAfterResume;
	int iCurrentStreamPosition;
};

#ifdef __cplusplus
	extern CScreen *rootScreen;
	
	class CPSPRadio : public CPSPApp
	{
	private:
		CIniParser		*m_Config;
		CPSPSound		*m_Sound;
		CScreenHandler	*m_ScreenHandler;
		char 			*m_strCWD;
		_PowerEventData  m_PowerEventData;
	
	public:
		IPSPRadio_UI	*m_UI;
		CPSPRadio(): CPSPApp("PSPRadio", IF_VERSION, REPO_VERSION)
		{
			/** Initialize to some sensible defaults */
			m_Config = NULL;
			m_Sound = NULL;
			m_UI = NULL;
			m_ScreenHandler = NULL;
			m_strCWD = NULL;
			m_ExclusiveAccessPluginType = (plugin_type)-1;
		};
		
		int Main(int argc, char **argv);
		
		CIniParser *GetConfig(){return m_Config;}
		CScreenHandler *GetScreenHandler() { return m_ScreenHandler; }
	
		void TakeScreenShot();
	
	private:
		int Setup(int argc, char **argv);
		int ProcessEvents();
		
		char *ScreenshotName(char *path);
		void  ScreenshotStore(char *filename);
	
	public:
		UIPlugin  *m_UIPluginData;

		VisPluginConfig m_VisPluginConfig;
		VisPluginGuFunctions m_VisPluginGuFunctions;
		VisPlugin *m_VisPluginData;

		int LoadPlugin(const char *strPlugin, plugin_type type);
		int UnloadPlugin(plugin_type type);
		char *GetActivePluginName(plugin_type type);
		void SetExclusiveAccessPluginType(plugin_type type) { m_ExclusiveAccessPluginType = type; }
		int Setup_ButtonMapping(CIniParser *pConfig);
		CPSPSound *GetSoundObject(){ return m_Sound; }

	private:
		CPRXLoader *m_ModuleLoader[NUMBER_OF_PLUGIN_TYPES];
		plugin_type m_ExclusiveAccessPluginType;
	
	private:
		/** Setup */
		int Setup_OpenConfigFile(char *strCurrentDir);
		int Setup_Logging(char *strCurrentDir);
		int Setup_UI(char *strCurrentDir);
		int Setup_Sound();
		
		void OnExit();
		int OnPowerEvent(int pwrflags);
		int OnPowerEventResumeComplete();
	};
#endif // __cplusplus

#endif
