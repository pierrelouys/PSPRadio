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
#include <psprtc.h>
#include "ScreenHandler.h"
#include "PlayListScreen.h"
#include "SHOUTcastScreen.h"
#include "TextUI.h"
#include "TextUI3D.h"
#include <ivorbisfile.h>
#include "Screen.h"
#include "PSPRadio.h"

CScreen rootScreen;

/** Setup */
int CPSPRadio::Setup(int argc, char **argv)
{
	/** open config file */
	char *strDir = NULL;
	char strAppTitle[140];

	sprintf(strAppTitle, "%s",
		GetProgramVersion());

	strDir = (char*)malloc(MAXPATHLEN);
	if (!strDir)
		return -1;

	getcwd(strDir, MAXPATHLEN);

	Setup_OpenConfigFile(strDir);

	Setup_Logging(strDir);

	Setup_Sound();

	if (-1 != m_Config->GetInteger("SYSTEM:MAIN_THREAD_PRIO"))
	{

		Log(LOG_INFO, "Setting MAIN_THREAD_PRIO to %d as specified in config file.",
			m_Config->GetInteger("SYSTEM:MAIN_THREAD_PRIO"));

		sceKernelChangeThreadPriority(sceKernelGetThreadId(), m_Config->GetInteger("SYSTEM:MAIN_THREAD_PRIO"));
	}

	CScreenHandler::Screen iInitialScreen = (CScreenHandler::Screen)m_Config->GetInteger("SYSTEM:INITIAL_SCREEN", 0);
	if (0 != iInitialScreen)
	{
		Log(LOG_INFO, "Initial screen set to %d in config file.", iInitialScreen);
	}
	m_ScreenHandler = new CScreenHandler(strDir, m_Config, m_Sound, iInitialScreen);
	Setup_UI(strDir);

	m_UI->SetTitle(strAppTitle);
	m_UI->DisplayMainCommands();

	Log(LOG_VERYLOW, "Freeing strDir");
	free(strDir);

	Log(LOG_VERYLOW, "StartPolling()");
	StartPolling();

	Log(LOG_LOWLEVEL, "Exiting Setup()");

	return 0;
}

int CPSPRadio::Setup_OpenConfigFile(char *strCurrentDir)
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

int CPSPRadio::Setup_Logging(char *strCurrentDir)
{
	char *strFilename = NULL;
	if ( m_Config )
	{
		int iLoglevel = m_Config->GetInteger("DEBUGGING:LOGLEVEL", 100);

		InstantiateLogging();

		strFilename = (char*)malloc(strlen(strCurrentDir) + 1 + strlen(m_Config->GetStr("DEBUGGING:LOGFILE")) + 10);
		sprintf(strFilename, "%s/%s", strCurrentDir, m_Config->GetStr("DEBUGGING:LOGFILE"));
		/** Set Logging Global Object to use the configured logfile and loglevels */
		pLogging->Set(strFilename, (loglevel_enum)iLoglevel);
		free(strFilename), strFilename = NULL;

		Log(LOG_ALWAYS, "--------------------------------------------------------");
		Log(LOG_ALWAYS, "%s Version %s Starting - Using Loglevel %d", GetProgramName(), GetProgramVersion(),
			iLoglevel);
	}

	return 0;
}

int CPSPRadio::Setup_UI(char *strCurrentDir)
{
	Log(LOG_LOWLEVEL, "UI Mode = %s", m_Config->GetStr("UI:MODE"));

	#ifdef GRAPHICS_UI
	if (0 == strcmp(m_Config->GetStr("UI:MODE"), "Graphics"))
	{
		m_UI = m_ScreenHandler->StartUI(CScreenHandler::UI_GRAPHICS);
	}
	#endif
	if (0 == strcmp(m_Config->GetStr("UI:MODE"), "3D"))
	{
		m_UI = m_ScreenHandler->StartUI(CScreenHandler::UI_3D);
	}
	else
	{
		m_UI = m_ScreenHandler->StartUI(CScreenHandler::UI_TEXT);
	}

	return 0;
}

int CPSPRadio::Setup_Sound()
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

void CPSPRadio::OnExit()
{
	Log(LOG_VERYLOW, "PSPRadio::OnExit()");

	if (m_ScreenHandler)
	{
		Log(LOG_VERYLOW, "Exiting. Preparing UI for shutdown");
		m_ScreenHandler->PrepareShutdown();
	}

	rootScreen.SetBackgroundImage("Shutdown.png");
	rootScreen.Clear();

	if (m_Sound)
	{
		Log(LOG_VERYLOW, "Exiting. Destroying m_Sound object");
		delete(m_Sound);
		m_Sound = NULL;
	}
	if (m_Config)
	{
		Log(LOG_VERYLOW, "Exiting. Destroying m_Config object");
		delete(m_Config);
	}
	if (m_ScreenHandler)
	{
		Log(LOG_VERYLOW, "Exiting. Destroying m_ScreenHandler object");
		delete(m_ScreenHandler);
	}

	Log(LOG_VERYLOW, "Exiting. The end.");
}

