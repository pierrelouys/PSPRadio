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
#include "DirList.h"
#include "PlayList.h"
#include "TextUI.h"
#include "GraphicsUI.h"
#include "SandbergUI.h" 

PlayListScreen::PlayListScreen(int Id, CScreenHandler *ScreenHandler): IScreen(Id, ScreenHandler)
{
	Log(LOG_VERYLOW,"PlayListScreen Ctor.");
	m_CurrentPlayList = new CPlayList();
	m_CurrentPlayListDir = new CDirList();

	LoadLists();
}

PlayListScreen::~PlayListScreen()
{
	if (m_CurrentPlayListDir)
	{
		Log(LOG_VERYLOW, "~PlayListScreen(). Destroying m_CurrentPlayListDir object");
		delete(m_CurrentPlayListDir);
	}
	if (m_CurrentPlayList)
	{
		Log(LOG_VERYLOW, "~PlayListScreen(). Destroying m_CurrentPlayList object");
		delete(m_CurrentPlayList);
	}
}

void PlayListScreen::LoadLists()
{
	if (m_CurrentPlayListDir && m_CurrentPlayList)
	{
		Log(LOG_LOWLEVEL, "Displaying current playlist");
		m_CurrentPlayListDir->Clear();
		char *strFileName = NULL;
		strFileName = (char *)malloc(strlen(m_ScreenHandler->GetCWD()) + strlen("PlayLists") + 10);
		sprintf(strFileName, "%s/PlayLists", m_ScreenHandler->GetCWD());
		m_CurrentPlayListDir->LoadDirectory(strFileName); //**//
		free(strFileName),strFileName = NULL;
		//m_CurrentPlayListDir->LoadDirectory(); //**//
		if (m_CurrentPlayListDir->Size() > 0)
		{
			Log(LOG_LOWLEVEL, "Loading Playlist file '%s'.", m_CurrentPlayListDir->GetCurrentURI());
			m_CurrentPlayList->Clear();
			m_CurrentPlayList->LoadPlayListURI(m_CurrentPlayListDir->GetCurrentURI());
		}

		m_CurrentPlayListSideSelection = PlayListScreen::PLAYLIST_LIST;

	}
}

