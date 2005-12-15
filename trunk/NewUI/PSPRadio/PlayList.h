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
	#include <PSPSound.h>
	using namespace std;

	class CPlayList
	{
	public:
		
		CPlayList();
		~CPlayList();
		
		int GetCurrentSong(MetaData *pData);
		char *GetCurrentURI() { return (*m_songiterator).strURI?(*m_songiterator).strURI:(char*)""; };
		int GetCurrentIndex() { return (*m_songiterator).iItemIndex; };
	
		void Next();
		void Prev();
		int GetNumberOfSongs();
		void InsertURI(char *strFileName);
		void LoadPlayListURI(char *strFileName);
		void LoadPlayListFromSHOUTcastXML(char *strFileName);
		void Clear();
		
		/** Accessors */
		list<MetaData> *GetList() { return &m_playlist; }
		list<MetaData>::iterator *GetCurrentElementIterator() { return &m_songiterator; }
		
	private:
		list<MetaData> m_playlist; 
		list<MetaData>::iterator m_songiterator;
	};

#endif