int CPSPRadio::ProcessEvents()
{
	CPSPEventQ::QEvent event = { 0, 0, NULL };
	int rret = 0;
	IScreen *CurrentScreen = NULL; //(PlayListScreen *)m_ScreenHandler->GetCurrentScreen();
	IScreen *StreamOwnerScreen = NULL;

	for (;;)
	{
		//Log(LOG_VERYLOW, "ProcessMessages()::Calling Receive. %d Messages in Queue", m_EventToPSPApp->Size());
		if ( m_EventToPSPApp->Size() > 100 )
		{
			Log(LOG_ERROR, "ProcessEvents(): Too many events backed-up!: %d. Exiting!", m_EventToPSPApp->Size());
			if (m_UI)
				m_UI->DisplayErrorMessage("Event Queue Backed-up, Exiting!");
			OnExit();
			/** Calling return causes the app to terminate */
			return 0;
		}

		rret = m_EventToPSPApp->Receive(event);


		CurrentScreen 		= m_ScreenHandler->GetCurrentScreen();
		StreamOwnerScreen	= m_ScreenHandler->GetStreamOwnerScreen();

		if (MID_THPLAY_PCMBUFFER != event.EventId &&
			MID_BUFF_PERCENT_UPDATE != event.EventId)
		{
			/** Don't log buffer messages, or percent update messages, or we the queue backs up while playing */
			Log(LOG_VERYLOW, "*ProcessMessages()*::Receive Ret=%d. eventid=0x%08x.", rret, event.EventId);
		}
		if (SID_PSPAPP == event.SenderId)
		{
			switch (event.EventId)
			{
			case MID_PSPAPP_EXITING:
				if ( (SID_PSPAPP == event.SenderId) )
				{
					Log(LOG_INFO, "ProcessEvents(): MID_PSPAPP_EXITING received.");
					OnExit();
					/** Calling return causes the app to terminate */
					return 0;
				}
				break;

			case MID_ERROR:
				if (m_UI)
					m_UI->DisplayErrorMessage((char*)event.pData);
				Log(LOG_ERROR, (char*)event.pData);
				free(event.pData);
				continue;
				break;
			}
		}

		switch (event.SenderId)
		{
		//case PSPAPP_SENDER_ID:
		//	switch(event.EventId)
		//	[
		//	case ERROR
		//case SID_PSPSOUND:
		case SID_SCREENHANDLER:
			switch(event.EventId)
			{
			case EID_NEW_UI_POINTER:
				m_UI = (IPSPRadio_UI *)event.pData;
				Log(LOG_LOWLEVEL, "Received new UI address = 0x%x", m_UI );
				break;
			case EID_EXIT_SELECTED:
					Log(LOG_INFO, "ProcessEvents(): EID_EXIT_SELECTED received.");
					OnExit();
					/** Calling return causes the app to terminate */
					return 0;
			}
			break;

		default:
			//Log(LOG_VERYLOW, "OnMessage: Message: MID=0x%x SID=0x%x", event.EventId, event.SenderId);
			switch(event.EventId)
			{
			case MID_BUFF_PERCENT_UPDATE:
				if (CPSPSound::PLAY == m_Sound->GetPlayState())
				{
					if (m_UI)
						m_UI->DisplayBufferPercentage(m_Sound->GetBufferFillPercentage());
				}
				break;

			case MID_DECODE_STREAM_OPENING:
				if (m_UI)
					m_UI->OnStreamOpening();
				break;

			case MID_SOUND_STOPPED:
				Log(LOG_VERYLOW, "MID_SOUND_STOPPED received, calling OnPlayStateChange(STOP)");
				if (StreamOwnerScreen)
				{
					StreamOwnerScreen->OnPlayStateChange(PLAYSTATE_STOP);
				}
				else
				{
					Log(LOG_ERROR, "Wanted to call OnPlayStateChange(STOP), but the stream has no owner.");
				}
				break;

			case MID_DECODE_STREAM_OPEN_ERROR:
				Log(LOG_VERYLOW, "MID_DECODE_STREAM_OPEN_ERROR received, calling OnPlayStateChange(STOP)");
				if (StreamOwnerScreen)
				{
					StreamOwnerScreen->OnPlayStateChange(PLAYSTATE_STOP);
				}
				else
				{
					Log(LOG_ERROR, "Wanted to call OnPlayStateChange(STOP), but the stream has no owner.");
				}
				if (m_UI)
					m_UI->OnStreamOpeningError();
				break;

			case MID_THDECODE_DECODING:
				Log(LOG_VERYLOW, "MID_THDECODE_DECODING received, calling OnPlayStateChange(PLAY)", event.EventId);
				if (StreamOwnerScreen)
				{
					StreamOwnerScreen->OnPlayStateChange(PLAYSTATE_PLAY);
				}
				else
				{
					Log(LOG_ERROR, "Wanted to call OnPlayStateChange(PLAY), but the stream has no owner.");
				}
				//if (m_UI)
				//	m_UI->OnStreamOpeningSuccess();
				break;

			case MID_THDECODE_EOS:
				Log(LOG_VERYLOW, "MID_THPLAY_EOS received, calling EOSHandler()");
				if (StreamOwnerScreen)
				{
					StreamOwnerScreen->EOSHandler();//OnPlayStateChange(PLAYSTATE_EOS);
				}
				else
				{
					Log(LOG_ERROR, "Wanted to call EOSHandler(), but the stream has no owner.");
				}
				break;

			case MID_THPLAY_PCMBUFFER:
				if (m_UI)
					m_UI->NewPCMBuffer((short *)event.pData);
				break;


			case MID_NEW_METADATA_AVAILABLE:
				if (m_UI)
					m_UI->OnNewSongData(m_Sound->GetCurrentStream()->GetMetaData());
				break;
			
			case MID_STREAM_TIME_UPDATED:
				if (m_UI)
					m_UI->OnStreamTimeUpdate(m_Sound->GetCurrentStream()->GetMetaData());
				break;

			case MID_TCP_CONNECTING_PROGRESS:
				if (m_UI)
					m_UI->OnConnectionProgress();
				break;

			case MID_ONBUTTON_PRESSED:
				Log(LOG_VERYLOW, "On button pressed received. data = 0x%x", ((int)event.pData));
				break;

			case MID_ONBUTTON_RELEASED:
				Log(LOG_VERYLOW, "On button released received. data = 0x%x", ((int)event.pData));
				m_ScreenHandler->CommonInputHandler((int)(event.pData), MID_ONBUTTON_RELEASED);
				break;
				
			case MID_ONBUTTON_REPEAT:
				Log(LOG_VERYLOW, "On button repeat received. data = 0x%x", ((int)event.pData));
				m_ScreenHandler->CommonInputHandler((int)(event.pData), MID_ONBUTTON_REPEAT);
				break;

			case MID_ONBUTTON_LONG_PRESS:
				Log(LOG_VERYLOW, "On button long press received. data = 0x%x", ((int)event.pData));
				m_ScreenHandler->CommonInputHandler((int)(event.pData), MID_ONBUTTON_LONG_PRESS);
				break;

			case MID_ONHPRM_RELEASED:
				m_ScreenHandler->OnHPRMReleased((u32)(event.pData));
				break;

			case MID_ONBATTERY_CHANGE:
				if (m_UI)
					m_UI->OnBatteryChange(*((int*)event.pData));
				break;

			case MID_ONTIME_CHANGE:
				{
				pspTime	*localTime = (pspTime *)event.pData;

				if (m_UI)
					m_UI->OnTimeChange(localTime);
				}
				break;

			case MID_USB_ENABLE:
				if (m_UI)
					m_UI->OnUSBEnable();
				break;

			case MID_USB_DISABLE:
				if (m_UI)
					m_UI->OnUSBDisable();
				break;

			/** This is not used, vblank notification is done via 'callback' of OnVBlank from PSPApp */
			case MID_ONVBLANK:
				if (m_UI)
					m_UI->OnVBlank();
				break;

			default:
				Log(LOG_VERYLOW, "ProcessEvents: Unhandled event: MID=0x%x SID=0x%x",
					event.EventId, event.SenderId);
				break;
			}
		}
	}
	return 0;
}

