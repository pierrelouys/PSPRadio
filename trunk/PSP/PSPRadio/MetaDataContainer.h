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
#ifndef __CONTAINER_H__
	#define __CONTAINER_H__
	#include <PSPStream.h>
	
	#include <string>
	#include <list>
	#include <map>
	
	using namespace std;

	class CMetaDataContainer
	{
		public:
			enum Side
			{
				CONTAINER_SIDE_CONTAINERS,
				CONTAINER_SIDE_ELEMENTS
			};

			CMetaDataContainer();
			~CMetaDataContainer();
		
			void Clear();
			void NextContainer();
			void PrevContainer();
			void AssociateElementList();
			void NextElement();
			void PrevElement();

			void LoadDirectory(char *strDirectory);
			void LoadSHOUTcastXML(char *strFileName);
			Side GetCurrentSide() { return m_CurrentSide; }
			void SetCurrentSide(Side side) { m_CurrentSide = side; }
	
			/** Accessors */
			map< string, list<MetaData>* > *GetContainerList() { return &m_containerListMap; }
			map< string, list<MetaData>* >::iterator *GetCurrentContainerIterator() { return &m_currentContainerIterator;}
	
			list<MetaData> *GetElementList() { return m_currentElementList; }
			list<MetaData>::iterator *GetCurrentElementIterator() { return &m_currentElementIterator; }
	
		private:
			map< string, list<MetaData>* > m_containerListMap;
			map< string, list<MetaData>* >::iterator m_currentContainerIterator;
			list<MetaData> *m_currentElementList;
			list<MetaData>::iterator m_currentElementIterator;
			Side m_CurrentSide;

			void ProcessGenre(MetaData *metadata);
			void AddToGenre(MetaData *metadata, char *strGenre);

			void LoadPlayListURIIntoCurrentElementList(char *strFileName);
	};
#endif
