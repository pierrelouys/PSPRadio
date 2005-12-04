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
#include "PlayListScreen.h"
#include "TextUI.h"
#include "GraphicsUI.h"
#include "SandbergUI.h" 

PlayListScreen::PlayListScreen(int Id, CScreenHandler *ScreenHandler): IScreen(Id, ScreenHandler)
{
	Log(LOG_VERYLOW,"PlayListScreen Ctor.");
	m_Lists = NULL;

	m_Lists = new CMetaDataContainer();

	LoadLists();
}

PlayListScreen::~PlayListScreen()
{
	if (m_Lists)
	{
		delete(m_Lists), m_Lists = NULL;
	}
}

void PlayListScreen::LoadLists()
{
	if (m_Lists)
	{
		Log(LOG_LOWLEVEL, "Loading playlists");
		m_Lists->Clear();
		
		char *strFileName = NULL;
		strFileName = (char *)malloc(strlen(m_ScreenHandler->GetCWD()) + strlen("PlayLists") + 10);
		sprintf(strFileName, "%s/PlayLists", m_ScreenHandler->GetCWD());
		m_Lists->LoadDirectory(strFileName); //**//

		m_Lists->SetCurrentSide(CMetaDataContainer::CONTAINER_SIDE_CONTAINERS);

	}
}

