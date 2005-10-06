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
#include <pspwlan.h> 
#include "PlayList.h"
#include "TextUI.h"
#include "GraphicsUI.h"

asm(".global __lib_stub_top");
asm(".global __lib_stub_bottom");

/* Define the module info section */
PSP_MODULE_INFO("PSPRADIO", 0x1000, 1, 1);
/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_VFPU);
PSP_MAIN_THREAD_PRIORITY(80);


#define CFG_FILENAME "PSPRadio.cfg"
#define ReportError pPSPApp->ReportError

#define METADATA_STREAMURL_TAG "StreamUrl='"
#define METADATA_STREAMTITLE_TAG "StreamTitle='"

class myPSPApp : public CPSPApp
{
private:
	CIniParser *config;
	CPSPSound_MP3 *MP3;
	CPlayList *m_PlayList;
	IPSPRadio_UI *UI;
	int m_iNetworkProfile;
	int m_NetworkStarted;
	
public:
	myPSPApp(): CPSPApp("PSPRadio", "0.35-pre1"){};

	/** Setup */
	int Setup(int argc, char **argv)
	{
		/** open config file */
		char strCfgFile[256], strLogFile[256];
		char strDir[256];
		char strAppTitle[140];
		
		m_ExitSema->Up(); /** This to prevent the app to exit while in this area */
		
		/** Initialize to some sensible defaults */
		m_iNetworkProfile = 1;
		m_NetworkStarted = 0;
		
		sprintf(strAppTitle, "%s by Raf (http://rafpsp.blogspot.com/) Version %s\n",
			GetProgramName(),
			GetProgramVersion());
		
		strcpy(strDir, argv[0]);
		dirname(strDir); /** Retrieve the directory name */
		sprintf(strCfgFile, "%s/%s", strDir, CFG_FILENAME);
			
		config = new CIniParser(strCfgFile);

		if (1 == config->GetInteger("DEBUGGING:LOGFILE_ENABLED", 0))
		{
			int iLoglevel = config->GetInteger("DEBUGGING:LOGLEVEL", 100);
			sprintf(strLogFile, "%s/%s", strDir, config->GetStr("DEBUGGING:LOGFILE"));
			/** Set Logging Global Object to use the configured logfile and loglevels */
			Logging.Set(strLogFile, (loglevel_enum)iLoglevel);
			Log(LOG_ALWAYS, "--------------------------------------------------------");
			Log(LOG_ALWAYS, "%s Version %s Starting - Using Loglevel %d", GetProgramName(), GetProgramVersion(),
				iLoglevel);
		}
		
		Log(LOG_LOWLEVEL, "UI Mode = %s", config->GetStr("UI:MODE"));
		
		if (0 == strcmp(config->GetStr("UI:MODE"), "Graphics"))
		{
			UI = new CGraphicsUI();
		}
		else
		{
			UI = new CTextUI();
		}
		
		UI->Initialize(strDir); /* Initialize takes cwd */
		UI->SetTitle(strAppTitle);
		
		if (1 == config->GetInteger("USB:ENABLED", 0))
		{
			EnableUSB();
		}
		
		m_iNetworkProfile = config->GetInteger("WIFI:PROFILE", 1);
		
		if (m_iNetworkProfile < 1)
		{
			m_iNetworkProfile = 1;
			Log(LOG_ERROR, "Network Profile in config file is invalid. Network profiles start from 1.");
		}
		
		if (config->GetInteger("WIFI:AUTOSTART", 0) == 1)
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
		
		MP3 = new CPSPSound_MP3();
		m_PlayList = new CPlayList();

		if (MP3 && m_PlayList)
		{
			if (strlen(config->GetStr("MUSIC:PLAYLIST")))
			{
				Log(LOG_LOWLEVEL, "Playlist file defined ('%s'), loading.", config->GetStr("MUSIC:PLAYLIST"));
				m_PlayList->LoadPlayListFile(config->GetStr("MUSIC:PLAYLIST"));
			}
			else if (strlen(config->GetStr("MUSIC:FILE")))
			{
				Log(LOG_LOWLEVEL, "Music File defined ('%s'), loading.", config->GetStr("MUSIC:FILE"));
				m_PlayList->InsertFile(config->GetStr("MUSIC:FILE"));
			}

			/** Set the sound buffer size */
			size_t bufsize = config->GetInteger("SOUND:BUFFERSIZE", 0);
			if (0 != bufsize)
			{
				Log(LOG_INFO, "SOUND BUFFERSIZE specified in config file as %d.", bufsize);
				MP3->ChangeBufferSize(bufsize);
			}
			
			SetCurrentFile();
			UI->DisplayMainCommands();
		}
		else
		{
			Log(LOG_ERROR, "Error creating CPSPSound_MP3 object, or CPlaylist object.");
		}
	
		Log(LOG_LOWLEVEL, "Exiting Setup()");

		m_ExitSema->Down();

		return 0;
	}
	
