/*
	PSPRadio / Music streaming client for the PSP. (Initial Release: Sept. 2005)
	Copyright (C) 2005  Rafael Cabezas a.k.a. Raf

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

// 	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include <PSPApp.h>
#include <PSPSound.h>
#include <Main.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <malloc.h>
#include <iniparser.h>
#include <Tools.h>
#include <Logging.h>
#include <pspwlan.h>
#include <psphprm.h>
#include "ScreenHandler.h"
#include "PlayListScreen.h"
#include "OptionsScreen.h"
#include "OptionsPluginMenuScreen.h"
#include "SHOUTcastScreen.h"
#include "LocalFilesScreen.h"
#include <UI_Interface.h>
#include "PRXLoader.h"
#include "PSPRadio_Exports.h"

#define ReportError pPSPApp->ReportError

CScreenHandler::CScreenHandler(char *strCWD, CIniParser *Config, CPSPSound *Sound, Screen InitialScreen)
{
	m_RequestOnPlayOrStop = NOTHING;
	m_strCurrentUIName = strdup(DEFAULT_UI_MODULE);
	m_strCurrentSkin = strdup(DEFAULT_SKIN);
	m_UI = NULL;
	m_strCWD = strdup(strCWD);
	m_Config = Config;
	m_Sound = Sound;
	m_PlayMode = PLAYMODE_NORMAL; //PLAYMODE_SINGLE;//

	Log(LOG_VERYLOW, "CScreenHandler Ctor");
	/** Create Screens... */
	Screens[PSPRADIO_SCREEN_LOCALFILES] =
		new LocalFilesScreen(PSPRADIO_SCREEN_LOCALFILES, this);
	((LocalFilesScreen*)Screens[PSPRADIO_SCREEN_LOCALFILES])->SetConfigFilename("LocalFilesScreen.cfg");
	((LocalFilesScreen*)Screens[PSPRADIO_SCREEN_LOCALFILES])->LoadLists();

	Screens[PSPRADIO_SCREEN_PLAYLIST] =
		new PlayListScreen(PSPRADIO_SCREEN_PLAYLIST, this);
	((PlayListScreen*)Screens[PSPRADIO_SCREEN_PLAYLIST])->SetConfigFilename("PlaylistScreen.cfg");
	((PlayListScreen*)Screens[PSPRADIO_SCREEN_PLAYLIST])->LoadLists();

	Screens[PSPRADIO_SCREEN_SHOUTCAST_BROWSER] =
		new SHOUTcastScreen(PSPRADIO_SCREEN_SHOUTCAST_BROWSER, this);
	((SHOUTcastScreen*)Screens[PSPRADIO_SCREEN_SHOUTCAST_BROWSER])->SetConfigFilename("SHOUTcastScreen.cfg");
	((SHOUTcastScreen*)Screens[PSPRADIO_SCREEN_SHOUTCAST_BROWSER])->LoadLists();

	Screens[PSPRADIO_SCREEN_OPTIONS] =
		new OptionsScreen(PSPRADIO_SCREEN_OPTIONS, this);
	((OptionsScreen*)Screens[PSPRADIO_SCREEN_OPTIONS])->SetConfigFilename("OptionsScreen.cfg");

#ifdef DYNAMIC_BUILD
	Screens[PSPRADIO_SCREEN_OPTIONS_PLUGIN_MENU] =
		new OptionsPluginMenuScreen(PSPRADIO_SCREEN_OPTIONS_PLUGIN_MENU, this);
	((OptionsScreen*)Screens[PSPRADIO_SCREEN_OPTIONS_PLUGIN_MENU])->SetConfigFilename("OptionsPluginScreen.cfg");