void CPSPRadio::OnVBlank()
{
	m_ScreenHandler->OnVBlank();
}

int CPSPRadio::OnPowerEvent(int pwrflags)
{
	/* check for power switch and suspending as one is manual and the other automatic */
	Log(LOG_INFO, "OnPowerEvent() flags: 0x%08X", pwrflags);
	
	if (pwrflags & PSP_POWER_CB_POWER_SWITCH || pwrflags & PSP_POWER_CB_SUSPENDING) 
	{
		Log(LOG_INFO, "OnPowerEvent: Suspending");
	}
	if (pwrflags & PSP_POWER_CB_RESUMING)
	{
		Log(LOG_INFO, "OnPowerEvent: Resuming from suspend mode");
	}
	if (pwrflags & PSP_POWER_CB_RESUME_COMPLETE)
	{
		Log(LOG_INFO, "OnPowerEvent: Resume Complete");

		bool bWasPlaying = false;
		if (CPSPSound::PLAY == m_Sound->GetPlayState())
		{
			Log(LOG_LOWLEVEL, "OnPowerEvent: Was playing before, so stopping...");
			m_Sound->Stop();
			bWasPlaying = true;
		}
		if (IsNetworkEnabled())
		{
			Log(LOG_LOWLEVEL, "OnPowerEvent: Network was enabled, disabling...");
			DisableNetwork();
			#if 0 //test
			((OptionsScreen *) m_ScreenHandler->GetScreen(CScreenHandler::PSPRADIO_SCREEN_OPTIONS))->Start_Network();

			if(true == IsNetworkEnabled() && true == bWasPlaying)
			{
				m_Sound->Play();
			}
			#endif
		}
		else
		{
			#if 0 //test
			if (true == bWasPlaying)
			{
				m_Sound->Play();
			}
			#endif
		}
	}
	if (pwrflags & PSP_POWER_CB_STANDBY)
	{
		Log(LOG_INFO, "OnPowerEvent: Entering Standby mode");
	}
	
	Log(LOG_VERYLOW, "OnPowerEvent() End.");
	
	return 0;
}
