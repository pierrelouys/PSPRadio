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
#include <UI_Interface.h>
#include "PlayListScreen.h"
#include <PSPRadio.h>

PlayListScreen::PlayListScreen(int Id, CScreenHandler *ScreenHandler): IScreen(Id, ScreenHandler)
{
	Log(LOG_VERYLOW,"PlayListScreen Ctor.");
	m_Lists = NULL;

	//m_Lists = new CMetaDataContainer();

	//LoadLists();
}

PlayListScreen::~PlayListScreen()
{
	if (m_Lists)
	{
		delete(m_Lists), m_Lists = NULL;
	}
}

int PlayListScreen::LoadLists()
{
	if (!m_Lists)
	{
		m_Lists = new CMetaDataContainer();
	}
	
	if (m_Lists)
	{
		Log(LOG_LOWLEVEL, "Loading playlists");
		m_Lists->Clear();
		
		char *strFileName = NULL;
		strFileName = (char *)malloc(strlen(m_ScreenHandler->GetCWD()) + strlen("PlayLists") + 10);
		sprintf(strFileName, "%s/PlayLists", m_ScreenHandler->GetCWD());
		m_Lists->LoadPlaylistsFromDirectory(strFileName); //**//

		m_Lists->SetCurrentSide(CMetaDataContainer::CONTAINER_SIDE_CONTAINERS);

	}

	return 0;
}

void PlayListScreen::Activate(IPSPRadio_UI *UI)
{
	Log(LOG_VERYLOW, "Activate(): Start");
	
	IScreen::Activate(UI);

	if (m_Lists)
	{
		if (false == m_Lists->GetContainerList()->empty())
		{
			Log(LOG_VERYLOW, "Activate(): Calling DisplayContainers");
			m_UI->DisplayContainers(m_Lists);
	
			//Log(LOG_VERYLOW, "Activate(): 1");
			if ( (m_Lists->GetElementList()) &&
				(false == m_Lists->GetElementList()->empty()) )
			{
				Log(LOG_VERYLOW, "Activate(): Calling DisplayElements");
				m_UI->DisplayElements(m_Lists);
			}
		}
		//Log(LOG_VERYLOW, "Activate(): 2");
		m_UI->OnCurrentContainerSideChange(m_Lists); 
	}


	if (CPSPSound::PLAY == m_ScreenHandler->GetSound()->GetPlayState())
	{
		/** Populate m_CurrentMetaData */
		//don't until user starts it!
		//m_CurrentPlayList->GetCurrentSong(m_CurrentMetaData);
		m_UI->OnNewSongData(m_ScreenHandler->GetSound()->GetCurrentStream()->GetMetaData());
	}
}

