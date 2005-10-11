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
#include "DirList.h"
#include "PlayList.h"
#include "TextUI.h"
#include "GraphicsUI.h"
#include "SandbergUI.h" 

asm(".global __lib_stub_top");
asm(".global __lib_stub_bottom");

/* Define the module info section */
PSP_MODULE_INFO("PSPRADIO", 0x1000, 1, 1);
/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_VFPU);
PSP_MAIN_THREAD_PRIORITY(80);
//PSP_MAIN_THREAD_STACK_SIZE_KB(512);


#define CFG_FILENAME "PSPRadio.cfg"
#define ReportError pPSPApp->ReportError

#define METADATA_STREAMURL_TAG "StreamUrl='"
#define METADATA_STREAMTITLE_TAG "StreamTitle='"

class myPSPApp : public CPSPApp
{
private:
	CIniParser *m_Config;
	CPSPSound *m_Sound;
	CPlayList *m_CurrentPlayList;
	CDirList  *m_CurrentPlayListDir;
	CPlayList::songmetadata *m_CurrentMetaData;
	IPSPRadio_UI *UI;
	int m_iNetworkProfile;
	bool m_NetworkStarted;
	
public:
	myPSPApp(): CPSPApp("PSPRadio", "0.35-pre4")
	{
		/** Initialize to some sensible defaults */
		m_iNetworkProfile = 1;
		m_NetworkStarted  = false;
		m_Config = NULL;
		m_Sound = NULL;
		m_CurrentPlayList = NULL;
		m_CurrentPlayListDir = NULL;
		UI = NULL;
		m_CurrentMetaData = NULL;
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
		
		m_CurrentMetaData  = new CPlayList::songmetadata;
		if (m_CurrentMetaData)
		{
			memset(m_CurrentMetaData, 0, sizeof(CPlayList::songmetadata));
		}
		else
		{
			Log(LOG_ERROR, "Memory allocation error during Metadata allocation.");
		}
		
		Setup_UI(strDir);
	
		UI->SetTitle(strAppTitle);
		
		if (1 == m_Config->GetInteger("USB:ENABLED", 0))
		{
			EnableUSB();
		}
		
		if (-1 != m_Config->GetInteger("SYSTEM:MAIN_THREAD_PRIO"))
		{
		
			Log(LOG_INFO, "Setting MAIN_THREAD_PRIO to %d as specified in config file.", 
				m_Config->GetInteger("SYSTEM:MAIN_THREAD_PRIO"));

			sceKernelChangeThreadPriority(sceKernelGetThreadId(), m_Config->GetInteger("SYSTEM:MAIN_THREAD_PRIO"));
		}
		
		Setup_PlayLists();
	
		Setup_Sound();
		
		Setup_Network();
		
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
			UI = new CGraphicsUI();
		}
		else if (0 == strcmp(m_Config->GetStr("UI:MODE"), "3D"))
		{
			UI = new CSandbergUI();
		}
		else
		{
			UI = new CTextUI();
		}
		
		UI->Initialize(strCurrentDir); /* Initialize takes cwd */
		