void PlayListScreen::Activate(IPSPRadio_UI *UI)
{
	IScreen::Activate(UI);

	Log(LOG_VERYLOW, "Activate(): Start");
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
	Log(LOG_VERYLOW, "OnButtonReleased(): iButtonMask=0x%x", iButtonMask);
	
		
	if (iButtonMask & PSP_CTRL_LEFT)
	{
		m_Lists->SetCurrentSide(CMetaDataContainer::CONTAINER_SIDE_CONTAINERS);

		/** tell ui of m_Lists->GetCurrentSide() change. */
		m_UI->OnCurrentContainerSideChange(m_Lists); 
	}
	else if (iButtonMask & PSP_CTRL_LTRIGGER)
	{
		switch(m_Lists->GetCurrentSide())
		{
			case CMetaDataContainer::CONTAINER_SIDE_CONTAINERS:
				if (false == m_Lists->GetContainerList()->empty())
				{
					for (int i = 0; i < 10; i++)
					{
						m_Lists->PrevContainer();
					}
					m_UI->DisplayContainers(m_Lists);
				}
				break;
			
			case CMetaDataContainer::CONTAINER_SIDE_ELEMENTS:
				if (false == m_Lists->GetElementList()->empty())
				{
					for (int i = 0; i < 10; i++)
					{
						m_Lists->PrevElement();
					}
					m_UI->DisplayElements(m_Lists);
				}
				break;
		}
	}
	else if (iButtonMask & PSP_CTRL_RTRIGGER)
	{
		switch(m_Lists->GetCurrentSide())
		{
			case CMetaDataContainer::CONTAINER_SIDE_CONTAINERS:
				if (false == m_Lists->GetContainerList()->empty())
				{
					for (int i = 0; i < 10; i++)
					{
						m_Lists->NextContainer();
					}
					m_UI->DisplayContainers(m_Lists);
				}
				break;
			
			case CMetaDataContainer::CONTAINER_SIDE_ELEMENTS:
				if (false == m_Lists->GetElementList()->empty())
				{
					for (int i = 0; i < 10; i++)
					{
						m_Lists->NextElement();
					}
					m_UI->DisplayElements(m_Lists);
				}
				break;
		}
	}
	else if (iButtonMask & PSP_CTRL_UP)
	{
		switch(m_Lists->GetCurrentSide())
		{
			case CMetaDataContainer::CONTAINER_SIDE_CONTAINERS:
				m_Lists->PrevContainer();
				m_UI->DisplayContainers(m_Lists);
				break;
			
			case CMetaDataContainer::CONTAINER_SIDE_ELEMENTS:
				m_Lists->PrevElement();
				m_UI->DisplayElements(m_Lists);
				break;
		}
	}
	else if (iButtonMask & PSP_CTRL_DOWN)
	{
		switch(m_Lists->GetCurrentSide())
		{
			case CMetaDataContainer::CONTAINER_SIDE_CONTAINERS:
				m_Lists->NextContainer();
				m_UI->DisplayContainers(m_Lists);
				break;
			
			case CMetaDataContainer::CONTAINER_SIDE_ELEMENTS:
				m_Lists->NextElement();
				m_UI->DisplayElements(m_Lists);
				break;
		}
	}
	else if (iButtonMask & PSP_CTRL_CROSS || iButtonMask & PSP_CTRL_CIRCLE) 
	{
		switch(m_Lists->GetCurrentSide())
		{
			case CMetaDataContainer::CONTAINER_SIDE_CONTAINERS:
				if (false == m_Lists->GetContainerList()->empty())
				{
					m_Lists->AssociateElementList();
					m_UI->DisplayElements(m_Lists);
					m_Lists->SetCurrentSide(CMetaDataContainer::CONTAINER_SIDE_ELEMENTS);
					/** Notify the UI of m_Lists->GetCurrentSide() change. */
					m_UI->OnCurrentContainerSideChange(m_Lists);
				}
				break;
			
			case CMetaDataContainer::CONTAINER_SIDE_ELEMENTS:
				if (false == m_Lists->GetElementList()->empty())
				{
					m_ScreenHandler->SetStreamOwnerScreen(this);
					switch(playingstate)
					{
						case CPSPSound::STOP:
						case CPSPSound::PAUSE:
							m_ScreenHandler->GetSound()->GetCurrentStream()->
									SetURI((*(m_Lists->GetCurrentElementIterator()))->strURI);
							Log(LOG_LOWLEVEL, "Calling Play. URI set to '%s'", m_ScreenHandler->GetSound()->GetCurrentStream()->GetURI());
							m_ScreenHandler->GetSound()->Play();
							break;
						case CPSPSound::PLAY:
							/** No pausing for URLs, only for Files(local) */
							//if (CPSPStream::STREAM_TYPE_FILE == m_ScreenHandler->GetSound()->GetCurrentStream()->GetType())
							//{
							//	m_ScreenHandler->GetSound()->GetCurrentStream()->
							//		SetURI((*(m_Lists->GetCurrentElementIterator()))->strURI);
							//	m_UI->DisplayActiveCommand(CPSPSound::PAUSE);
							//	m_ScreenHandler->GetSound()->Pause();
							//}
							//else
							{
								/** If currently playing a stream, and the user presses play, then start the 
								currently selected stream! */
								/** We do this by stopping the stream, and asking the handler to start playing
								when the stream stops. */
								if (CPSPStream::STREAM_STATE_OPEN == m_ScreenHandler->GetSound()->GetCurrentStream()->GetState())
								{
									/** If the new stream is different than the current, only then stop-"restart" */
									if (0 != strcmp(m_ScreenHandler->GetSound()->GetCurrentStream()->GetURI(), (*(m_Lists->GetCurrentElementIterator()))->strURI))
									{
										Log(LOG_VERYLOW, "Calling Stop() at InputHandler, X or O pressed, and was playing. Also setting  request to play.");
										m_ScreenHandler->GetSound()->Stop();
										m_ScreenHandler->m_RequestOnPlayOrStop = CScreenHandler::PLAY_REQUEST;
									}
									else
									{
										//the selected stream == current stream, if a file stream, then pause.
										if (CPSPStream::STREAM_TYPE_FILE == m_ScreenHandler->GetSound()->GetCurrentStream()->GetType())
										{
											//m_ScreenHandler->GetSound()->GetCurrentStream()->
											//	SetURI((*(m_Lists->GetCurrentElementIterator()))->strURI);
											m_ScreenHandler->GetSound()->Pause();
											OnPlayStateChange(PLAYSTATE_PAUSE);
										}
									}
								}
								else // this shouldn't happen, but we should handle it anyways.
								{
									m_ScreenHandler->GetSound()->GetCurrentStream()->
										SetURI((*(m_Lists->GetCurrentElementIterator()))->strURI);
									Log(LOG_LOWLEVEL, "Calling Play. URI set to '%s'", m_ScreenHandler->GetSound()->GetCurrentStream()->GetURI());
									m_ScreenHandler->GetSound()->Play();
								}
							}
							break;
					}
				}
				break;
			
			
		}
	}
	else if (iButtonMask & PSP_CTRL_SQUARE)
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
	Log(LOG_VERYLOW, "OnHPRMReleased(): iHPRMMask=0x%x", iHPRMMask);
	CPSPSound::pspsound_state playingstate = m_ScreenHandler->GetSound()->GetPlayState();

	if (iHPRMMask & PSP_HPRM_BACK)
	{
		m_Lists->PrevElement();
		m_UI->DisplayElements(m_Lists);
	}
	else if (iHPRMMask & PSP_HPRM_FORWARD)
	{
		m_Lists->NextElement();
		m_UI->DisplayElements(m_Lists);
	}

	else if (iHPRMMask & PSP_HPRM_PLAYPAUSE) 
	{
		switch(playingstate)
		{
			case CPSPSound::STOP:
			case CPSPSound::PAUSE:
				m_ScreenHandler->GetSound()->GetCurrentStream()->SetURI((*(m_Lists->GetCurrentElementIterator()))->strURI);
				m_UI->DisplayActiveCommand(CPSPSound::PLAY);
				m_ScreenHandler->GetSound()->Play();
				/** Populate m_CurrentMetaData */
				//m_CurrentPlayList->GetCurrentSong(m_ScreenHandler->GetSound()->GetCurrentStream()->GetMetaData());
				///m_Sound->GetCurrentStream()->SetURI(m_CurrentPlayList->GetURI());
				//m_UI->OnNewSongData(m_ScreenHandler->GetSound()->GetCurrentStream()->GetMetaData());
				break;
			case CPSPSound::PLAY:
				m_UI->DisplayActiveCommand(CPSPSound::STOP);
				Log(LOG_VERYLOW, "Calling Stop() on HPRM PLAY/PAUSE pressed; currently Playing.");
				m_ScreenHandler->GetSound()->Stop();
				break;
		}
	}
}