void PlayListScreen::InputHandler(int iButtonMask)
{
	CPSPSound::pspsound_state playingstate = m_ScreenHandler->GetSound()->GetPlayState();
	CMetaDataContainerIndexer *SelectionIndexer = m_Lists->GetCurrentSelectionIndexer();
	CPSPStream *currentStream = m_ScreenHandler->GetSound()->GetCurrentStream();

	Log(LOG_VERYLOW, "OnButtonReleased(): iButtonMask=0x%x", iButtonMask);
		
	if (IS_BUTTON_PRESSED(iButtonMask, PSPRadioButtonMap.BTN_CANCEL))
	{
		m_Lists->SetCurrentSide(CMetaDataContainer::CONTAINER_SIDE_CONTAINERS);

		/** tell ui of m_Lists->GetCurrentSide() change. */
		m_UI->OnCurrentContainerSideChange(m_Lists); 
	}
	else if (IS_BUTTON_PRESSED(iButtonMask, PSPRadioButtonMap.BTN_PGUP))
	{
		switch(m_Lists->GetCurrentSide())
		{
			case CMetaDataContainer::CONTAINER_SIDE_CONTAINERS:
				if (false == m_Lists->GetContainerList()->empty())
				{
					for (int i = 0; i < 10; i++)
					{
						//m_Lists->PrevContainer();
						SelectionIndexer->PrevContainer();
					}
					m_UI->DisplayContainers(m_Lists);
				}
				break;
			
			case CMetaDataContainer::CONTAINER_SIDE_ELEMENTS:
				if (false == m_Lists->GetElementList()->empty())
				{
					for (int i = 0; i < 10; i++)
					{
						//m_Lists->PrevElement();
						SelectionIndexer->PrevElement();
					}
					m_UI->DisplayElements(m_Lists);
				}
				break;
		}
	}
	else if (IS_BUTTON_PRESSED(iButtonMask, PSPRadioButtonMap.BTN_PGDN))
	{
		switch(m_Lists->GetCurrentSide())
		{
			case CMetaDataContainer::CONTAINER_SIDE_CONTAINERS:
				if (false == m_Lists->GetContainerList()->empty())
				{
					for (int i = 0; i < 10; i++)
					{
						//m_Lists->NextContainer();
						SelectionIndexer->NextContainer();
					}
					m_UI->DisplayContainers(m_Lists);
				}
				break;
			
			case CMetaDataContainer::CONTAINER_SIDE_ELEMENTS:
				if (false == m_Lists->GetElementList()->empty())
				{
					for (int i = 0; i < 10; i++)
					{
						//m_Lists->NextElement();
						SelectionIndexer->NextElement();
					}
					m_UI->DisplayElements(m_Lists);
				}
				break;
		}
	}
	else if (IS_BUTTON_PRESSED(iButtonMask, PSPRadioButtonMap.BTN_BACK))
	{
		switch(m_Lists->GetCurrentSide())
		{
			case CMetaDataContainer::CONTAINER_SIDE_CONTAINERS:
				//m_Lists->PrevContainer();
				SelectionIndexer->PrevContainer();
				m_UI->DisplayContainers(m_Lists);
				break;
			
			case CMetaDataContainer::CONTAINER_SIDE_ELEMENTS:
				//m_Lists->PrevElement();
				SelectionIndexer->PrevElement();
				m_UI->DisplayElements(m_Lists);
				break;
		}
	}
	else if (IS_BUTTON_PRESSED(iButtonMask, PSPRadioButtonMap.BTN_FWD))
	{
		switch(m_Lists->GetCurrentSide())
		{
			case CMetaDataContainer::CONTAINER_SIDE_CONTAINERS:
				//m_Lists->NextContainer();
				SelectionIndexer->NextContainer();
				m_UI->DisplayContainers(m_Lists);
				break;
			
			case CMetaDataContainer::CONTAINER_SIDE_ELEMENTS:
				//m_Lists->NextElement();
				SelectionIndexer->NextElement();
				m_UI->DisplayElements(m_Lists);
				break;
		}
	}
	else if (IS_BUTTON_PRESSED(iButtonMask, PSPRadioButtonMap.BTN_OK))
	{
		switch(m_Lists->GetCurrentSide())
		{
			case CMetaDataContainer::CONTAINER_SIDE_CONTAINERS:
				if (false == m_Lists->GetContainerList()->empty())
				{
					//m_Lists->AssociateElementList();
					SelectionIndexer->AssociateElementList();
					m_UI->DisplayElements(m_Lists);
					m_Lists->SetCurrentSide(CMetaDataContainer::CONTAINER_SIDE_ELEMENTS);
					/** Notify the UI of m_Lists->GetCurrentSide() change. */
					m_UI->OnCurrentContainerSideChange(m_Lists);
				}
				break;
			
			case CMetaDataContainer::CONTAINER_SIDE_ELEMENTS:
				if (false == m_Lists->GetElementList()->empty())
				{
					switch(playingstate)
					{
						/** The user pressed X and no track is playing (STOPPED) */
						case CPSPSound::STOP:
							/** Make the play index be whatever is selected */
							m_Lists->SetPlayingToSelection();
							if (m_UI)
							{
								m_UI->DisplayContainers(m_Lists);
								m_UI->DisplayElements(m_Lists);
							}
							/** We start decoding */
							PlaySetStream();
							break;
						/** The user pressed X and the track is PAUSED */
						case CPSPSound::PAUSE:
							/** We Unpause */
							PlaySetStream();
							
							/** If the new selected (not playing) stream is different than the current, only then stop-"restart" */
							if (0 != strcmp(currentStream->GetURI(), (*(m_Lists->GetCurrentElementIterator()))->strURI))
							{
								m_Lists->SetPlayingToSelection();
								
								if (m_UI)
								{
									m_UI->DisplayContainers(m_Lists);
									m_UI->DisplayElements(m_Lists);
								}
								/** We start decoding */
								PlaySetStream();
							}
							break;
						/**  The user pressed X, but a track is currently playing */
						case CPSPSound::PLAY:
							/** No pausing for URLs, only for Files(local) */
							/** If currently playing a stream, and the user presses play, then start the 
							currently selected stream! */
							/** We do this by stopping the stream, and asking the handler to start playing
							when the stream stops. */
							if (CPSPStream::STREAM_STATE_OPEN == currentStream->GetState())
							{
								/** If the new selected (not playing) stream is different than the current, only then stop-"restart" */
								if (0 != strcmp(currentStream->GetURI(), (*(m_Lists->GetCurrentElementIterator()))->strURI))
								{
									m_Lists->SetPlayingToSelection();
									
									if (m_UI)
									{
										m_UI->DisplayContainers(m_Lists);
										m_UI->DisplayElements(m_Lists);
									}

									PlaySetStream();
									/*
									Log(LOG_VERYLOW, "Calling Stop() at InputHandler, X or O pressed, and was playing. Also setting  request to play.");
									m_ScreenHandler->GetSound()->Stop();
									m_ScreenHandler->m_RequestOnPlayOrStop = CScreenHandler::PLAY_REQUEST;
									*/
								}
								else //the selected stream == current stream, if a file stream, then pause.
								{
									if (CPSPStream::STREAM_TYPE_FILE == currentStream->GetType())
									{
										//currentStream->
										//	SetURI((*(m_Lists->GetCurrentElementIterator()))->strURI);
										m_ScreenHandler->GetSound()->Pause();
										OnPlayStateChange(PLAYSTATE_PAUSE);
									}
								}
							}
							else // this shouldn't happen, but we should handle it anyways.
							{
								PlaySetStream();
							}
							break;
					}
				}
				break;
			
			
		}
	}
	else if (IS_BUTTON_PRESSED(iButtonMask, PSPRadioButtonMap.BTN_STOP))
	{
		if (playingstate == CPSPSound::PLAY || playingstate == CPSPSound::PAUSE)
		{
			Log(LOG_VERYLOW, "Calling Stop() at InputHandler, [] pressed.");
			m_ScreenHandler->GetSound()->Stop();
			m_ScreenHandler->m_RequestOnPlayOrStop = CScreenHandler::STOP_REQUEST;
		}
	}
}