		return 0;
	}
	
	int Setup_Network()
	{
		m_iNetworkProfile = m_Config->GetInteger("WIFI:PROFILE", 1);
		
		if (m_iNetworkProfile < 1)
		{
			m_iNetworkProfile = 1;
			Log(LOG_ERROR, "Network Profile in m_Config file is invalid. Network profiles start from 1.");
		}
		
		if (1 == m_Config->GetInteger("WIFI:AUTOSTART", 0))
		{
			UI->DisplayMessage_EnablingNetwork();
			Log(LOG_INFO, "WIFI AUTOSTART SET: Enabling Network; using profile: %d", m_iNetworkProfile);
			EnableNetwork(m_iNetworkProfile);
			UI->DisplayMessage_NetworkReady(GetMyIP());
			Log(LOG_INFO, "Enabling Network: Done. IP='%s'", GetMyIP());
			m_NetworkStarted = 1;
		}
		else
		{
			Log(LOG_INFO, "WIFI AUTOSTART Not Set, Not starting network");
			DisplayCurrentNetworkSelection();
		}
		
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
					Log(LOG_LOWLEVEL, "Displaying current playlist");
					UI->DisplayPLList(m_CurrentPlayListDir);
					
					if(m_CurrentPlayList->GetNumberOfSongs() > 0)
					{
						UI->DisplayPLEntries(m_CurrentPlayList);
						/** Populate m_CurrentMetaData */
						//don't until user starts it!
						//m_CurrentPlayList->GetCurrentSong(m_CurrentMetaData);
						//UI->OnNewSongData(m_CurrentMetaData);
					}
				}
			}
			UI->DisplayMainCommands();
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
		if (UI)
		{
			Log(LOG_LOWLEVEL, "Exiting. Calling UI->Terminate");
			UI->Terminate();
			Log(LOG_LOWLEVEL, "Exiting. Destroying UI object");
			delete(UI);
			UI = NULL;
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
		if (m_CurrentMetaData)
		{
			delete (m_CurrentMetaData);
		}
		Log(LOG_LOWLEVEL, "Exiting. The end.");
	}

	void OnButtonReleased(int iButtonMask)
	{
		Log(LOG_VERYLOW, "OnButtonReleased(): iButtonMask=0x%x", iButtonMask);
		if (m_Sound)
		{
			CPSPSound::pspsound_state playingstate = m_Sound->GetPlayState();
			
			
			if (iButtonMask & PSP_CTRL_LEFT && !m_NetworkStarted)
			{
				m_iNetworkProfile --;
				m_iNetworkProfile %= (GetNumberOfNetworkProfiles());
				if (m_iNetworkProfile < 1)
					m_iNetworkProfile = 1;
				
				DisplayCurrentNetworkSelection();
			}
			else if (iButtonMask & PSP_CTRL_RIGHT && !m_NetworkStarted)
			{
				m_iNetworkProfile ++;
				m_iNetworkProfile %= (GetNumberOfNetworkProfiles());
				if (m_iNetworkProfile < 1)
					m_iNetworkProfile = 1;
				
				DisplayCurrentNetworkSelection();
			}
			else if (iButtonMask & PSP_CTRL_LTRIGGER)
			{
				m_CurrentPlayList->Prev();
				UI->DisplayPLEntries(m_CurrentPlayList);
			}
			else if (iButtonMask & PSP_CTRL_RTRIGGER)
			{
				m_CurrentPlayList->Next();
				UI->DisplayPLEntries(m_CurrentPlayList);
			}
			else if (iButtonMask & PSP_CTRL_UP)
			{
				m_CurrentPlayListDir->Prev();
				UI->DisplayPLList(m_CurrentPlayListDir);
				m_CurrentPlayList->Clear();
				m_CurrentPlayList->LoadPlayListURI(m_CurrentPlayListDir->GetCurrentURI());
				UI->DisplayPLEntries(m_CurrentPlayList);
			}
			else if (iButtonMask & PSP_CTRL_DOWN)
			{
				m_CurrentPlayListDir->Next();
				UI->DisplayPLList(m_CurrentPlayListDir);
				m_CurrentPlayList->Clear();
				m_CurrentPlayList->LoadPlayListURI(m_CurrentPlayListDir->GetCurrentURI());
				UI->DisplayPLEntries(m_CurrentPlayList);
			}
			else if (iButtonMask & PSP_CTRL_CROSS || iButtonMask & PSP_CTRL_CIRCLE) 
			{
				switch(playingstate)
				{
					case CPSPSound::STOP:
					case CPSPSound::PAUSE:
						//eCurrentMetaData();
						m_Sound->GetStream()->SetFile(m_CurrentPlayList->GetCurrentFileName());
						UI->DisplayActiveCommand(CPSPSound::PLAY);
						m_Sound->Play();
						/** Populate m_CurrentMetaData */
						m_CurrentPlayList->GetCurrentSong(m_CurrentMetaData);
						UI->OnNewSongData(m_CurrentMetaData);
						break;
					case CPSPSound::PLAY:
						/** No pausing for URLs, only for Files(local) */
						if (m_Sound->GetStream()->GetType() == CPSPSoundStream::STREAM_TYPE_FILE)
						{
							m_Sound->GetStream()->SetFile(m_CurrentPlayList->GetCurrentFileName());
							UI->DisplayActiveCommand(CPSPSound::PAUSE);
							m_Sound->Pause();
						}
						break;
				}
			}
			else if (iButtonMask & PSP_CTRL_SQUARE)
			{
				if (playingstate == CPSPSound::PLAY || playingstate == CPSPSound::PAUSE)
				{
					UI->DisplayActiveCommand(CPSPSound::STOP);
					m_Sound->Stop();
				}
			}
			else if (iButtonMask & PSP_CTRL_TRIANGLE && !m_NetworkStarted)
			{
				if (sceWlanGetSwitchState() != 0)
				{
					m_ExitSema->Up();

					UI->DisplayActiveCommand(CPSPSound::STOP);
					m_Sound->Stop();
					sceKernelDelayThread(50000);  
					
					if (m_NetworkStarted)
					{
						UI->DisplayMessage_DisablingNetwork();
		
						Log(LOG_INFO, "Triangle Pressed. Restarting networking...");
						DisableNetwork();
						sceKernelDelayThread(500000);  
					}
					
					UI->DisplayMessage_EnablingNetwork();
	
					EnableNetwork(m_iNetworkProfile);
					
					UI->DisplayMessage_NetworkReady(GetMyIP());
					Log(LOG_INFO, "Triangle Pressed. Networking Enabled, IP='%s'...", GetMyIP());
					
					m_NetworkStarted = 1;
					
					m_ExitSema->Down();
				}
				else
				{
					ReportError("The Network Switch is OFF, Cannot Start Network.");
				}
				
			}
		}
	};
	
	int ProcessEvents()
	{
		char MData[MAX_METADATA_SIZE];
		char *strURL = "";
		char *strTitle = "";
		CPSPEventQ::QEvent event = { 0, 0, NULL };
		int rret = 0;

		m_ExitSema->Up();
		for (;;)
		{
			//Log(LOG_VERYLOW, "ProcessMessages()::Calling Receive. %d Messages in Queue", m_EventToPSPApp->Size());
			if ( m_EventToPSPApp->Size() > 100 )
			{
				Log(LOG_ERROR, "ProcessEvents(): Too many events backed-up!: %d. Exiting!", m_EventToPSPApp->Size());
				UI->DisplayErrorMessage("Event Queue Backed-up, Exiting!");
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
				UI->DisplayErrorMessage((char*)event.pData);
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
					if (CPSPSound::PLAY == m_Sound->GetPlayState())
					{
						UI->DisplayBufferPercentage(m_Sound->GetBufferFillPercentage());
					}
					break;
				case MID_THPLAY_DONE: /** Done with the current stream! */
					UI->DisplayActiveCommand(CPSPSound::STOP);
					m_Sound->Stop();
					break;
					
				case MID_THDECODE_DECODING:
					UI->OnNewStreamStarted();
					break;
				//case MID_THDECODE_ASLEEP:
				//	break;
					
				case MID_DECODE_STREAM_OPENING:
					UI->OnStreamOpening();
					break;
				case MID_DECODE_STREAM_OPEN_ERROR:
					UI->OnStreamOpeningError();
					m_Sound->Stop(); /** Stop on error!. patch by echto */
					break;
				case MID_DECODE_STREAM_OPEN:
					UI->OnStreamOpeningSuccess();
					break;
				case MID_DECODE_METADATA_INFO:
					memcpy(MData, event.pData, MAX_METADATA_SIZE);
					
					/** GetMetadataValue() modifies the metadata, so call it
					with the last tag first */
					strURL   = GetMetadataValue(MData, METADATA_STREAMURL_TAG);
					strTitle = GetMetadataValue(MData, METADATA_STREAMTITLE_TAG);
					
					/** Update m_CurrentMetaData, and notify UI. */
					strcpy(m_CurrentMetaData->strFileTitle, strTitle);
					strcpy(m_CurrentMetaData->strURL, strURL);
					UI->OnNewSongData(m_CurrentMetaData);
					break;
				//case MID_DECODE_DONE:
				//	break;
				case MID_DECODE_FRAME_INFO_HEADER:
					struct mad_header *Header;
					Header = (struct mad_header *)event.pData;
					/** Update m_CurrentMetaData, and notify UI. */
					m_CurrentMetaData->SampleRate = Header->samplerate;
					m_CurrentMetaData->BitRate = Header->bitrate;
					UI->OnNewSongData(m_CurrentMetaData);
					
					break;
				case MID_DECODE_FRAME_INFO_LAYER:
					/** Update m_CurrentMetaData, and notify UI. */
					strcpy(m_CurrentMetaData->strMPEGLayer, (char*)event.pData);
					UI->OnNewSongData(m_CurrentMetaData);
					break;
				case MID_TCP_CONNECTING_PROGRESS:
					UI->OnConnectionProgress();
					break;
					
				case MID_ONBUTTON_PRESSED:
					break;
					
				case MID_ONBUTTON_RELEASED:
					OnButtonReleased(*((int*)event.pData));
					break;
					
				case MID_ONVBLANK:
					UI->OnVBlank();
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
		UI->OnVBlank();
	};
	
	/** Raw metadata looks like this:
	 *  "StreamTitle='title of the song';StreamUrl='url address';"
	 */
	char *GetMetadataValue(char *strMetadata, char *strTag)
	{
		char *ret = "Parse Error";
		
		if (strMetadata && 
			strTag && 
			(strlen(strMetadata) > strlen(strTag)) && 
			strstr(strMetadata, strTag))
		{
			ret = strstr(strMetadata, strTag) + strlen(strTag);
			if (strchr(ret, ';'))
			{
				*(strchr(ret, ';') - 1) = 0;
			}
		}
		
		return ret;
	}
	
	int GetNumberOfNetworkProfiles()
	{
		int numNetConfigs = 1;
		while (sceUtilityCheckNetParam(numNetConfigs++) == 0)
		{};
		
	
		return numNetConfigs-1;
	}
	
	void DisplayCurrentNetworkSelection()
	{
		netData data;
		memset(&data, 0, sizeof(netData));
		data.asUint = 0xBADF00D;
		memset(&data.asString[4], 0, 124);
		sceUtilityGetNetParam(m_iNetworkProfile, 0/**Profile Name*/, &data);
		
		UI->DisplayMessage_NetworkSelection(m_iNetworkProfile, data.asString);
		Log(LOG_INFO, "Current Network Profile Selection: %d Name: '%s'", m_iNetworkProfile, data.asString);
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