	void OnExit()
	{
		Log(LOG_LOWLEVEL, "PSPRadio::OnExit()");
		if (UI)
		{
			Log(LOG_LOWLEVEL, "Exiting. Calling UI->Terminate");
			UI->Terminate();
			Log(LOG_LOWLEVEL, "Exiting. Destroying UI object");
			delete(UI);
			UI = NULL;
		}
		if (MP3)
		{
			Log(LOG_LOWLEVEL, "Exiting. Destroying MP3 object");
			delete(MP3);
			MP3 = NULL;
		}
		if (config)
		{
			Log(LOG_LOWLEVEL, "Exiting. Destroying config object");
			delete(config);
		}
		Log(LOG_LOWLEVEL, "Exiting. The end.");
	}

	void OnButtonReleased(int iButtonMask)
	{
		Log(LOG_VERYLOW, "OnButtonReleased(): iButtonMask=0x%x", iButtonMask);
		if (MP3)
		{
			CPSPSound::pspsound_state playingstate = MP3->GetPlayState();
			
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
				MP3->Stop();
				m_PlayList->Prev();
				SetCurrentFile();
				sceKernelDelayThread(500000);  
				/** Only auto-play the prev song if currently playing */
				if (CPSPSound::PLAY == playingstate)
				{
					UI->DisplayActiveCommand(playingstate);
					MP3->Play();
				}
			}
			else if (iButtonMask & PSP_CTRL_RTRIGGER)
			{
				MP3->Stop();
				m_PlayList->Next();
				SetCurrentFile();
				sceKernelDelayThread(500000);  
				/** Only auto-play the next song if currently playing */
				if (CPSPSound::PLAY == playingstate)
				{
					UI->DisplayActiveCommand(playingstate);
					MP3->Play();
				}
			}
			else if (iButtonMask & PSP_CTRL_CROSS || iButtonMask & PSP_CTRL_CIRCLE) 
			{
				switch(playingstate)
				{
					case CPSPSound::STOP:
					case CPSPSound::PAUSE:
						UI->DisplayActiveCommand(CPSPSound::PLAY);
						MP3->Play();
						break;
					case CPSPSound::PLAY:
						/** No pausing for URLs, only for Files(local) */
						if (MP3->GetStream()->GetType() == CPSPSoundStream::STREAM_TYPE_FILE)
						{
							UI->DisplayActiveCommand(CPSPSound::PAUSE);
							MP3->Pause();
						}
						break;
				}
			}
			else if (iButtonMask & PSP_CTRL_SQUARE)
			{
				if (playingstate == CPSPSound::PLAY || playingstate == CPSPSound::PAUSE)
				{
					UI->DisplayActiveCommand(CPSPSound::STOP);
					MP3->Stop();
				}
			}
			else if (iButtonMask & PSP_CTRL_TRIANGLE && !m_NetworkStarted)
			{
				if (sceWlanGetSwitchState() != 0)
				{
					m_ExitSema->Up();

					UI->DisplayActiveCommand(CPSPSound::STOP);
					MP3->Stop();
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
	
	int OnMessage(int iMessageId, void *pMessage, int iSenderId)
	{
		char MData[MAX_METADATA_SIZE];
		CPlayList::songmetadata data;
		int iTmp = 0;
		char *strURL = "";
		char *strTitle = "";
		
		m_ExitSema->Up();
		
		if (iMessageId == MID_ERROR)
		{
			UI->DisplayErrorMessage((char*)pMessage);
			Log(LOG_ERROR, (char*)pMessage);
		}
		else
		{
			switch (iSenderId)
			{
			//case PSPAPP_SENDER_ID:
			//	switch(iMessageId)
			//	{
			//	case ERROR
			//case SID_PSPSOUND:
			default:
				switch(iMessageId)
				{
				//case MID_THPLAY_BEGIN:
				//	break;
				//case MID_THPLAY_END:
				//	break;
				case MID_BUFF_PERCENT_UPDATE:
					UI->DisplayBufferPercentage(MP3->GetBufferFillPercentage());
					break;
				case MID_THPLAY_DONE: /** Done with the current stream! */
					UI->DisplayActiveCommand(CPSPSound::STOP);
					MP3->Stop();
					break;
					
				case MID_THDECODE_AWOKEN:
					UI->OnNewStreamStarted();
					break;
				//case MID_THDECODE_ASLEEP:
				//	break;
					
				case MID_DECODE_STREAM_OPENING:
					UI->OnStreamOpening();
					break;
				case MID_DECODE_STREAM_OPEN_ERROR:
					UI->OnStreamOpeningError();
					break;
				case MID_DECODE_STREAM_OPEN:
					UI->OnStreamOpeningSuccess();
					break;
				case MID_DECODE_METADATA_INFO:
					memcpy(MData, pMessage, MAX_METADATA_SIZE);
					
					/** GetMetadataValue() modifies the metadata, so call it
					with the last tag first */
					strURL   = GetMetadataValue(MData, METADATA_STREAMURL_TAG);
					strTitle = GetMetadataValue(MData, METADATA_STREAMTITLE_TAG);
					
					//UI->DisplayMetadata(strTitle, strURL);
					iTmp = 0;
					MP3->GetStream()->SetFile(m_PlayList->GetCurrentFileName());
					iTmp = m_PlayList->GetCurrentSong(&data);
					if (0 == iTmp)
					{
						strcpy(data.strFileTitle, strTitle);
						strcpy(data.strURL, strURL);
						UI->OnNewSongData(&data);
					}
					
					break;
				//case MID_DECODE_DONE:
				//	break;
				case MID_DECODE_FRAME_INFO_HEADER:
					struct mad_header *Header;
					Header = (struct mad_header *)pMessage;
					UI->DisplaySampleRateAndKBPS(Header->samplerate, Header->bitrate/1000);
					break;
				case MID_DECODE_FRAME_INFO_LAYER:
					UI->DisplayMPEGLayerType((char*)pMessage);
					break;
				case MID_TCP_CONNECTING_PROGRESS:
					UI->OnConnectionProgress();
					break;
				
				default:
					Log(LOG_VERYLOW, "OnMessage: Unhandled message: MID=0x%x SID=0x%x", iMessageId, iSenderId);
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
	}
	
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
		data.asUint = 0xBADF00D;
		memset(&data.asString[4], 0, 124);
		sceUtilityGetNetParam(m_iNetworkProfile, 0/**Profile Name*/, &data);
		
		UI->DisplayMessage_NetworkSelection(m_iNetworkProfile, data.asString);
		Log(LOG_INFO, "Current Network Profile Selection: %d Name: '%s'", m_iNetworkProfile, data.asString);
	}
	
	void SetCurrentFile()
	{
		CPlayList::songmetadata data;
		int iRet = 0;
		MP3->GetStream()->SetFile(m_PlayList->GetCurrentFileName());
		iRet = m_PlayList->GetCurrentSong(&data);
		if (0 == iRet)
		{
			UI->OnNewSongData(&data);
		}
	}

	
};

/** main */
int main(int argc, char **argv) 
{
	myPSPApp *PSPApp  = new myPSPApp();
	PSPApp->Setup(argc, argv);
	PSPApp->Run();
	
	delete(PSPApp);
	
	return 0;
}