void PlayListScreen::OnHPRMReleased(u32 iHPRMMask)
{
	CPSPSound::pspsound_state playingstate = m_ScreenHandler->GetSound()->GetPlayState();
	CMetaDataContainerIndexer *PlayIndexer = m_Lists->GetPlayingTrackIndexer();
	
	Log(LOG_VERYLOW, "OnHPRMReleased(): iHPRMMask=0x%x", iHPRMMask);
	
	if (PlayIndexer->GetElementList() == NULL)
	{
		/** Make the play index be whatever is selected */
		m_Lists->SetPlayingToSelection();
	}
	
	if (iHPRMMask & PSP_HPRM_BACK)
	{
		if (false == m_Lists->GetContainerList()->empty())
		{
			PlayIndexer->PrevGlobalElement();
			if (CPSPSound::PAUSE == playingstate)
			{
				PlaySetStream();
			}
			PlaySetStream();
		}
	}
	else if (iHPRMMask & PSP_HPRM_FORWARD)
	{
		if (false == m_Lists->GetContainerList()->empty())
		{
			PlayIndexer->NextGlobalElement();
			if (CPSPSound::PAUSE == playingstate)
			{
				PlaySetStream();
			}
			PlaySetStream();
		}
	}
	else if (iHPRMMask & PSP_HPRM_PLAYPAUSE) 
	{
		switch(playingstate)
		{
			/** The user pressed X and no track is playing (STOPPED) */
			case CPSPSound::STOP:
				/** Make the play index be whatever is selected */
				m_Lists->SetPlayingToSelection();
			/** The user pressed X and the track is PAUSED or STOPPED */
			case CPSPSound::PAUSE:
				/** We start/continue decoding */
				PlaySetStream();
				break;
			/**  The user pressed X, but a track is currently playing */
			case CPSPSound::PLAY:
				m_ScreenHandler->GetSound()->Pause();
				OnPlayStateChange(PLAYSTATE_PAUSE);
				break;
		}
	}
	
	m_UI->DisplayContainers(m_Lists);
	m_UI->DisplayElements(m_Lists);
}

