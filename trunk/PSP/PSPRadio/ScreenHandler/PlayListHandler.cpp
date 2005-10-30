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

void CScreenHandler::PlayListScreenInputHandler(int iButtonMask)
{
	CPSPSound::pspsound_state playingstate = m_Sound->GetPlayState();
	Log(LOG_VERYLOW, "OnButtonReleased(): iButtonMask=0x%x", iButtonMask);
	
		
	if (iButtonMask & PSP_CTRL_LEFT)
	{
		m_CurrentPlayListSideSelection = PLAYLIST_LIST;
		/** tell ui of m_CurrentPlayListSideSelection change. */
		m_UI->OnCurrentPlayListSideSelectionChange(m_CurrentPlayListSideSelection); 
	}
	else if (iButtonMask & PSP_CTRL_LTRIGGER)
	{
		switch(m_CurrentPlayListSideSelection)
		{
			case PLAYLIST_LIST:
				for (int i = 0; i < 10; i++)
				{
					m_CurrentPlayListDir->Prev();
				}
				m_UI->DisplayPLList(m_CurrentPlayListDir);
				break;
			
			case PLAYLIST_ENTRIES:
				for (int i = 0; i < 10; i++)
				{
					m_CurrentPlayList->Prev();
				}
				m_UI->DisplayPLEntries(m_CurrentPlayList);
				break;
		}
	}
	else if (iButtonMask & PSP_CTRL_RTRIGGER)
	{
		switch(m_CurrentPlayListSideSelection)
		{
			case PLAYLIST_LIST:
				for (int i = 0; i < 10; i++)
				{
					m_CurrentPlayListDir->Next();
				}
				m_UI->DisplayPLList(m_CurrentPlayListDir);
				break;
			
			case PLAYLIST_ENTRIES:
				for (int i = 0; i < 10; i++)
				{
					m_CurrentPlayList->Next();
				}
				m_UI->DisplayPLEntries(m_CurrentPlayList);
				break;
		}
	}
	else if (iButtonMask & PSP_CTRL_UP)
	{
		switch(m_CurrentPlayListSideSelection)
		{
			case PLAYLIST_LIST:
				m_CurrentPlayListDir->Prev();
				m_UI->DisplayPLList(m_CurrentPlayListDir);
				break;
			
			case PLAYLIST_ENTRIES:
				m_CurrentPlayList->Prev();
				m_UI->DisplayPLEntries(m_CurrentPlayList);
				break;
		}
	}
	else if (iButtonMask & PSP_CTRL_DOWN)
	{
		switch(m_CurrentPlayListSideSelection)
		{
			case PLAYLIST_LIST:
				m_CurrentPlayListDir->Next();
				m_UI->DisplayPLList(m_CurrentPlayListDir);
				break;
			
			case PLAYLIST_ENTRIES:
				m_CurrentPlayList->Next();
				m_UI->DisplayPLEntries(m_CurrentPlayList);
				break;
		}
	}
	else if (iButtonMask & PSP_CTRL_CROSS || iButtonMask & PSP_CTRL_CIRCLE) 
	{
		switch(m_CurrentPlayListSideSelection)
		{
			case PLAYLIST_LIST:
				m_CurrentPlayList->Clear();
				if (PSPRADIO_SCREEN_SHOUTCAST_BROWSER == m_CurrentScreen)
				{
					m_CurrentPlayList->LoadPlayListFromSHOUTcastXML(m_CurrentPlayListDir->GetCurrentURI());
				}
				else
				{
					m_CurrentPlayList->LoadPlayListURI(m_CurrentPlayListDir->GetCurrentURI());
				}
				m_UI->DisplayPLEntries(m_CurrentPlayList);
				m_CurrentPlayListSideSelection = PLAYLIST_ENTRIES;
				/** Notify the UI of m_CurrentPlayListSideSelection change. */
				m_UI->OnCurrentPlayListSideSelectionChange(m_CurrentPlayListSideSelection); 
				break;
			
			case PLAYLIST_ENTRIES:
				switch(playingstate)
				{
					case CPSPSound::STOP:
					case CPSPSound::PAUSE:
						CurrentSoundStream->SetURI(m_CurrentPlayList->GetCurrentURI());
						Log(LOG_LOWLEVEL, "Calling Play. URI set to '%s'", CurrentSoundStream->GetURI());
						m_Sound->Play();
						break;
					case CPSPSound::PLAY:
						/** No pausing for URLs, only for Files(local) */
						if (CPSPStream::STREAM_TYPE_FILE == CurrentSoundStream->GetType())
						{
							CurrentSoundStream->SetURI(m_CurrentPlayList->GetCurrentURI());
							m_UI->DisplayActiveCommand(CPSPSound::PAUSE);
							m_Sound->Pause();
						}
						else
						{
							/** If currently playing a stream, and the user presses play, then start the 
							currently selected stream! */
							/** We do this by stopping the stream, and asking the handler to start playing
							when the stream stops. */
							if (CPSPStream::STREAM_STATE_OPEN == CurrentSoundStream->GetState())
							{
								/** If the new stream is different than the current, only then stop-"restart" */
								if (0 != strcmp(CurrentSoundStream->GetURI(), m_CurrentPlayList->GetCurrentURI()))
								{
									Log(LOG_VERYLOW, "Calling Stop() at InputHandler, X or O pressed, and was playing. Also setting  request to play.");
									m_Sound->Stop();
									m_RequestOnPlayOrStop = PLAY;
								}
								else
								{
									Log(LOG_VERYLOW, "Not Stopping/Restarting, as the selected stream == current stream");
								}
							}
						}
						break;
				}
				break;
			
			
		}
	}
	else if (iButtonMask & PSP_CTRL_SQUARE)
	{
		if (playingstate == CPSPSound::PLAY || playingstate == CPSPSound::PAUSE)
		{
			Log(LOG_VERYLOW, "Calling Stop() at InputHandler, [] pressed.");
			m_Sound->Stop();
		}
	}
}