#endif	
	m_CurrentScreen = Screens[InitialScreen];
	Log(LOG_LOWLEVEL, "Setting m_CurrentScreen to Screens[%d]=%p", InitialScreen, m_CurrentScreen);
	
	/* To avoid getting stuck when Options are the initial screen */
	if (InitialScreen == PSPRADIO_SCREEN_OPTIONS)
	{
		m_PreviousScreen = Screens[PSPRADIO_SCREEN_LOCALFILES];
	}
	else
	{
		m_PreviousScreen = m_CurrentScreen;
	}
	m_StreamOwnerScreen = NULL;

	m_UIModuleLoader = new CPRXLoader();
	if (m_UIModuleLoader == NULL)
	{
		Log(LOG_ERROR, "Memory error - Unable to create CPRXLoader.");
	}
	
	SetInitialScreen(InitialScreen);

	
}

CScreenHandler::~CScreenHandler()
{
	if (m_UI)
	{
		Log(LOG_LOWLEVEL, "Exiting. Calling UI->Terminate");
		m_UI->Terminate();
		Log(LOG_LOWLEVEL, "Exiting. Destroying UI object");
		delete(m_UI);
		m_UI = NULL;
		Log(LOG_LOWLEVEL, "Exiting. UI object deleted.");
	}
	if (m_strCWD)
	{
		Log(LOG_LOWLEVEL, "Exiting. deleting m_strCWD.");
		free(m_strCWD), m_strCWD = NULL;
	}

	for (int i=PSPRADIO_SCREEN_LIST_BEGIN; i < PSPRADIO_SCREEN_LIST_END; i++)
	{
		Log(LOG_LOWLEVEL, "Exiting. Deleting Screens.");
		delete(Screens[i]), Screens[i] = NULL;
	}
	
	if (m_strCurrentSkin)
	{
		Log(LOG_LOWLEVEL, "Exiting. Deleting m_strCurrentSkin string.");
		free(m_strCurrentSkin), m_strCurrentSkin = NULL;
	}

	if (m_strCurrentUIName)
	{
		Log(LOG_LOWLEVEL, "Exiting. Deleting m_strCurrentUIName string.");
		free(m_strCurrentUIName), m_strCurrentUIName = NULL;
	}
	Log(LOG_LOWLEVEL, "~CScreenHandler() End.");
}