void PlayListScreen::OnPlayStateChange(playstates NewPlayState)
{
	static playstates OldPlayState = PLAYSTATE_STOP;
	//static time_t	LastEOSinMS = 0;
	CPSPStream *pCurrentStream = m_ScreenHandler->GetSound()->GetCurrentStream();
	//time_t  CurrentStateInMS = clock()*1000/CLOCKS_PER_SEC;
	IPSPRadio_UI *UI = NULL;
	
	Log(LOG_VERYLOW, "OnPlayStateChange(%d) called. (this=%p)", (int)NewPlayState, this);
	
	/* Only tell the UI about changes if the current active screen is this */
	/* This prvents screens like the option screen from getting notified */
	if (this == m_ScreenHandler->GetCurrentScreen())
	{
		UI = m_UI;
	}
	else
	{
		UI = NULL;
	}
	Log(LOG_VERYLOW, "OnPlayStateChange() Right before the switch");
 	switch (NewPlayState)
	{
		/** The stream just started playing */
		case PLAYSTATE_PLAY:
			Log(LOG_VERYLOW, "OnPlayStateChange() Case PLAYSTATE_PLAY");
			if (UI)
				UI->DisplayActiveCommand(CPSPSound::PLAY);
			if (PLAYSTATE_PAUSE != OldPlayState)
			{
				/** If no Metadata, then populate metadata from metadatacontainer list */
				/** Metadata can be available at this point if retrieved from ID3 tag */
				if (strlen(pCurrentStream->GetTitle()) == 0)
				{
					/** Populate m_CurrentMetaData */
					MetaData *source = &(*(*(m_Lists->GetCurrentElementIterator())));
					memcpy(pCurrentStream->GetMetaData(), source, sizeof(MetaData));
				}
				if (UI)
					UI->OnNewSongData(pCurrentStream->GetMetaData());
			}
			Log(LOG_VERYLOW, "OnPlayStateChange() Case PLAYSTATE_PLAY Done.");
			break;
		/** The stream was paused */
		case PLAYSTATE_PAUSE:
			Log(LOG_VERYLOW, "OnPlayStateChange() Case PLAYSTATE_PAUSE");
			if (UI)
				UI->DisplayActiveCommand(CPSPSound::PAUSE);
			break;
		/** The stream was stopped */
		case PLAYSTATE_STOP:
			Log(LOG_VERYLOW, "OnPlayStateChange() Case PLAYSTATE_STOP");
			//if (CurrentStateInMS - LastEOSinMS > 500)
			//{
				/** We have a request to play along with the stream stopping */
				if (CScreenHandler::PLAY_REQUEST == m_ScreenHandler->m_RequestOnPlayOrStop)
				{
					Log(LOG_VERYLOW, "OnPlayStateChange(STOP): with PLAY_REQUEST");
					PlaySetStream();
					
					m_UI->DisplayContainers(m_Lists);
					m_UI->DisplayElements(m_Lists);

				}
				else if (CScreenHandler::STOP_REQUEST == m_ScreenHandler->m_RequestOnPlayOrStop) 
				{
					Log(LOG_VERYLOW, "OnPlayStateChange(STOP): with STOP_REQUEST");
					/** If user selected to stop, just stop */
					if (UI)
						UI->DisplayActiveCommand(CPSPSound::STOP);
				}
				else
				{
					Log(LOG_VERYLOW, "OnPlayStateChange(STOP): with %d request", m_ScreenHandler->m_RequestOnPlayOrStop);
					/** Must have stop because of an error */
					if (UI)
						UI->DisplayActiveCommand(CPSPSound::STOP);
				}
/*			}
			else
			{
				Log(LOG_ERROR, "OnPlayStateChange(): newstate = PLAYSTATE_STOP, but we had an EOS %dms ago, so ignoring.",
					CurrentStateInMS - LastEOSinMS);
			}*/
			break;
	}

	m_ScreenHandler->m_RequestOnPlayOrStop = CScreenHandler::NOTHING; /** Reset */
	
	OldPlayState = NewPlayState;
}

