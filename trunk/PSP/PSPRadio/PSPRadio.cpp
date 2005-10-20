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
#include <Logging.h>
#include <pspwlan.h> 
#include <psphprm.h>
#include "ScreenHandler.h"
#include "DirList.h"
#include "PlayList.h"
#include "TextUI.h"
#include "GraphicsUI.h"
#include "SandbergUI.h" 
#include <ivorbisfile.h>

/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_VFPU);
PSP_MAIN_THREAD_PRIORITY(80);
//PSP_MAIN_THREAD_STACK_SIZE_KB(512);

#define CFG_FILENAME "PSPRadio.cfg"
#define ReportError pPSPApp->ReportError

class myPSPApp : public CPSPApp
{
private:
	CIniParser *m_Config;
	CPSPSound *m_Sound;
	CPlayList *m_CurrentPlayList;
	CDirList  *m_CurrentPlayListDir;
	IPSPRadio_UI *m_UI;
	CScreenHandler *m_ScreenHandler;
		
public:
	myPSPApp(): CPSPApp("PSPRadio", "0.36-pre2")
	{
		/** Initialize to some sensible defaults */
		m_Config = NULL;
		m_Sound = NULL;
		m_CurrentPlayList = NULL;
		m_CurrentPlayListDir = NULL;
		m_UI = NULL;
		m_ScreenHandler = NULL;
	};

	/** Setup */
	int Setup(int argc, char **argv)
	{
		/** open config file */
		char *strDir = NULL;
		char strAppTitle[140];
		
		m_ExitSema->Up(); /** This to prevent the app from exiting while in this area */
		
		sprintf(strAppTitle, "%s by Raf (http://rafpsp.blogspot.com/) Version %s\n",
			GetProgramName(),
			GetProgramVersion());
		
		strDir = strdup(argv[0]);
		if (!strDir)
			return -1;
			
		dirname(strDir); /** Retrieve the directory name */
		
		Setup_OpenConfigFile(strDir);
		
		Setup_Logging(strDir);
		
		Setup_Sound();
		
		Setup_UI(strDir);
	
		m_UI->SetTitle(strAppTitle);
		
		if (-1 != m_Config->GetInteger("SYSTEM:MAIN_THREAD_PRIO"))
		{
		
			Log(LOG_INFO, "Setting MAIN_THREAD_PRIO to %d as specified in config file.", 
				m_Config->GetInteger("SYSTEM:MAIN_THREAD_PRIO"));

			sceKernelChangeThreadPriority(sceKernelGetThreadId(), m_Config->GetInteger("SYSTEM:MAIN_THREAD_PRIO"));
		}
		
		Setup_PlayLists();
	
		m_ScreenHandler->SetUp(m_UI, m_Config, m_Sound, m_CurrentPlayList, m_CurrentPlayListDir);
		
		if (1 == m_Config->GetInteger("WIFI:AUTOSTART", 0))
		{
			Log(LOG_INFO, "WIFI AUTOSTART SET: Enabling Network; using profile: %d", 	
				m_ScreenHandler->GetCurrentNetworkProfile());
			m_ScreenHandler->Start_Network(m_Config->GetInteger("WIFI:PROFILE", 1));
		}
		else
		{
			Log(LOG_INFO, "WIFI AUTOSTART Not Set, Not starting network");
		}
		
		Log(LOG_VERYLOW, "Freeing strDir");
		free(strDir);
		
		Log(LOG_VERYLOW, "StartPolling()");
		StartPolling();
		
		Log(LOG_LOWLEVEL, "Exiting Setup()");

		m_ExitSema->Down();

		return 0;
	}
	
	int Setup_OpenConfigFile(char *strCurrentDir)
	{
		char *strFilename = NULL;
		
		strFilename = (char*)malloc(strlen(strCurrentDir) + 1 + strlen(CFG_FILENAME) + 10);
		
		if (!strFilename)
			return -1;
		
		sprintf(strFilename, "%s/%s", strCurrentDir, CFG_FILENAME);
		
		m_Config = new CIniParser(strFilename);
		
		free(strFilename), strFilename = NULL;
		
		return 0;
	}
	
	int Setup_Logging(char *strCurrentDir)
	{
		char *strFilename = NULL;
		if ( m_Config && (1 == m_Config->GetInteger("DEBUGGING:LOGFILE_ENABLED", 0)) )
		{
			int iLoglevel = m_Config->GetInteger("DEBUGGING:LOGLEVEL", 100);
			
			strFilename = (char*)malloc(strlen(strCurrentDir) + 1 + strlen(m_Config->GetStr("DEBUGGING:LOGFILE")) + 10);
			sprintf(strFilename, "%s/%s", strCurrentDir, m_Config->GetStr("DEBUGGING:LOGFILE"));
			/** Set Logging Global Object to use the configured logfile and loglevels */
			Logging.Set(strFilename, (loglevel_enum)iLoglevel);
			free(strFilename), strFilename = NULL;
			
			Log(LOG_ALWAYS, "--------------------------------------------------------");
			Log(LOG_ALWAYS, "%s Version %s Starting - Using Loglevel %d", GetProgramName(), GetProgramVersion(),
				iLoglevel);
		}
		
		return 0;
	}
	