void PlayListScreen::OnPlayStateChange(playstates NewPlayState)
{
	static playstates OldPlayState = PLAYSTATE_STOP;
	static time_t	LastEOSinMS = 0;
	CPSPStream *pCurrentStream = m_ScreenHandler->GetSound()->GetCurrentStream();
	time_t  CurrentStateInMS = clock()*1000/CLOCKS_PER_SEC;
	IPSPRadio_UI *UI = NULL;
	
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
	
	Log(LOG_VERYLOW, "OnPlayStateChange() Called with %d (%s)",
		NewPlayState, 
		(NewPlayState==PLAYSTATE_PLAY)?"PLAY":
		((NewPlayState==PLAYSTATE_STOP)?"STOP":
		((NewPlayState==PLAYSTATE_PAUSE)?"PAUSE":
		((NewPlayState==PLAYSTATE_EOS)?"EOS":"Undefined") ) ) );
	
	switch (NewPlayState)
	{
		case PLAYSTATE_PLAY:
			if (UI)
				UI->DisplayActiveCommand(CPSPSound::PLAY);
			if (PLAYSTATE_PAUSE != OldPlayState)
			{
				/** Populate m_CurrentMetaData */
				MetaData *source = &(*(*(m_Lists->GetCurrentElementIterator())));
				memcpy(pCurrentStream->GetMetaData(), source, sizeof(MetaData));
				if (UI)
					UI->OnNewSongData(pCurrentStream->GetMetaData());
			}
			break;
		case PLAYSTATE_PAUSE:
			if (UI)
				UI->DisplayActiveCommand(CPSPSound::PAUSE);
			break;
		case PLAYSTATE_STOP:
			if (CurrentStateInMS - LastEOSinMS > 500)
			{
				if (CScreenHandler::PLAY_REQUEST == m_ScreenHandler->m_RequestOnPlayOrStop)
				{
					Log(LOG_VERYLOW, "OnPlayStateChange(STOP): with PLAY_REQUEST");
					pCurrentStream->SetURI((*(m_Lists->GetCurrentElementIterator()))->strURI);
					
					/** Populate m_CurrentMetaData */
					///memcpy(&(*(m_Lists->GetCurrentElementIterator())), m_ScreenHandler->GetSound()->GetCurrentStream()->GetMetaData(), sizeof(MetaData));
					MetaData *source = &(*(*(m_Lists->GetCurrentElementIterator())));
					memcpy(pCurrentStream->GetMetaData(), source, sizeof(MetaData));
					if (UI)
						UI->OnNewSongData(pCurrentStream->GetMetaData());
					
					m_ScreenHandler->GetSound()->Play();
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
			}
			else
			{
				Log(LOG_ERROR, "OnPlayStateChange(): newstate = PLAYSTATE_STOP, but we had a EOS %sms ago, so ignoring.",
					CurrentStateInMS - LastEOSinMS);
			}
			break;
		case PLAYSTATE_EOS:
			LastEOSinMS = CurrentStateInMS;
			switch(m_ScreenHandler->GetPlayMode())
			{
				case PLAYMODE_SINGLE:
					if (UI)
						UI->DisplayActiveCommand(CPSPSound::STOP);
					break;
				
				case PLAYMODE_NORMAL:
				{
					//play next strack
					m_Lists->NextElement();
					if (UI)
						UI->DisplayElements(m_Lists);

					pCurrentStream->SetURI((*(m_Lists->GetCurrentElementIterator()))->strURI);
					
					/** Populate m_CurrentMetaData */
					///memcpy(&(*(m_Lists->GetCurrentElementIterator())), m_ScreenHandler->GetSound()->GetCurrentStream()->GetMetaData(), sizeof(MetaData));
					MetaData *source = &(*(*(m_Lists->GetCurrentElementIterator())));
					memcpy(pCurrentStream->GetMetaData(), source, sizeof(MetaData));
					if (UI)
						UI->OnNewSongData(pCurrentStream->GetMetaData());
					
					m_ScreenHandler->GetSound()->Play();
					
					break;
				}
					
				case PLAYMODE_REPEAT:
				{
					MetaData *source = &(*(*(m_Lists->GetCurrentElementIterator())));
					memcpy(pCurrentStream->GetMetaData(), source, sizeof(MetaData));
					if (UI)
						UI->OnNewSongData(pCurrentStream->GetMetaData());
					
					m_ScreenHandler->GetSound()->Play();
					break;
				}
			}
			break;
	}

	m_ScreenHandler->m_RequestOnPlayOrStop = CScreenHandler::NOTHING; /** Reset */
	
	OldPlayState = NewPlayState;
}