/** The current stream just finished (decoding) */
void PlayListScreen::EOSHandler()
{
	CMetaDataContainerIndexer *PlayIndexer = m_Lists->GetPlayingTrackIndexer();
	IPSPRadio_UI *UI = NULL;
	
	Log(LOG_VERYLOW, "EOSHandler() called. (this=%p)", this);
	
	/* Only tell the UI about changes if the current active screen is this */
	/* This prvents screens like the option screen from getting notified */
	if (this == m_ScreenHandler->GetCurrentScreen())
	{
		UI = m_UI;
	}
	else
	{
		UI = NULL;
	}
	
	//LastEOSinMS = CurrentStateInMS;
	switch(m_ScreenHandler->GetPlayMode())
	{
		case PLAYMODE_SINGLE:
			if (UI)
				UI->DisplayActiveCommand(CPSPSound::STOP);
			break;
		
		case PLAYMODE_NORMAL:
		{
			//play next strack
			PlayIndexer->NextElement();
			if (UI)
				UI->DisplayElements(m_Lists);

			PlaySetStream();
			break;
		}
			
		case PLAYMODE_GLOBAL_NEXT:
		{
			//play next strack
			PlayIndexer->NextGlobalElement();
			if (UI)
			{
				UI->DisplayContainers(m_Lists);
				UI->DisplayElements(m_Lists);
			}

			PlaySetStream();
			break;
		}
		
		case PLAYMODE_REPEAT:
		{
			PlaySetStream();
			break;
		}
	}
}


void PlayListScreen::PlaySetStream()
{
	CPSPSound::pspsound_state playingstate = m_ScreenHandler->GetSound()->GetPlayState();
	CPSPStream *currentStream = m_ScreenHandler->GetSound()->GetCurrentStream();

	switch (playingstate)
	{
		case CPSPSound::PLAY:
		case CPSPSound::STOP:
			memcpy(currentStream->GetMetaData(), &(*(*(m_Lists->GetCurrentElementIterator()))), sizeof(MetaData));
			currentStream->SetURI((*(m_Lists->GetPlayingElementIterator()))->strURI);
			break;
		case CPSPSound::PAUSE:
			break;
	}
	
	Log(LOG_LOWLEVEL, "Calling Play. URI set to '%s'", currentStream->GetURI());
	#if 0
	/** Populate m_CurrentMetaData */
		///memcpy(&(*(m_Lists->GetCurrentElementIterator())), m_ScreenHandler->GetSound()->GetCurrentStream()->GetMetaData(), sizeof(MetaData));
		MetaData *source = &(*(*(m_Lists->GetCurrentElementIterator())));
		memcpy(pCurrentStream->GetMetaData(), source, sizeof(MetaData));
		if (UI)
			UI->OnNewSongData(pCurrentStream->GetMetaData());
	#endif
	m_ScreenHandler->GetSound()->Play();
	m_ScreenHandler->SetStreamOwnerScreen(this);
}