IPSPRadio_UI *CScreenHandler::StartUI(const char *strUIModule, const char *strSkinName, bool bJustChangeSkin)
{
 	char strSkinDirectory[MAXPATHLEN];
	
	bool wasPolling = false;

  /* strSkinDirectory = prxname without extension */
  strlcpy(strSkinDirectory, strUIModule, MAXPATHLEN);
  strSkinDirectory[MAXPATHLEN - 1] = 0;
  if (strrchr(strSkinDirectory, '.'))
  {
    *strrchr(strSkinDirectory, '.') = 0;
  }
  
  /* If skin name is passed defined, it is added */
  if (strcmp(strSkinName, DEFAULT_SKIN) != 0)
  {
    strcat(strSkinDirectory, ".");
    strcat(strSkinDirectory, strSkinName);
  }

  if (bJustChangeSkin == false)
  {
	  if (m_UI)
	  {
      wasPolling = pPSPApp->IsPolling();
		  if (wasPolling)
		  {
			  /** If PSPRadio was running, then notify it that we're switching UIs */
			  Log(LOG_LOWLEVEL, "Notifying PSPRadio That we're switching UIs: From '%s' to '%s'. Skin From '%s' to '%s'",
				  GetCurrentUIName(), strUIModule,
				  GetCurrentSkin(), strSkinName);
			  pPSPApp->SendEvent(EID_NEW_UI_POINTER, NULL, SID_SCREENHANDLER);
			  pPSPApp->StopPolling();
		  }
  
		  Log(LOG_INFO, "StartUI: Destroying current UI");
		  m_UI->Terminate();
		  delete(m_UI), m_UI = NULL;
		  Log(LOG_INFO, "Unloading Module");
		  m_UIModuleLoader->Unload();
		  Log(LOG_LOWLEVEL, "StartUI: Current UI destroyed.");
	  }
	  
	  Log(LOG_LOWLEVEL, "StartUI: Starting UI '%s' using skin: '%s' (%s)", 
					  strUIModule, 
					  strSkinName,
					  strSkinDirectory );

    int id = m_UIModuleLoader->Load(strUIModule);
    
    if (m_UIModuleLoader->IsLoaded() == true)
    {
      SceKernelModuleInfo modinfo;
      memset(&modinfo, 0, sizeof(modinfo));
      modinfo.size = sizeof(modinfo);
      sceKernelQueryModuleInfo(id, &modinfo);
      Log(LOG_ALWAYS, "TEXT_ADDR: '%s' Loaded at text_addr=0x%x",
        strUIModule, modinfo.text_addr);
    
      int iRet = m_UIModuleLoader->Start();
      
      Log(LOG_LOWLEVEL, "Module start returned: 0x%x", iRet);
      
    }
    else
    {
      Log(LOG_ERROR, "Error loading '%s' Module. Error=0x%x", strUIModule, m_UIModuleLoader->GetError());
    }
    Log(LOG_INFO, "Calling ModuleStartUI() (at addr %p)", &ModuleStartUI);
    m_UI = ModuleStartUI();

    SetCurrentUIName(strUIModule);
  }
	
	
	//Log(LOG_LOWLEVEL, "In PSPRadioPRX: _global_impure_ptr=%p, _impure_ptr=%p", _global_impure_ptr, _impure_ptr);
	
  SetCurrentSkinName(strSkinName);

	Log(LOG_INFO, "%s m_UI = %p, name='%s', skin='%s'", 
     bJustChangeSkin?"Skin Changed:":"UI Started:",
     m_UI, GetCurrentUIName(), GetCurrentSkin());
	
	Log(LOG_LOWLEVEL, "Calling m_UI->Initialize");
	m_UI->Initialize(GetCWD(), strSkinDirectory);
	
	Log(LOG_LOWLEVEL, "Calling currentscreen activate in (m_CurrentScreen=%p)", m_CurrentScreen);
	
	m_CurrentScreen->Activate(m_UI);
	Log(LOG_LOWLEVEL, "Activate called.");
	
	if (wasPolling)
	{
		/** If PSPRadio was running, then notify it of the new address of the UI */
		Log(LOG_LOWLEVEL, "Notifying PSPRadio of new UI's address (%p)", m_UI );
		pPSPApp->SendEvent(EID_NEW_UI_POINTER, m_UI, SID_SCREENHANDLER);
		pPSPApp->StartPolling();
	}

	return m_UI;
}

void CScreenHandler::PrepareShutdown()
{
	if(m_UI)
	{
		m_UI->PrepareShutdown();
	}
}

void CScreenHandler::OnVBlank()
{
	if (m_UI)
	{
		m_UI->OnVBlank();
	}
}

