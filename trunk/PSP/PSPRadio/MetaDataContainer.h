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
	
	class CMetaDataContainerIndexer
	{
		public:
			CMetaDataContainerIndexer(map< string, list<MetaData>* > *ContainerListMap)
				{   m_pContainerListMap = ContainerListMap; 
					m_ContainerIterator = m_pContainerListMap->begin();
					m_ElementList = NULL;  
					AssociateElementList(); }
		
			/** Navigation */
			void NextContainer();
			void PrevContainer();
			void AssociateElementList();
			/** Local elements */
			void NextElement();
			void PrevElement();
			/** Global */
			void NextGlobalElement();
			void PrevGlobalElement();
			
			map< string, list<MetaData>* >::iterator *GetContainerIterator() 
					{ return &m_ContainerIterator;}
			list<MetaData>::iterator *GetElementIterator() 
					{ return &m_ElementIterator; }
			list<MetaData> *GetElementList() { return m_ElementList; }
			void SetElementList(list<MetaData> *List) { m_ElementList = List; }
					
			
		public:
			map< string, list<MetaData>* > *m_pContainerListMap;
			
			map< string, list<MetaData>* >::iterator m_ContainerIterator;
			list<MetaData> *m_ElementList;
			list<MetaData>::iterator m_ElementIterator;
	};

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
			
			/** Population */
			void LoadDirectory(char *strPath);
			void LoadPlaylistsFromDirectory(char *strDirectory);
			int LoadSHOUTcastXML(char *strFileName);
			int LoadNewSHOUTcastXML(char *strFileName);
			

			Side GetCurrentSide() { return m_CurrentSide; }
			void SetCurrentSide(Side side) { m_CurrentSide = side; }
	
			/** Accessors */
			map< string, list<MetaData>* > *GetContainerList() { return &m_containerListMap; }
	
			/** Selected track accesssors */
			CMetaDataContainerIndexer *GetCurrentSelectionIndexer() { return m_CurrentSelectionIndexer; }
			list<MetaData> *GetElementList() { return m_CurrentSelectionIndexer->GetElementList(); }
			list<MetaData> &GetCurrentElementListRef() { return *m_CurrentSelectionIndexer->GetElementList(); }
			map< string, list<MetaData>* >::iterator *GetCurrentContainerIterator() 
					{ return m_CurrentSelectionIndexer->GetContainerIterator();}
			list<MetaData>::iterator *GetCurrentElementIterator() 
					{ return m_CurrentSelectionIndexer->GetElementIterator(); }
					
			map< string, list<MetaData>* >::iterator &GetCurrentContainerIteratorRef() 
					{ return *m_CurrentSelectionIndexer->GetContainerIterator();}
			list<MetaData>::iterator &GetCurrentElementIteratorRef() 
					{ return *m_CurrentSelectionIndexer->GetElementIterator(); }
			
					
			/** Playing track accessors */
			CMetaDataContainerIndexer *GetPlayingTrackIndexer() { return m_PlayingTrackIndexer; }
			map< string, list<MetaData>* >::iterator *GetPlayingContainerIterator() 
					{ return m_PlayingTrackIndexer->GetContainerIterator();}
			list<MetaData>::iterator *GetPlayingElementIterator() 
					{ return m_PlayingTrackIndexer->GetElementIterator(); }
					
					
			void SetPlayingToSelection()
			{
				m_PlayingTrackIndexer->m_pContainerListMap 	= m_CurrentSelectionIndexer->m_pContainerListMap;
				m_PlayingTrackIndexer->m_ContainerIterator	= m_CurrentSelectionIndexer->m_ContainerIterator;
				m_PlayingTrackIndexer->m_ElementList		= m_CurrentSelectionIndexer->m_ElementList;
				m_PlayingTrackIndexer->m_ElementIterator 	= m_CurrentSelectionIndexer->m_ElementIterator;
			}
		
		private:
			CMetaDataContainerIndexer   *m_CurrentSelectionIndexer;
			CMetaDataContainerIndexer   *m_PlayingTrackIndexer;
			map< string, list<MetaData>* > m_containerListMap;
			Side m_CurrentSide;

			void ProcessGenre(MetaData *metadata);
			void AddToGenre(MetaData *metadata, char *strGenre);

			void LoadPlayListURIIntoCurrentElementList(char *strFileName);
			void LoadFilesIntoCurrentElementList(char *dirname);
	};
	

#endif
