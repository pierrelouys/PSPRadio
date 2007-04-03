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
#define PLUGIN_TYPE 0
#include "PSPRadio_Exports.h"
#include "PSPRadio.h"
#include "VisualizerInterface/vis_if.h"
#include <pspgu.h>

_button_mappings_struct_ PSPRadioButtonMap;
CScreen *rootScreen;
DeviceBuffer *g_PCMBuffer = NULL;

int CPSPRadio::Main(int argc, char **argv)
{
	Setup(argc, argv);

	Log(LOG_VERYLOW, "Main(): this=%p", this);
	Log(LOG_INFO, "PSPRadio() Main, Calling ProcessEvents()");
	ProcessEvents();

	return 0;
}

/** Setup */
int CPSPRadio::Setup(int argc, char **argv)
{
	/** open config file */
	char strAppTitle[140];

	m_UIPluginData = NULL;

	m_VisPluginData = NULL;
	m_VisPluginConfig.sc_width = 480;
	m_VisPluginConfig.sc_height = 272;
	m_VisPluginConfig.sc_pitch = 512;
	m_VisPluginConfig.sc_pixel_format = PSP_DISPLAY_PIXEL_FORMAT_8888;
	m_VisPluginConfig.x1 = 0;
	m_VisPluginConfig.y1 = 0;
	m_VisPluginConfig.x2 = 480;
	m_VisPluginConfig.y2 = 272;
	//m_VisPluginConfig.pcm_right_shift = 8;

	m_VisPluginGuFunctions.sceGuEnable = sceGuEnable;
	m_VisPluginGuFunctions.sceGuGetMemory = sceGuGetMemory;
	m_VisPluginGuFunctions.sceGuColor = sceGuColor;
	m_VisPluginGuFunctions.sceGuDrawArray = sceGuDrawArray;
	m_VisPluginGuFunctions.sceGuDisable = sceGuDisable;


	m_PowerEventData.bEnableNetworkAfterResume = false;
	m_PowerEventData.bPauseAfterResume = false;
	m_PowerEventData.bPlayAfterResume = false;
	m_PowerEventData.iCurrentStreamPosition = 0;

	sprintf(strAppTitle, "%s", GetProgramVersion());

	m_strCWD = (char*)malloc(MAXPATHLEN);
	if (!m_strCWD)
		return -1;

	getcwd(m_strCWD, MAXPATHLEN);

	Setup_OpenConfigFile(m_strCWD);

	Setup_Logging(m_strCWD);

	Log(LOG_VERYLOW, "Main(): this=%p", this);

	Setup_ButtonMapping(m_Config);
	
	do_fft_init();

	for (int i = 0; i < argc ; i++)
	{
		if (i == 3)
		{
			Log(LOG_ALWAYS, "TEXT_ADDR: PSPRadio.prx: Main(Arg %d)='%s'", i, argv[i]); /** Log text address for debugging */
		}
		else
		{
			Log(LOG_LOWLEVEL, "Main(Arg %d)='%s'", i, argv[i]);
		}
	}

	for (int i = 0 ; i < NUMBER_OF_PLUGIN_TYPES; i++)
	{
		m_ModuleLoader[i] = NULL;
		m_ModuleLoader[i] = new CPRXLoader();
		if (m_ModuleLoader[i] == NULL)
		{
			Log(LOG_ERROR, "Memory error - Unable to create CPRXLoader #%d.", i);
		}
		else
		{
			m_ModuleLoader[i]->SetName(PLUGIN_OFF_STRING);
		}
	}

	Setup_Sound();

	if (-1 != m_Config->GetInteger("SYSTEM:MAIN_THREAD_PRIO"))
	{

		Log(LOG_INFO, "Setting MAIN_THREAD_PRIO to %d as specified in config file.",
			m_Config->GetInteger("SYSTEM:MAIN_THREAD_PRIO"));

		sceKernelChangeThreadPriority(sceKernelGetThreadId(), m_Config->GetInteger("SYSTEM:MAIN_THREAD_PRIO"));
	}

	if (m_Config->GetInteger("WIFI:USE_PROXY") == 1)
	{
		SetNetworkProxy(m_Config->GetStr("WIFI:PROXY"));
		Log(LOG_INFO, "Configured To use proxy '%s'", GetNetworkProxy());
	}

	CScreenHandler::Screen iInitialScreen = (CScreenHandler::Screen)m_Config->GetInteger("SYSTEM:INITIAL_SCREEN", 0);
	if (0 != iInitialScreen)
	{
		Log(LOG_INFO, "Initial screen set to %d in config file.", iInitialScreen);
	}
	m_ScreenHandler = new CScreenHandler(m_strCWD, m_Config, m_Sound, iInitialScreen);
	Setup_UI(m_strCWD);

	m_UI->SetTitle(strAppTitle);
	m_UI->DisplayMainCommands();


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

		pLogging->SetPSPApp(this);

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

int CPSPRadio::Setup_ButtonMapping(CIniParser *pConfig)
{
	if ( pConfig == NULL )
	{
		pConfig = m_Config;
	}

	if ( pConfig )
	{
		PSPRadioButtonMap.BTN_TAKE_SCREENSHOT = 
			pConfig->GetInteger("BUTTONS:BTN_TAKE_SCREENSHOT", PSP_CTRL_SELECT);
		PSPRadioButtonMap.BTN_OK = 
			pConfig->GetInteger("BUTTONS:BTN_OK", PSP_CTRL_CROSS);
		PSPRadioButtonMap.BTN_CANCEL = 
			pConfig->GetInteger("BUTTONS:BTN_CANCEL", PSP_CTRL_CIRCLE);
		PSPRadioButtonMap.BTN_STOP = 
			pConfig->GetInteger("BUTTONS:BTN_STOP", PSP_CTRL_SQUARE);
		PSPRadioButtonMap.BTN_OPTIONS = 
			pConfig->GetInteger("BUTTONS:BTN_OPTIONS", PSP_CTRL_START);
		PSPRadioButtonMap.BTN_OPTIONS_EXIT = 
			pConfig->GetInteger("BUTTONS:BTN_OPTIONS_EXIT", PSP_CTRL_START);
		PSPRadioButtonMap.BTN_CYCLE_SCREENS = 
			pConfig->GetInteger("BUTTONS:BTN_CYCLE_SCREENS", PSP_CTRL_TRIANGLE);
		PSPRadioButtonMap.BTN_CYCLE_SCREENS_BACK = 
			pConfig->GetInteger("BUTTONS:BTN_CYCLE_SCREENS_BACK", PSP_CTRL_TRIANGLE);
		PSPRadioButtonMap.BTN_BACK = 
			pConfig->GetInteger("BUTTONS:BTN_BACK", PSP_CTRL_UP);
		PSPRadioButtonMap.BTN_FWD = 
			pConfig->GetInteger("BUTTONS:BTN_FWD", PSP_CTRL_DOWN);
		PSPRadioButtonMap.BTN_PGDN = 
			pConfig->GetInteger("BUTTONS:BTN_PGDN", PSP_CTRL_RTRIGGER);
		PSPRadioButtonMap.BTN_PGUP = 
			pConfig->GetInteger("BUTTONS:BTN_PGUP", PSP_CTRL_LTRIGGER);
		PSPRadioButtonMap.BTN_OPT_NAMES_FWD = 
			pConfig->GetInteger("BUTTONS:BTN_OPT_NAMES_FWD", PSP_CTRL_DOWN);
		PSPRadioButtonMap.BTN_OPT_NAMES_BACK = 
			pConfig->GetInteger("BUTTONS:BTN_OPT_NAMES_BACK", PSP_CTRL_UP);
		PSPRadioButtonMap.BTN_OPT_OPTIONS_FWD = 
			pConfig->GetInteger("BUTTONS:BTN_OPT_OPTIONS_FWD", PSP_CTRL_RIGHT);
		PSPRadioButtonMap.BTN_OPT_OPTIONS_BACK = 
			pConfig->GetInteger("BUTTONS:BTN_OPT_OPTIONS_BACK", PSP_CTRL_LEFT);
		PSPRadioButtonMap.BTN_OPT_ACTIVATE = 
			pConfig->GetInteger("BUTTONS:BTN_OPT_ACTIVATE", PSP_CTRL_CROSS);
	}

	return 0;
}

int CPSPRadio::Setup_UI(char *strCurrentDir)
{
	char *strVisPlugin = m_Config->GetString("PLUGINS:DEFAULT_VISUALIZER", "");
	Log(LOG_LOWLEVEL, "Loading Default UI Module = %s, Skin = %s", 
		m_Config->GetString("PLUGINS:UI", DEFAULT_UI_MODULE),
		m_Config->GetString("PLUGINS:UI_SKIN", DEFAULT_SKIN));

	m_ScreenHandler->StartUI(m_Config->GetString("PLUGINS:UI", DEFAULT_UI_MODULE), 
									m_Config->GetString("PLUGINS:UI_SKIN", DEFAULT_SKIN));


	if (strlen(strVisPlugin) > 0 && strcmp(strVisPlugin, "Off" ) != 0)
	{
		Log(LOG_LOWLEVEL, "Loading Default Visualizer Plugin %s", strVisPlugin);
	
		LoadPlugin(strVisPlugin, PLUGIN_VIS);
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

	m_UI->OnScreenshot(CScreenHandler::PSPRADIO_SCREENSHOT_ACTIVE); /* Stop UI from rendering while we shutdown */

	if (m_ScreenHandler)
	{
		Log(LOG_VERYLOW, "Exiting. Preparing UI for shutdown");
		m_ScreenHandler->PrepareShutdown();
	}

	rootScreen->Clear(0);
	rootScreen->SetFrameBuffer(0);
	rootScreen->LoadBuffer(1, "Shutdown.png");
	rootScreen->SetFrameBuffer(1);
	

// 	Log(LOG_INFO, "OnExit(): Stopping network drivers");
// 	StopNetworkDrivers();

	Log(LOG_INFO, "Exiting. The end. -- Calling sceKernelExitGame");
	sceKernelExitGame();

#if 0 /** sceKernelExitGame() always works, so we call that, the OS will do the cleanup for us. */
	if (m_Sound)
	{
		Log(LOG_VERYLOW, "Exiting. Destroying m_Sound object");
		delete(m_Sound);
		m_Sound = NULL;
	}
	if (m_Config)
	{
		Log(LOG_VERYLOW, "Exiting. Destroying m_Config object");
		delete(m_Config), m_Config = NULL;
	}
	if (m_ScreenHandler)
	{
		Log(LOG_VERYLOW, "Exiting. Destroying m_ScreenHandler object");
		delete(m_ScreenHandler), m_ScreenHandler = NULL;
	}
	if (m_UI)
	{
		Log(LOG_LOWLEVEL, "Exiting. Calling UI->Terminate");
		m_UI->Terminate();
		Log(LOG_LOWLEVEL, "Exiting. Destroying UI object");
		delete(m_UI);
		m_UI = NULL;
		Log(LOG_LOWLEVEL, "Exiting. UI object deleted.");
	}

	for (int i = NUMBER_OF_PLUGIN_TYPES - 1 ; i >= 0 ; i--)
	{
		if (m_ModuleLoader[i])
		{
			Log(LOG_VERYLOW, "Exiting. Destroying m_ModuleLoader[%d] object. Name='%s'", i, m_ModuleLoader[i]->GetName());
			delete(m_ModuleLoader[i]), m_ModuleLoader[i] = NULL;
		}
	}

	if (m_strCWD)
	{
		Log(LOG_VERYLOW, "Freeing strDir");
		free(m_strCWD); m_strCWD = NULL;
	}

	do_fft_destroy();
	
	Log(LOG_VERYLOW, "Exiting. The end.");
#endif
}

int CPSPRadio::ProcessEvents()
{
	CPSPEventQ::QEvent event = { 0, 0, NULL };
	int rret = 0;
	IScreen *CurrentScreen = NULL; //(PlayListScreen *)m_ScreenHandler->GetCurrentScreen();
	IScreen *StreamOwnerScreen = NULL;

	Log(LOG_VERYLOW, "ProcessEvents(): Starts.");

	Log(LOG_VERYLOW, "ProcessEvents(): StartPolling()");
	StartPolling();

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

		/** Receive blocks until there is a message queued up */
		rret = m_EventToPSPApp->Receive(event);

		CurrentScreen 		= m_ScreenHandler->GetCurrentScreen();
		StreamOwnerScreen	= m_ScreenHandler->GetStreamOwnerScreen();

		if (MID_THPLAY_PCMBUFFER != event.EventId &&
			MID_BUFF_PERCENT_UPDATE != event.EventId)
		{
			/** Don't log buffer messages, or percent update messages, or we the queue backs up while playing */
			Log(LOG_VERYLOW, "*ProcessMessages()*::Receive Ret=%d. eventid=0x%08x.", rret, event.EventId);
		}

		if (event.SenderId == SID_PSPAPP)
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
		case SID_PSPRADIO:
			{
				switch (event.EventId)
				{
				/* User: 1- killed a plugin or 2-selected to switch to pspradio from a plugin : */
				case MID_GIVEUPEXCLISIVEACCESS:
					if (m_UI)
					{
						Log(LOG_INFO, "MID_GIVEUPEXCLISIVEACCESS: Calling OnScreenshot");
						m_UI->OnScreenshot(CScreenHandler::PSPRADIO_SCREENSHOT_NOT_ACTIVE);
					  	sceKernelDelayThread(100*1000); /* give UI time to react */
						/** Re-draw the current screen */
						Log(LOG_INFO, "MID_GIVEUPEXCLISIVEACCESS: Calling Activate");
						GetScreenHandler()->GetCurrentScreen()->Activate();
					  	sceKernelDelayThread(100*1000); /* give UI time to react */
					}
					Log(LOG_INFO, "MID_GIVEUPEXCLISIVEACCESS: Calling StarKeyLatch");
					StartKeyLatch();
					Log(LOG_INFO, "MID_GIVEUPEXCLISIVEACCESS: Done.");
					break;
				case MID_PLUGINEXITED:
					plugin_type type = (plugin_type)((int)event.pData); /** Passed by value */
					Log(LOG_INFO, "Plugin Exited. Unloading. type = %d", type);
					UnloadPlugin(type);
					break;
				}
			}
			break;

		case SID_SCREENHANDLER:
			switch(event.EventId)
			{
			case EID_NEW_UI_POINTER:
				m_UI = (IPSPRadio_UI *)event.pData;
				Log(LOG_LOWLEVEL, "Received new UI address = %p", m_UI );
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
			case MID_POWER_EVENT_RESUME_COMPLETE:
				OnPowerEventResumeComplete();
				break;

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
					Log(LOG_LOWLEVEL, "Wanted to call OnPlayStateChange(STOP), but the stream has no owner.");
				}
				break;

			case MID_DECODE_STREAM_OPEN_ERROR:
				Log(LOG_VERYLOW, "MID_DECODE_STREAM_OPEN_ERROR received, calling OnPlayStateChange(STOP)");
				m_Sound->Stop(); /* this sends a MID_SOUND_STOPPED */
				if (m_UI)
					m_UI->OnStreamOpeningError();
				/*
				if (StreamOwnerScreen)
				{
					StreamOwnerScreen->OnPlayStateChange(PLAYSTATE_STOP);
				}
				else
				{
					Log(LOG_ERROR, "Wanted to call OnPlayStateChange(STOP), but the stream has no owner.");
				}
				*/
				break;

			case MID_THDECODE_DECODING:
				Log(LOG_VERYLOW, "MID_THDECODE_DECODING received, calling OnPlayStateChange(PLAY)");
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
				g_PCMBuffer = (DeviceBuffer *)event.pData;
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
				if (m_UI)
					m_UI->OnButtonReleased((int)(event.pData));
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

			case MID_KEY_LATCH_ENABLED_WITH_KEY_COMBO: /* User used key combo to kill plugin */
				/** Unload the bad plugin */
				UnloadPlugin(m_ExclusiveAccessPluginType);
				PSPRadioExport_GiveUpExclusiveAccess();
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

int CPSPRadio::OnPowerEvent(int pwrflags)
{
	/* check for power switch and suspending as one is manual and the other automatic */
	Log(LOG_INFO, "OnPowerEvent() flags: 0x%08X", pwrflags);

	if (pwrflags & PSP_POWER_CB_POWER_SWITCH || pwrflags & PSP_POWER_CB_SUSPENDING)
	{
		Log(LOG_INFO, "OnPowerEvent: Suspending");
		if (CPSPSound::PLAY == m_Sound->GetPlayState())
		{
			m_PowerEventData.bPlayAfterResume = true;
			m_PowerEventData.iCurrentStreamPosition = m_Sound->GetCurrentStream()->GetBytePosition();
			Log(LOG_LOWLEVEL, "OnPowerEvent: Suspending: Was playing (pos=%d), so stopping...", m_PowerEventData.iCurrentStreamPosition);
			m_Sound->Stop();
		}
		
		if (CPSPSound::PAUSE == m_Sound->GetPlayState())
		{
			m_PowerEventData.bPauseAfterResume = true;
			m_PowerEventData.iCurrentStreamPosition = m_Sound->GetCurrentStream()->GetBytePosition();
			Log(LOG_LOWLEVEL, "OnPowerEvent: Suspending: Was paused (pos=%d), so stopping...", m_PowerEventData.iCurrentStreamPosition);
			m_Sound->Stop();
		}
		
		if (IsNetworkEnabled())
		{
			m_PowerEventData.bEnableNetworkAfterResume = true;
			Log(LOG_LOWLEVEL, "OnPowerEvent: Suspending: Network was enabled, disabling...");
			DisableNetwork();
		}
	}
	if (pwrflags & PSP_POWER_CB_RESUMING)
	{
		Log(LOG_INFO, "OnPowerEvent: Resuming from suspend mode");
	}
	if (pwrflags & PSP_POWER_CB_STANDBY)
	{
		Log(LOG_INFO, "OnPowerEvent: Entering Standby mode");
	}

	Log(LOG_VERYLOW, "OnPowerEvent() End.");

	return 0;

}

int CPSPRadio::OnPowerEventResumeComplete()
{
	Log(LOG_INFO, "OnPowerEvent: Resume Complete");
	if (m_PowerEventData.bEnableNetworkAfterResume)
	{
		m_PowerEventData.bEnableNetworkAfterResume = false;
		Log(LOG_LOWLEVEL, "OnPowerEvent: Resume Complete: Network was enabled, re-enabling...");
		((OptionsScreen *) m_ScreenHandler->GetScreen(CScreenHandler::PSPRADIO_SCREEN_OPTIONS))->Start_Network();
		//SendEvent(MID_NETWORKRECONNECT, NULL, SID_PSPRADIO);
	}
	
	if (m_PowerEventData.bPlayAfterResume || m_PowerEventData.bPauseAfterResume)
	{
		if (m_Sound->GetCurrentStream()->GetType() == CPSPStream::STREAM_TYPE_FILE)
		{
			Log(LOG_LOWLEVEL, "OnPowerEvent: Resume Complete: Was playing, so continuing (pos=%d)...", m_PowerEventData.iCurrentStreamPosition);
			m_Sound->Play();
			m_Sound->Seek(m_PowerEventData.iCurrentStreamPosition);
			if (m_PowerEventData.bPauseAfterResume)
				m_Sound->Pause();
		}
		else
		{
			//Log(LOG_LOWLEVEL, "OnPowerEvent: Resume Complete: Was playing, but it was an online stream. Let the user restart it.");
			m_Sound->Play();
			if (m_PowerEventData.bPauseAfterResume)
				m_Sound->Pause();
		}
		m_PowerEventData.bPlayAfterResume = false;
		m_PowerEventData.bPauseAfterResume = false;
	}
	return 0;
}

#include <png.h>
#include <pspdisplay.h>


void CPSPRadio::TakeScreenShot()
{
	char	path[MAXPATHLEN];
	char	*filename;

	sprintf(path, "%s/Screenshots/", m_strCWD);

	filename = ScreenshotName(path);

// 	if (m_UI)
// 	{
// 		m_UI->OnScreenshot(PSPRADIO_SCREENSHOT_ACTIVE);
// 	}

	if  (filename)
	{
		ScreenshotStore(filename);
		Log(LOG_INFO, "Screenshot stored as : %s", filename);
		free(filename);
	}
	else
	{
		Log(LOG_INFO, "No screenshot taken..");
	}

/*	if (m_UI)
	{
		m_UI->OnScreenshot(PSPRADIO_SCREENSHOT_NOT_ACTIVE);
	}*/
}

char *CPSPRadio::ScreenshotName(char *path)
{
	char	*filename;
	int		image_number;
	FILE	*temp_handle;

	filename = (char *) malloc(MAXPATHLEN);
	if (filename)
	{
		for (image_number = 0 ; image_number < 1000 ; image_number++)
		{
			sprintf(filename, "%sPSPRadio_Screen%03d.png", path, image_number);
			temp_handle = fopen(filename, "r");
			// If the file didn't exist we can use this current filename for the screenshot
			if (!temp_handle)
			{
				break;
			}
			fclose(temp_handle);
		}
	}
	return filename;
}

//The code below is take from an example for libpng.
void CPSPRadio::ScreenshotStore(char *filename)
{
	u32* vram32;
	u16* vram16;
	int bufferwidth;
	int pixelformat;
	int unknown;
	int i, x, y;
	png_structp png_ptr;
	png_infop info_ptr;
	FILE* fp;
	u8* line;
	fp = fopen(filename, "wb");
	if (!fp) return;
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) return;
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		fclose(fp);
		return;
	}
	png_init_io(png_ptr, fp);
	png_set_IHDR(png_ptr, info_ptr, SCREEN_WIDTH, SCREEN_HEIGHT,
		8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	line = (u8*) malloc(SCREEN_WIDTH * 3);
	sceDisplayWaitVblankStart();  // if framebuf was set with PSP_DISPLAY_SETBUF_NEXTFRAME, wait until it is changed
	sceDisplayGetFrameBuf((void**)&vram32, &bufferwidth, &pixelformat, &unknown);
	vram16 = (u16*) vram32;
	for (y = 0; y < SCREEN_HEIGHT; y++) {
		for (i = 0, x = 0; x < SCREEN_WIDTH; x++) {
			u32 color = 0;
			u8 r = 0, g = 0, b = 0;
			switch (pixelformat) {
				case PSP_DISPLAY_PIXEL_FORMAT_565:
					color = vram16[x + y * bufferwidth];
					r = (color & 0x1f) << 3;
					g = ((color >> 5) & 0x3f) << 2 ;
					b = ((color >> 11) & 0x1f) << 3 ;
					break;
				case PSP_DISPLAY_PIXEL_FORMAT_5551:
					color = vram16[x + y * bufferwidth];
					r = (color & 0x1f) << 3;
					g = ((color >> 5) & 0x1f) << 3 ;
					b = ((color >> 10) & 0x1f) << 3 ;
					break;
				case PSP_DISPLAY_PIXEL_FORMAT_4444:
					color = vram16[x + y * bufferwidth];
					r = (color & 0xf) << 4;
					g = ((color >> 4) & 0xf) << 4 ;
					b = ((color >> 8) & 0xf) << 4 ;
					break;
				case PSP_DISPLAY_PIXEL_FORMAT_8888:
					color = vram32[x + y * bufferwidth];
					r = color & 0xff;
					g = (color >> 8) & 0xff;
					b = (color >> 16) & 0xff;
					break;
			}
			line[i++] = r;
			line[i++] = g;
			line[i++] = b;
		}
		png_write_row(png_ptr, line);
	}
	free(line);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	fclose(fp);
}

#ifdef DYNAMIC_BUILD
	#include <FSS_Exports.h>
	#include <APP_Exports.h>
	#include <GAME_Exports.h>
	#include <VIS_Plugin.h>

	int CPSPRadio::LoadPlugin(const char *strPlugin, plugin_type type)
	{
		char strModulePath[MAXPATHLEN+1];
		char cwd[MAXPATHLEN+1];
		//char strPluginVersion[64];

		if (type < NUMBER_OF_PLUGIN_TYPES)
		{
			if (m_ModuleLoader[type]->IsLoaded() == true)
			{
				Log(LOG_INFO, "LoadPlugin(): Unloading currently running plugin");
				UnloadPlugin(type);
			}

			sprintf(strModulePath, "%s/%s", getcwd(cwd, MAXPATHLEN), strPlugin);

			int id = m_ModuleLoader[type]->Load(strModulePath);

			if (m_ModuleLoader[type]->IsLoaded() == true)
			{
				m_ModuleLoader[type]->SetName(strPlugin);

				SceKernelModuleInfo modinfo;
				memset(&modinfo, 0, sizeof(modinfo));
				modinfo.size = sizeof(modinfo);
				sceKernelQueryModuleInfo(id, &modinfo);
				Log(LOG_ALWAYS, "TEXT_ADDR: '%s' Loaded at text_addr=0x%x",
					strPlugin, modinfo.text_addr);

				int iRet = m_ModuleLoader[type]->Start();

				Log(LOG_INFO, "Module start returned: 0x%x", iRet);

				switch(type)
				{
					case PLUGIN_UI:
						if (get_uiplugin_info != NULL)
							m_UIPluginData = get_uiplugin_info();
						if (m_UIPluginData && m_UIPluginData->interface_version == PLUGIN_UI_VERSION)
						{
							m_UIPluginData->PSPRadioObject = this;
							Log(LOG_INFO, "Module description: '%s' Interface Version: %d", 
								m_UIPluginData->description,
								m_UIPluginData->interface_version);
							//m_VisPluginData->init();
						    m_UI = (IPSPRadio_UI*)m_UIPluginData->create_ui_object();
						}
						else
						{
							Log(LOG_ERROR, "Unable to load plugin '%s'. This plugin supports i/f version %d. This version of PSPRadio uses version %d.", 
								m_UIPluginData->filename,
								m_UIPluginData->interface_version,
								PLUGIN_UI_VERSION);
							m_UIPluginData = NULL;
							m_ModuleLoader[type]->Unload();
							m_ModuleLoader[type]->SetName(PLUGIN_OFF_STRING);
							return -2;
						}
						break;
					case PLUGIN_FSS:
						ModuleStartFSS();
						break;
					case PLUGIN_APP:
						ModuleStartAPP();
						break;
					case PLUGIN_GAME:
						ModuleStartGAME();
						break;
					case PLUGIN_VIS:
						if (get_vplugin_info != NULL)
							m_VisPluginData = get_vplugin_info();
						if (m_VisPluginData)
						{
							m_VisPluginData->config = &m_VisPluginConfig;
							m_VisPluginData->gu = &m_VisPluginGuFunctions;
						}
						if (m_VisPluginData && m_VisPluginData->interface_version == PLUGIN_VIS_VERSION)
						{
							Log(LOG_INFO, "Module description: '%s' Interface Version: %d", 
								m_VisPluginData->description,
								m_VisPluginData->interface_version);
							m_VisPluginData->init();
						}
						else
						{
							Log(LOG_ERROR, "Unable to load plugin '%s'. This plugin supports i/f version %d. This version of PSPRadio uses version %d.", 
								m_ModuleLoader[type]->GetFilename(),
								m_VisPluginData->interface_version,
								PLUGIN_VIS_VERSION);
							m_VisPluginData = NULL;
							m_ModuleLoader[type]->Unload();
							m_ModuleLoader[type]->SetName(PLUGIN_OFF_STRING);
							return -2;
						}
						break;
					case NUMBER_OF_PLUGIN_TYPES: 
					case PLUGIN_NA:
					default:
						Log(LOG_INFO, "LoadPlugin: Unknown plugin type: ", (int)type);
				}

				return 0;

			}
			else
			{
				Log(LOG_ERROR, "Error loading '%s' Module. Error=0x%x", strModulePath, m_ModuleLoader[type]->GetError());
				return -1;
			}
		}
		else if (type == (plugin_type)-1)
		{
			Log(LOG_ERROR, "Unable to Load/Unload plugin, as it was not set.");
			return -1;
		}
		else /* type >= NUMBER_OF_PLUGINS */
		{
			Log(LOG_ERROR, "Wrong type %d used to load plugin.", type);
			return -1;
		}
	}

	int CPSPRadio::UnloadPlugin(plugin_type type)
	{
		//char strModulePath[MAXPATHLEN+1];
		//char cwd[MAXPATHLEN+1];

		if (type < NUMBER_OF_PLUGIN_TYPES)
		{
			switch(type)
			{
				case PLUGIN_UI:
				{
					Log(LOG_INFO, "UnloadPlugin(): Destroying current UI");
					m_UI->Terminate();
					sceKernelDelayThread(100*1000); /* give UI time to terminate */
					delete(m_UI), m_UI = NULL;
					m_UIPluginData = NULL;
				}
				break;
				case PLUGIN_VIS:
				{
					/* If plugin has cleanup() defined, then call it before unloading */
					Log(LOG_INFO, "UnloadPlugin(): Destroying current Visual Plugin");
					if (m_VisPluginData->cleanup != NULL)
					{
						m_VisPluginData->cleanup();
						sceKernelDelayThread(100*1000); /* give plugin time to terminate */
					}
					m_VisPluginData = NULL;
				}
				break;
				default:
				break;
			}
			Log(LOG_INFO, "UnloadPlugin(): Unloading PRX");
			m_ModuleLoader[type]->Unload();
			m_ModuleLoader[type]->SetName(PLUGIN_OFF_STRING);
		}
		return 0;
	}
	

	char *CPSPRadio::GetActivePluginName(plugin_type type)
	{
		return m_ModuleLoader[type]->GetName();
	}
#endif