void CScreenHandler::CommonInputHandler(int iButtonMask, u32 iEventType) /** Event Type is MID_ONBUTTON_RELEASED or MID_ONBUTTON_REPEAT */
{
	/** Only do UP and DOWN repeats */
	if (MID_ONBUTTON_REPEAT == iEventType)
	{
		if(IS_BUTTON_PRESSED(iButtonMask, PSPRadioButtonMap.BTN_FWD) || IS_BUTTON_PRESSED(iButtonMask, PSPRadioButtonMap.BTN_BACK))
		{
			return;
		}
	}
	else if (MID_ONBUTTON_LONG_PRESS == iEventType)
	{
		if (IS_BUTTON_PRESSED(iButtonMask, PSPRadioButtonMap.BTN_TAKE_SCREENSHOT))
		{
			GetSound()->Stop(); /** Stop stream if playing, else the event queue can back-up */

			// Generic screenshot method, which works for all UI classes

			if (m_UI)
			{
				m_UI->OnScreenshot(PSPRADIO_SCREENSHOT_ACTIVE);
			}

			gPSPRadio->TakeScreenShot();

			if (m_UI)
			{
				m_UI->OnScreenshot(PSPRADIO_SCREENSHOT_NOT_ACTIVE);
			}
		}
	}

	switch (m_CurrentScreen->GetId())
	{
		case CScreenHandler::PSPRADIO_SCREEN_LOCALFILES:
		case CScreenHandler::PSPRADIO_SCREEN_PLAYLIST:
		case CScreenHandler::PSPRADIO_SCREEN_SHOUTCAST_BROWSER:
			if (IS_BUTTON_PRESSED(iButtonMask, PSPRadioButtonMap.BTN_OPTIONS))		/** Go to Options screen */
			{
				// Enter option menu and store the current screen
				m_PreviousScreen = m_CurrentScreen;
				m_CurrentScreen  = Screens[PSPRADIO_SCREEN_OPTIONS];
				m_CurrentScreen->Activate(m_UI);
			}
			else if (IS_BUTTON_PRESSED(iButtonMask, PSPRadioButtonMap.BTN_CYCLE_SCREENS)) /** Cycle through screens (except options) */
			{
				m_CurrentScreen = Screens[m_CurrentScreen->GetId()+1];
				/** Don't go to options screen with triangle;
					Also, as options screen is the last one in the list
					we cycle back to the first one */
				if (m_CurrentScreen->GetId() == PSPRADIO_SCREEN_OPTIONS)
				{
					m_CurrentScreen = Screens[PSPRADIO_SCREEN_LIST_BEGIN];
				}
				m_CurrentScreen->Activate(m_UI);
			}
			else if (IS_BUTTON_PRESSED(iButtonMask, PSPRadioButtonMap.BTN_CYCLE_SCREENS_BACK)) /** Cycle through screens (except options) */
			{
				int newscreenindex = m_CurrentScreen->GetId() - 1;
				if (newscreenindex < PSPRADIO_SCREEN_LIST_BEGIN)
				{
					/** Don't go to options screen with triangle;
						Also, as options screen is the last one in the list
						we cycle back to the first one */
					newscreenindex = PSPRADIO_SCREEN_OPTIONS -1;
				}
					
				m_CurrentScreen = Screens[newscreenindex];
				m_CurrentScreen->Activate(m_UI);
			}
			else
			{
				m_CurrentScreen->InputHandler(iButtonMask);
			}
			break;
		case CScreenHandler::PSPRADIO_SCREEN_OPTIONS:
#ifdef DYNAMIC_BUILD
		case CScreenHandler::PSPRADIO_SCREEN_OPTIONS_PLUGIN_MENU:
#endif
			if (IS_BUTTON_PRESSED(iButtonMask, PSPRadioButtonMap.BTN_OPTIONS_EXIT))	/** Get out of Options screen */
			{
				// Go back to where we were before entering the options menu
				m_CurrentScreen = m_PreviousScreen;
				m_CurrentScreen->Activate(m_UI);
			}
			else
			{
				m_CurrentScreen->InputHandler(iButtonMask);
			}
			break;
	}
}

void CScreenHandler::OnHPRMReleased(u32 iHPRMMask)
{
	if (GetCurrentScreen())
		((PlayListScreen*)GetCurrentScreen())->OnHPRMReleased(iHPRMMask);
};

bool  CScreenHandler::SetCurrentUIName(const char *strNewName)
{
  char strTempName[128];
  strlcpy(strTempName, strNewName, 128);

  if (m_strCurrentUIName)
  {
    free(m_strCurrentUIName), m_strCurrentUIName = NULL;
  }
  m_strCurrentUIName = strdup(strTempName);

  return (m_strCurrentUIName!=NULL);
}

bool  CScreenHandler::SetCurrentSkinName(const char *strNewName)
{
  char strTempName[128];
  strlcpy(strTempName, strNewName, 128);

  if (m_strCurrentSkin)
  {
    free(m_strCurrentSkin), m_strCurrentSkin = NULL;
  }
  m_strCurrentSkin = strdup(strTempName);

  return (m_strCurrentSkin!=NULL);
}

/*----------------------------------*/
void IScreen::Activate(IPSPRadio_UI *UI)
{
	m_UI = UI;
	m_UI->Initialize_Screen(this);
	m_ScreenHandler->SetCurrentScreen(this);
}