	int Setup_UI(char *strCurrentDir)
	{
		Log(LOG_LOWLEVEL, "UI Mode = %s", m_Config->GetStr("UI:MODE"));
		
		if (0 == strcmp(m_Config->GetStr("UI:MODE"), "Graphics"))
		{
			m_UI = new CGraphicsUI();
		}
		else if (0 == strcmp(m_Config->GetStr("UI:MODE"), "3D"))
		{
			m_UI = new CSandbergUI();
		}
		else
		{
			m_UI = new CTextUI();
		}
		
		m_UI->Initialize(strCurrentDir); /* Initialize takes cwd */
		m_ScreenHandler = new CScreenHandler(m_UI, m_Config, m_Sound);
		
		return 0;
	}
	
	int Setup_Sound()
	{
		m_Sound = new CPSPSound();

		if (m_Sound)
		{
			/** Set the sound buffer size */
			size_t bufsize = m_Config->GetInteger("SYSTEM:SOUND_BUFFERSIZE", 0);
			if (0 != bufsize)
			{
				Log(LOG_INFO, "Setting SOUND BUFFERSIZE specified in m_Config file as %d.", bufsize);
				m_Sound->ChangeBufferSize(bufsize);
			}
			
			/** Set the thread priorities */
			if (-1 != m_Config->GetInteger("SYSTEM:DECODE_THREAD_PRIO", -1))
			{
			
				Log(LOG_INFO, "Setting DECODE_THREAD_PRIO to %d as specified in config file.", 
					m_Config->GetInteger("SYSTEM:DECODE_THREAD_PRIO"));
	
				m_Sound->SetDecodeThreadPriority(m_Config->GetInteger("SYSTEM:DECODE_THREAD_PRIO"));
			}
			if (-1 != m_Config->GetInteger("SYSTEM:PLAY_THREAD_PRIO", -1))
			{
			
				Log(LOG_INFO, "Setting PLAY_THREAD_PRIO to %d as specified in config file.", 
					m_Config->GetInteger("SYSTEM:PLAY_THREAD_PRIO"));
	
				m_Sound->SetPlayThreadPriority(m_Config->GetInteger("SYSTEM:PLAY_THREAD_PRIO"));
			}
		}
		else
		{
			Log(LOG_ERROR, "Error creating m_Sound object.");
		}	
		
		return 0;
	}
	
	int Setup_PlayLists()
	{
		m_CurrentPlayList = new CPlayList();
		
		if (m_CurrentPlayList)
		{
			m_CurrentPlayListDir = new CDirList();
			
			if (m_CurrentPlayListDir)
			{
				m_CurrentPlayListDir->LoadDirectory("PlayLists"); //**//
				if (m_CurrentPlayListDir->Size() > 0)
				{
					Log(LOG_LOWLEVEL, "Loading Playlist file '%s'.", m_CurrentPlayListDir->GetCurrentURI());
					m_CurrentPlayList->LoadPlayListURI(m_CurrentPlayListDir->GetCurrentURI());
				}
			}
			m_UI->DisplayMainCommands();
		}
		else
		{
			Log(LOG_ERROR, "Error creating CPlaylist object.");
		}
		
		return 0;
	}
	
	void OnExit()
	{
		Log(LOG_LOWLEVEL, "PSPRadio::OnExit()");
		if (m_Sound)
		{
			Log(LOG_LOWLEVEL, "Exiting. Destroying m_Sound object");
			delete(m_Sound);
			m_Sound = NULL;
		}
		if (m_UI)
		{
			Log(LOG_LOWLEVEL, "Exiting. Calling UI->Terminate");
			m_UI->Terminate();
			Log(LOG_LOWLEVEL, "Exiting. Destroying UI object");
			delete(m_UI);
			m_UI = NULL;
		}
		if (m_Config)
		{
			Log(LOG_LOWLEVEL, "Exiting. Destroying m_Config object");
			delete(m_Config);
		}
		if (m_CurrentPlayListDir)
		{
			delete(m_CurrentPlayListDir);
		}
		if (m_CurrentPlayList)
		{
			delete(m_CurrentPlayList);
		}
		Log(LOG_LOWLEVEL, "Exiting. The end.");
	}

