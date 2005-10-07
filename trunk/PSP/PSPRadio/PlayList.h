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
#ifndef __PLAYLIST_H__
	#define __PLAYLIST_H__
	
	#include <list>
	using namespace std;

	class CPlayList
	{
	public:
		struct songmetadata
		{
			char strFileName[MAXPATHLEN];
			char strFileTitle[300];
			char strURL[MAXPATHLEN];
			char songTitle[300];
			char songAuthor[300];
			int iLength;
		};
		
		CPlayList();
		~CPlayList();
		
		int GetCurrentSong(songmetadata *pData);
		char *GetCurrentFileName() { return (*m_songiterator).strFileName?(*m_songiterator).strFileName:(char*)""; };
	
		void Next();
		void Prev();
		int GetNumberOfSongs();
		void InsertURI(char *strFileName);
		void LoadPlayListURI(char *strFileName);
		void Clear();
		
	private:
		list<songmetadata> m_playlist; 
		list<songmetadata>::iterator m_songiterator;
	};

#endif
