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
#ifndef __DIRLIST_H__
	#define __DIRLIST_H__
	
	#include <pspiofilemgr_dirent.h>
	#include <list>
	using namespace std;

	class CDirList
	{
	public:
		struct directorydata
		{
			char strURI[MAXPATHLEN];
		};
		
		CDirList();
		~CDirList();
		
		char *GetCurrentURI() { return (*m_diriterator).strURI?(*m_diriterator).strURI:(char*)""; };
	
		void Next();
		void Prev();
		int  Size();
		void LoadDirectory(char *strDirectory);
		void InsertURI(char *strURI);
		void Clear();
		
		/** Accessors */
		list<directorydata> *GetList() { return &m_dirlist; }
		list<directorydata>::iterator *GetCurrentElementIterator() { return &m_diriterator; }
		
	private:
		list<directorydata> m_dirlist; 
		list<directorydata>::iterator m_diriterator;
		
	};

#endif