	void OnButtonReleased(int iButtonMask)
	{
		static bool fOnExitMenu = false;
		
		if (false == fOnExitMenu)
		{
			if (iButtonMask & PSP_CTRL_HOME)
			{
				Log(LOG_VERYLOW, "Entering HOME menu, ignoring buttons..");
				fOnExitMenu = true;
				return;
			}
			switch (m_ScreenHandler->GetCurrentScreen())
			{
				case CScreenHandler::PSPRADIO_SCREEN_PLAYLIST:
					m_ScreenHandler->PlayListScreenInputHandler(iButtonMask);
					break;
				case CScreenHandler::PSPRADIO_SCREEN_OPTIONS:
					m_ScreenHandler->OptionsScreenInputHandler(iButtonMask);
					break;
			}
		}
		else
		{
			if ( (iButtonMask & PSP_CTRL_HOME)   ||
				 (iButtonMask & PSP_CTRL_CROSS)  ||
				 (iButtonMask & PSP_CTRL_CIRCLE)
				)
			{
				fOnExitMenu = false;
				Log(LOG_VERYLOW, "Exiting HOME menu");
			}
		}
	}
	
	void OnHPRMReleased(u32 iHPRMMask)
	{
		Log(LOG_VERYLOW, "OnHPRMReleased(): iHPRMMask=0x%x", iHPRMMask);
		if (m_Sound)
		{
			CPSPSound::pspsound_state playingstate = m_Sound->GetPlayState();

			if (iHPRMMask & PSP_HPRM_BACK)
			{
				m_CurrentPlayList->Prev();
				m_UI->DisplayPLEntries(m_CurrentPlayList);
			}
			else if (iHPRMMask & PSP_HPRM_FORWARD)
			{
				m_CurrentPlayList->Next();
				m_UI->DisplayPLEntries(m_CurrentPlayList);
			}

			else if (iHPRMMask & PSP_HPRM_PLAYPAUSE) 
			{
				switch(playingstate)
				{
					case CPSPSound::STOP:
					case CPSPSound::PAUSE:
						CurrentSoundStream->SetURI(m_CurrentPlayList->GetCurrentURI());
						m_UI->DisplayActiveCommand(CPSPSound::PLAY);
						m_Sound->Play();
						/** Populate m_CurrentMetaData */
						m_CurrentPlayList->GetCurrentSong(CurrentSoundStream->m_CurrentMetaData);
						//CurrentSoundStream->SetURI(m_CurrentPlayList->GetURI());
						m_UI->OnNewSongData(CurrentSoundStream->m_CurrentMetaData);
						break;
					case CPSPSound::PLAY:
						m_UI->DisplayActiveCommand(CPSPSound::STOP);
						Log(LOG_VERYLOW, "Calling Stop() on HPRM PLAY/PAUSE pressed; currently Playing.");
						m_Sound->Stop();
						break;
				}
			}
		}
	};