void PlayListScreen::Activate(IPSPRadio_UI *UI)
{
	IScreen::Activate(UI);

	if (m_CurrentPlayListDir->GetList()->size() > 0)
	{
		m_UI->DisplayPLList(m_CurrentPlayListDir);
	}
	/** tell ui of m_CurrentPlayListSideSelection change. */
	if(m_CurrentPlayList->GetList()->size() > 0)
	{
		m_UI->DisplayPLEntries(m_CurrentPlayList);
	}
	m_UI->OnCurrentPlayListSideSelectionChange(m_CurrentPlayListSideSelection); 

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
		m_CurrentPlayListSideSelection = PlayListScreen::PLAYLIST_LIST;
		/** tell ui of m_CurrentPlayListSideSelection change. */
		m_UI->OnCurrentPlayListSideSelectionChange(m_CurrentPlayListSideSelection); 
	}
	else if (iButtonMask & PSP_CTRL_LTRIGGER)
	{
		switch(m_CurrentPlayListSideSelection)
		{
			case PlayListScreen::PLAYLIST_LIST:
				if (m_CurrentPlayListDir->GetList()->size() > 0)
				{
					for (int i = 0; i < 10; i++)
					{
						m_CurrentPlayListDir->Prev();
					}
					m_UI->DisplayPLList(m_CurrentPlayListDir);
				}
				break;
			
			case PlayListScreen::PLAYLIST_ENTRIES:
				if (m_CurrentPlayList->GetList()->size() > 0)
				{
					for (int i = 0; i < 10; i++)
					{
						m_CurrentPlayList->Prev();
					}
					m_UI->DisplayPLEntries(m_CurrentPlayList);
				}
				break;
		}
	}
	else if (iButtonMask & PSP_CTRL_RTRIGGER)
	{
		switch(m_CurrentPlayListSideSelection)
		{
			case PlayListScreen::PLAYLIST_LIST:
				if (m_CurrentPlayListDir->GetList()->size() > 0)
				{
					for (int i = 0; i < 10; i++)
					{
						m_CurrentPlayListDir->Next();
					}
					m_UI->DisplayPLList(m_CurrentPlayListDir);
				}
				break;
			
			case PlayListScreen::PLAYLIST_ENTRIES:
				if (m_CurrentPlayList->GetList()->size() > 0)
				{
					for (int i = 0; i < 10; i++)
					{
						m_CurrentPlayList->Next();
					}
					m_UI->DisplayPLEntries(m_CurrentPlayList);
				}
				break;
		}
	}
	else if (iButtonMask & PSP_CTRL_UP)
	{
		switch(m_CurrentPlayListSideSelection)
		{
			case PlayListScreen::PLAYLIST_LIST:
				m_CurrentPlayListDir->Prev();
				m_UI->DisplayPLList(m_CurrentPlayListDir);
				break;
			
			case PlayListScreen::PLAYLIST_ENTRIES:
				m_CurrentPlayList->Prev();
				m_UI->DisplayPLEntries(m_CurrentPlayList);
				break;
		}
	}
	else if (iButtonMask & PSP_CTRL_DOWN)
	{
		switch(m_CurrentPlayListSideSelection)
		{
			case PlayListScreen::PLAYLIST_LIST:
				m_CurrentPlayListDir->Next();
				m_UI->DisplayPLList(m_CurrentPlayListDir);
				break;
			
			case PlayListScreen::PLAYLIST_ENTRIES:
				m_CurrentPlayList->Next();
				m_UI->DisplayPLEntries(m_CurrentPlayList);
				break;
		}
	}
	else if (iButtonMask & PSP_CTRL_CROSS || iButtonMask & PSP_CTRL_CIRCLE) 
	{
		switch(m_CurrentPlayListSideSelection)
		{
			case PlayListScreen::PLAYLIST_LIST:
				if (m_CurrentPlayListDir->GetList()->size() > 0)
				{
					m_CurrentPlayList->Clear();
					if (CScreenHandler::PSPRADIO_SCREEN_SHOUTCAST_BROWSER == m_ScreenHandler->GetCurrentScreen()->GetId())
					{
						m_CurrentPlayList->LoadPlayListFromSHOUTcastXML(m_CurrentPlayListDir->GetCurrentURI());
					}
					else
					{
						m_CurrentPlayList->LoadPlayListURI(m_CurrentPlayListDir->GetCurrentURI());
					}
					m_UI->DisplayPLEntries(m_CurrentPlayList);
					m_CurrentPlayListSideSelection = PlayListScreen::PLAYLIST_ENTRIES;
					/** Notify the UI of m_CurrentPlayListSideSelection change. */
					m_UI->OnCurrentPlayListSideSelectionChange(m_CurrentPlayListSideSelection); 
				}
				break;
			
			case PlayListScreen::PLAYLIST_ENTRIES:
				if (m_CurrentPlayList->GetList()->size() > 0)
				{
					switch(playingstate)
					{
						case CPSPSound::STOP:
						case CPSPSound::PAUSE:
							m_ScreenHandler->GetSound()->GetCurrentStream()->SetURI(m_CurrentPlayList->GetCurrentURI());
							Log(LOG_LOWLEVEL, "Calling Play. URI set to '%s'", m_ScreenHandler->GetSound()->GetCurrentStream()->GetURI());
							m_ScreenHandler->GetSound()->Play();
							break;
						case CPSPSound::PLAY:
							/** No pausing for URLs, only for Files(local) */
							if (CPSPStream::STREAM_TYPE_FILE == m_ScreenHandler->GetSound()->GetCurrentStream()->GetType())
							{
								m_ScreenHandler->GetSound()->GetCurrentStream()->SetURI(m_CurrentPlayList->GetCurrentURI());
								m_UI->DisplayActiveCommand(CPSPSound::PAUSE);
								m_ScreenHandler->GetSound()->Pause();
							}
							else
							{
								/** If currently playing a stream, and the user presses play, then start the 
								currently selected stream! */
								/** We do this by stopping the stream, and asking the handler to start playing
								when the stream stops. */
								if (CPSPStream::STREAM_STATE_OPEN == m_ScreenHandler->GetSound()->GetCurrentStream()->GetState())
								{
									/** If the new stream is different than the current, only then stop-"restart" */
									if (0 != strcmp(m_ScreenHandler->GetSound()->GetCurrentStream()->GetURI(), m_CurrentPlayList->GetCurrentURI()))
									{
										Log(LOG_VERYLOW, "Calling Stop() at InputHandler, X or O pressed, and was playing. Also setting  request to play.");
										m_ScreenHandler->GetSound()->Stop();
										m_ScreenHandler->m_RequestOnPlayOrStop = CScreenHandler::PLAY;
									}
									else
									{
										Log(LOG_VERYLOW, "Not Stopping/Restarting, as the selected stream == current stream");
									}
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
		}
	}
}

void PlayListScreen::OnHPRMReleased(u32 iHPRMMask)
{
	Log(LOG_VERYLOW, "OnHPRMReleased(): iHPRMMask=0x%x", iHPRMMask);
	CPSPSound::pspsound_state playingstate = m_ScreenHandler->GetSound()->GetPlayState();

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
				m_ScreenHandler->GetSound()->GetCurrentStream()->SetURI(m_CurrentPlayList->GetCurrentURI());
				m_UI->DisplayActiveCommand(CPSPSound::PLAY);
				m_ScreenHandler->GetSound()->Play();
				/** Populate m_CurrentMetaData */
				m_CurrentPlayList->GetCurrentSong(m_ScreenHandler->GetSound()->GetCurrentStream()->GetMetaData());
				//m_Sound->GetCurrentStream()->SetURI(m_CurrentPlayList->GetURI());
				m_UI->OnNewSongData(m_ScreenHandler->GetSound()->GetCurrentStream()->GetMetaData());
				break;
			case CPSPSound::PLAY:
				m_UI->DisplayActiveCommand(CPSPSound::STOP);
				Log(LOG_VERYLOW, "Calling Stop() on HPRM PLAY/PAUSE pressed; currently Playing.");
				m_ScreenHandler->GetSound()->Stop();
				break;
		}
	}
}

void PlayListScreen::OnPlayStateChange(CPSPSound::pspsound_state NewPlayState)
{
	static CPSPSound::pspsound_state OldPlayState = CPSPSound::STOP;
	
	switch(OldPlayState)
	{
		case CPSPSound::STOP:
			switch(NewPlayState)
			{
				case CPSPSound::PLAY:
					if (m_UI)
						m_UI->DisplayActiveCommand(CPSPSound::PLAY);
					/** Populate m_CurrentMetaData */
					m_CurrentPlayList->GetCurrentSong(m_ScreenHandler->GetSound()->GetCurrentStream()->GetMetaData());
					if (m_UI)
						m_UI->OnNewSongData(m_ScreenHandler->GetSound()->GetCurrentStream()->GetMetaData());
					break;
				
				case CPSPSound::STOP:
					if (m_UI)
						m_UI->DisplayActiveCommand(CPSPSound::STOP);
					Log(LOG_VERYLOW, "Calling Stop() on OnPlayStateChange Old=STOP, New=STOP.");
					m_ScreenHandler->GetSound()->Stop();
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
					if (m_UI)
						m_UI->DisplayActiveCommand(CPSPSound::STOP);
					//m_Sound->Stop();
					
					if (CScreenHandler::PLAY == m_ScreenHandler->m_RequestOnPlayOrStop)
					{
						m_ScreenHandler->GetSound()->GetCurrentStream()->SetURI(m_CurrentPlayList->GetCurrentURI());
						
						/** Populate m_CurrentMetaData */
						m_CurrentPlayList->GetCurrentSong(m_ScreenHandler->GetSound()->GetCurrentStream()->GetMetaData());
						
						m_ScreenHandler->GetSound()->Play();
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