	int ProcessEvents()
	{
		CPSPEventQ::QEvent event = { 0, 0, NULL };
		int rret = 0;

		m_ExitSema->Up();
		for (;;)
		{
			//Log(LOG_VERYLOW, "ProcessMessages()::Calling Receive. %d Messages in Queue", m_EventToPSPApp->Size());
			if ( m_EventToPSPApp->Size() > 100 )
			{
				Log(LOG_ERROR, "ProcessEvents(): Too many events backed-up!: %d. Exiting!", m_EventToPSPApp->Size());
				m_UI->DisplayErrorMessage("Event Queue Backed-up, Exiting!");
				m_ExitSema->Down();
				return 0;
			}
			rret = m_EventToPSPApp->Receive(event);
			//Log(LOG_VERYLOW, "ProcessMessages()::Receive Ret=%d. eventid=0x%08x.", rret, event.EventId);
			switch (event.EventId)
			{
			case MID_PSPAPP_EXITING:
				if ( (SID_PSPAPP == event.SenderId) )
				{
					Log(LOG_INFO, "ProcessEvents(): MID_PSPAPP_EXITING received.");
					m_ExitSema->Down();
					return 0;
				}
				break;
			
			case MID_ERROR:
				m_UI->DisplayErrorMessage((char*)event.pData);
				Log(LOG_ERROR, (char*)event.pData);
				continue;
				break;
			}
			
			switch (event.SenderId)
			{
			//case PSPAPP_SENDER_ID:
			//	switch(event.EventId)
			//	[
			//	case ERROR
			//case SID_PSPSOUND:
			default:
				//Log(LOG_VERYLOW, "OnMessage: Message: MID=0x%x SID=0x%x", event.EventId, event.SenderId);
				switch(event.EventId)
				{
				//case MID_THPLAY_BEGIN:
				//	break;
				//case MID_THPLAY_END:
				//	break;
				case MID_BUFF_PERCENT_UPDATE:
					if (CScreenHandler::PSPRADIO_SCREEN_PLAYLIST == m_ScreenHandler->GetCurrentScreen())
					{
						if (CPSPSound::PLAY == m_Sound->GetPlayState())
						{
							m_UI->DisplayBufferPercentage(m_Sound->GetBufferFillPercentage());
						}
					}
					break;
				case MID_THPLAY_DONE: /** Done with the current stream! */
					Log(LOG_VERYLOW, "MID_THPLAY_DONE received, calling OnPlayStateChange(STOP)");
					OnPlayStateChange(CPSPSound::STOP);
					break;
					
				case MID_THDECODE_DECODING:
					m_UI->OnNewStreamStarted();
					break;
				//case MID_THDECODE_ASLEEP:
				//	break;
					
				case MID_DECODE_STREAM_OPENING:
					m_UI->OnStreamOpening();
					break;
				case MID_DECODE_STREAM_OPEN_ERROR:
					Log(LOG_VERYLOW, "MID_DECODE_STREAM_OPEN_ERROR received, calling OnPlayStateChange(STOP)");
					OnPlayStateChange(CPSPSound::STOP);
					m_UI->OnStreamOpeningError();
					break;
				case MID_DECODE_STREAM_OPEN:
					Log(LOG_VERYLOW, "MID_DECODE_STREAM_OPEN received, calling OnPlayStateChange(PLAY)");
					OnPlayStateChange(CPSPSound::PLAY);
					m_UI->OnStreamOpeningSuccess();
					break;
					
				//case MID_DECODE_DONE:
				//	break;
				case MID_NEW_METADATA_AVAILABLE:
					m_UI->OnNewSongData(CurrentSoundStream->m_CurrentMetaData);
					break;
					
				case MID_TCP_CONNECTING_PROGRESS:
					m_UI->OnConnectionProgress();
					break;
					
				case MID_ONBUTTON_PRESSED:
					break;
					
				case MID_ONBUTTON_RELEASED:
					OnButtonReleased(*((int*)event.pData));
					break;
					
				case MID_ONHPRM_RELEASED:
					OnHPRMReleased(*((u32*)event.pData));
					break;

				case MID_ONVBLANK:
					m_UI->OnVBlank();
					break;
				
				default:
					Log(LOG_VERYLOW, "ProcessEvents: Unhandled event: MID=0x%x SID=0x%x", event.EventId, event.SenderId);
					break;
				}
			}
		}
		m_ExitSema->Down();
		return 0;
	}
	
	void OnVBlank()
	{
		m_UI->OnVBlank();
	};
	
	void OnPlayStateChange(CPSPSound::pspsound_state NewPlayState)
	{
		static CPSPSound::pspsound_state OldPlayState = CPSPSound::STOP;
		
		switch(OldPlayState)
		{
			case CPSPSound::STOP:
				switch(NewPlayState)
				{
					case CPSPSound::PLAY:
						m_UI->DisplayActiveCommand(CPSPSound::PLAY);
						/** Populate m_CurrentMetaData */
						m_CurrentPlayList->GetCurrentSong(CurrentSoundStream->m_CurrentMetaData);
						m_UI->OnNewSongData(CurrentSoundStream->m_CurrentMetaData);
						break;
					
					case CPSPSound::STOP:
						m_UI->DisplayActiveCommand(CPSPSound::STOP);
						Log(LOG_VERYLOW, "Calling Stop() on OnPlayStateChange Old=STOP, New=STOP.");
						m_Sound->Stop();
						break;
					case CPSPSound::PAUSE:
					default:
						break;
				}
				break;
			
			case CPSPSound::PLAY:
				switch(NewPlayState)
				{
					case CPSPSound::STOP:
						m_UI->DisplayActiveCommand(CPSPSound::STOP);
						//m_Sound->Stop();
						
						if (CScreenHandler::PLAY == m_ScreenHandler->m_RequestOnPlayOrStop)
						{
							CurrentSoundStream->SetURI(m_CurrentPlayList->GetCurrentURI());
							
							/** Populate m_CurrentMetaData */
							m_CurrentPlayList->GetCurrentSong(CurrentSoundStream->m_CurrentMetaData);
							
							m_Sound->Play();
						}
						break;
						
					case CPSPSound::PLAY:
					case CPSPSound::PAUSE:
					default:
						break;
				}
				break;
			
			case CPSPSound::PAUSE:
			default:
				break;
		}
		
		m_ScreenHandler->m_RequestOnPlayOrStop = CScreenHandler::NOTHING; /** Reset */
		
		OldPlayState = NewPlayState;
	}

};

/** main */
int main(int argc, char **argv) 
{
	myPSPApp *PSPApp  = new myPSPApp();
	if (PSPApp)
	{
		PSPApp->Setup(argc, argv);
		PSPApp->ProcessEvents();
		
		delete(PSPApp);
	}
	
	return 0;
}

