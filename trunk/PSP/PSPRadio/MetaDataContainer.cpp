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
#include <stdio.h>
#include <Logging.h>
#include "MetaDataContainer.h"

void CorrectHTTPString(char *strSrc)
{
	char *strSrcPtr = strSrc;
	int chr = 0;
	
	if (strSrcPtr[0] != 0)
	{
		char *strDest = strdup(strSrc);
		char *strDestPtr = strDest;
	
		while (strSrcPtr[0] != 0)
		{
			if (0 == strncmp(strSrcPtr, "&#", 2))
			{
				sscanf(strSrcPtr, "&#%o;", &chr);
				strDestPtr[0] = (char)chr;
				strSrcPtr = strchr(strSrcPtr, ';');
				if (strSrcPtr)
				{
					strSrcPtr++;
				}
			}
			else if (0 == strncmp(strSrcPtr, "&amp;", 5))
			{
				strDestPtr[0] = '&';
				strSrcPtr += 5;
			}
			else
			{
				strDestPtr[0] = strSrcPtr[0];
				strSrcPtr++;
			}
			strDestPtr++;
		}
		strDestPtr[0] = 0; 
		
		strcpy(strSrc, strDest);
		
		free(strDest), strDest = NULL;
	}
}


bool SortMetaDataByURI(MetaData &a, MetaData &b)
{
	return strcmp(a.strURI, b.strURI) < 0;
}

bool SortMetaDataByTitle(MetaData &a, MetaData &b)
{
	return strcmp(a.strTitle, b.strTitle) < 0;
}

CMetaDataContainer::CMetaDataContainer()
{
	m_currentContainerIterator = m_containerListMap.begin();
	m_currentElementList = NULL;
	//m_currentElementIterator = m_currentElementList.begin();
}

CMetaDataContainer::~CMetaDataContainer()
{
	Clear();
}

void CMetaDataContainer::Clear()
{
	if (false == m_containerListMap.empty())
	{
		
		for (m_currentContainerIterator = m_containerListMap.begin(); 
			m_currentContainerIterator != m_containerListMap.end(); 	
			m_currentContainerIterator++)
		{
			m_currentElementList = m_currentContainerIterator->second;
			while(false == m_currentElementList->empty())
			{
				m_currentElementList->pop_front();
			}
		}
	}

	m_containerListMap.clear();
	m_currentContainerIterator = m_containerListMap.begin();
	m_currentElementList = NULL;
}
	
void CMetaDataContainer::NextContainer()
{
	if (false == m_containerListMap.empty())
	{
		m_currentContainerIterator++;
		if (m_currentContainerIterator == m_containerListMap.end())
		{
			m_currentContainerIterator = m_containerListMap.begin();
		}
	}
}

void CMetaDataContainer::AssociateElementList()
{
	if (false == m_containerListMap.empty())
	{
		m_currentElementList = m_currentContainerIterator->second;
		m_currentElementIterator = m_currentElementList->begin();
	}
}

void CMetaDataContainer::PrevContainer()
{
	if (false == m_containerListMap.empty())
	{
		if (m_currentContainerIterator == m_containerListMap.begin())
		{
			m_currentContainerIterator = m_containerListMap.end();
		}
		m_currentContainerIterator--;
	}	
}

void CMetaDataContainer::NextGlobalElement()
{
	if ((NULL != m_currentElementList) && (false == m_currentElementList->empty()))
	{
		m_currentElementIterator++;
		if (m_currentElementIterator == m_currentElementList->end())
		{
			NextContainer();
			AssociateElementList();
		}
	}
}

void CMetaDataContainer::PrevGlobalElement()
{
	if ((NULL != m_currentElementList) && (false == m_currentElementList->empty()))
	{
		if (m_currentElementIterator == m_currentElementList->begin())
		{
			PrevContainer();
			AssociateElementList();
		}
		else
		{
			m_currentElementIterator--;
		}
	}
}

void CMetaDataContainer::NextElement()
{
	if ((NULL != m_currentElementList) && (false == m_currentElementList->empty()))
	{
		m_currentElementIterator++;
		if (m_currentElementIterator == m_currentElementList->end())
		{
			m_currentElementIterator = m_currentElementList->begin();
		}
	}
}

void CMetaDataContainer::PrevElement()
{
	if ((NULL != m_currentElementList) && (false == m_currentElementList->empty()))
	{
		if (m_currentElementIterator == m_currentElementList->begin())
		{
			m_currentElementIterator = m_currentElementList->end();
		}
		m_currentElementIterator--;
	}
}
	

#define SHOUTXML_URI_TAG 					"<entry Playstring=\""
#define SHOUTXML_TITLE_TAG					"<Name>"
#define SHOUTXML_GENRE_TAG					"<Genre>"
#define XML_END_TAG							"</"
void CMetaDataContainer::LoadSHOUTcastXML(char *strFileName)
{
	FILE *fd = NULL;
	char strLine[256];
	int iLines = 0;
	MetaData *songdata;
	char strURI[256];
	char strTitle[256];
	char strGenre[128];
	int iCount = 0;
	
	enum shoutcastxml_states
	{
		WAITING_FOR_URI,
		WAITING_FOR_TITLE,
		WAITING_FOR_GENRE,
	} shoutxml_state = WAITING_FOR_URI;
	
	songdata = new MetaData;
	
	fd = fopen(strFileName, "r");
	
	if(fd != NULL)
	{
		while ( !feof(fd) )
		{
			strLine[0] = 0;
			fgets(strLine, 256, fd);
			if (strlen(strLine) == 0 || strLine[0] == '\r' || strLine[0] == '\n')
			{
				continue;
			}
			if (strLine[0] == '<' && strLine[1] == '&')
			{
				/** A comment!, ignore */
				continue;
			}
			
			strLine[strlen(strLine)-1] = 0; /** Remove LF 0D*/
			if (strLine[strlen(strLine)-1] == 0x0D) 
				strLine[strlen(strLine)-1] = 0; /** Remove CR 0A*/
			
			/** This shouldn't happen at this point, but it won't hurt */
			if (0 == strlen(strLine))
			{
				continue;
			}
			/** We have a line with data here */
			
			//Log(LOG_VERYLOW, "line(%d) strLine='%s'", iLines, strLine);
			char *Tag;
			switch(shoutxml_state)
			{
			/* ie '<entry Playstring="http://www.shoutcast.com/sbin/tunein-station.pls?id=3281&amp;filename=playlist.pls">'*/
				case WAITING_FOR_URI: 
					Tag = strstr(strLine, SHOUTXML_URI_TAG);
					if (Tag)
					{
						strcpy(strURI, Tag + strlen(SHOUTXML_URI_TAG));
				//		Log(LOG_VERYLOW, "line(%d) strLine='%s' strURI = '%s'", iLines, strLine, strURI);
						if(strchr(strURI, '"'))
						{
							*strchr(strURI, '"') = 0;
						}
						shoutxml_state = WAITING_FOR_TITLE;
					}
					break;
			/* ie '     <Name>CLUB 977 The Hitz Channel (HIGH BANDWIDTH)</Name>' */
				case WAITING_FOR_TITLE: 
					Tag = strstr(strLine, SHOUTXML_TITLE_TAG);
					if (Tag)
					{
						strcpy(strTitle, Tag + strlen(SHOUTXML_TITLE_TAG));
				//		Log(LOG_VERYLOW, "line(%d) strLine='%s' strTitle = '%s'", iLines, strLine, strURI);
						
						/* Terminate the string where the end tag is */
						Tag = strstr(strTitle, XML_END_TAG);
						if (Tag)
						{
							Tag[0] = 0;
						}
						
						CorrectHTTPString(strTitle);
						
						shoutxml_state = WAITING_FOR_GENRE;
					}
					break;
			/* i.e '     <Genre>Pop Rock Top 40</Genre>' */
				case WAITING_FOR_GENRE: 
					Tag = strstr(strLine, SHOUTXML_GENRE_TAG);
					if (Tag)
					{
						strcpy(strGenre, Tag + strlen(SHOUTXML_GENRE_TAG));
				//		Log(LOG_VERYLOW, "line(%d) strLine='%s' strTitle = '%s'", iLines, strLine, strURI);
						
						Tag = strstr(strGenre, XML_END_TAG);
						if (Tag)
						{
							Tag[0] = 0;
						}

						/** Good!, all fields for this entry aquired, let's insert in the list! */
						memset(songdata, 0, sizeof(MetaData));
						//Log(LOG_LOWLEVEL, "Adding SHOUTcast Entry: URI='%s' Title='%s' Genre='%s' to the list.", 
						//	strURI, strTitle, strGenre);
						memcpy(songdata->strURI,  strURI,  256);
						songdata->strURI[255] = 0;
						memcpy(songdata->strTitle, strTitle, 256);
						songdata->strTitle[255] = 0;
						memcpy(songdata->strGenre, strGenre, 128);
						songdata->strGenre[127] = 0;
						
						if (0 == strlen(songdata->strGenre))
						{
							strcpy(songdata->strGenre, "Other");
						}

						ProcessGenre(songdata);

						iCount++;

						shoutxml_state = WAITING_FOR_URI;
					}
					break;					
			}
			
			iLines++;
		}
		fclose(fd), fd = NULL;
		
		/** Sort Element List (starting from second 'genre' as top 600 list should be sorted by popularity)*/
		m_currentContainerIterator = m_containerListMap.begin();
		m_currentContainerIterator++; /** Skip 600 listing */
		for (; m_currentContainerIterator != m_containerListMap.end(); m_currentContainerIterator++)
		{
			m_currentElementList = m_currentContainerIterator->second;
			if (m_currentElementList)
			{
				m_currentElementList->sort(SortMetaDataByTitle);
			}
		}
		
		m_currentContainerIterator = m_containerListMap.begin();
		m_currentElementList = m_currentContainerIterator->second;
		if (m_currentElementList)	
		{
			m_currentElementIterator = m_currentElementList->begin();
		}

	}
	else
	{
		ReportError("Unable to open XML file '%s'", strFileName);
	}
	
	if (songdata)
	{
		delete(songdata), songdata = NULL;
	}

	Log(LOG_INFO, "LoadSHOUTcastXML(): Processed %d Elements", 
		iCount);

}

void CMetaDataContainer::ProcessGenre(MetaData *metadata)
{
/// Genres: "New Age" "News" "Talk" "Pop" "Oldies" "70s" "80s" "90s" "Anime" "Country"
	char *strGenre = strdup(metadata->strGenre);

	strlwr(strGenre); /** Convert the string to lower case */
	strGenre[0] = toupper(strGenre[0]);
	
	/** All go here */
	AddToGenre(metadata, "**All Top 600**");

	if (strstr(strGenre, "70s"))
		AddToGenre(metadata, "70s");

	if (strstr(strGenre, "80s"))
		AddToGenre(metadata, "80s");

	if (strstr(strGenre, "90s"))
		AddToGenre(metadata, "90s");

	if (strstr(strGenre, "new age"))
		AddToGenre(metadata, "New Age");

	if (strstr(strGenre, "news"))
		AddToGenre(metadata, "News");

	if (strstr(strGenre, "talk"))
		AddToGenre(metadata, "Talk");

	if (strstr(strGenre, "pop"))
		AddToGenre(metadata, "Pop");

	if (strstr(strGenre, "dance"))
		AddToGenre(metadata, "Dance");

	if (strstr(strGenre, "trance"))
		AddToGenre(metadata, "Trance");

	if (strstr(strGenre, "anime"))
		AddToGenre(metadata, "Anime");

	if (strstr(strGenre, "oldies"))
		AddToGenre(metadata, "Oldies");

	if (strstr(strGenre, "country"))
		AddToGenre(metadata, "Country");

	if (strstr(strGenre, "50s"))
		AddToGenre(metadata, "Oldies");

	if (strstr(strGenre, "60s"))
		AddToGenre(metadata, "Oldies");

	if (strstr(strGenre, "rock"))
		AddToGenre(metadata, "Rock");

	if (strstr(strGenre, "classical"))
		AddToGenre(metadata, "Classical");

	if (strstr(strGenre, "alternative"))
		AddToGenre(metadata, "Alternative");

	if (strstr(strGenre, "acoustic"))
		AddToGenre(metadata, "Ambient");

	if (strstr(strGenre, "ambient"))
		AddToGenre(metadata, "Ambient");

	if (strstr(strGenre, "americana"))
		AddToGenre(metadata, "Americana");

	if (strstr(strGenre, "jazz"))
		AddToGenre(metadata, "Jazz");

	if (strstr(strGenre, "blues"))
		AddToGenre(metadata, "Blues");

	if (strstr(strGenre, "house"))
		AddToGenre(metadata, "House");

	if (strstr(strGenre, "christian"))
		AddToGenre(metadata, "Christian");

	if (strstr(strGenre, "christmas"))
		AddToGenre(metadata, "Holidays");

	if (strstr(strGenre, "rnb "))
		AddToGenre(metadata, "R&B");

	if (strstr(strGenre, "r&b"))
		AddToGenre(metadata, "R&B");

	if (strstr(strGenre, "comedy"))
		AddToGenre(metadata, "Talk");

	if (strstr(strGenre, "techno"))
		AddToGenre(metadata, "Techno");

	if (strstr(strGenre, "soundtrack"))
		AddToGenre(metadata, "Soundtrack");

	if (strstr(strGenre, "folk"))
		AddToGenre(metadata, "Folk");

	if (strstr(strGenre, "game"))
		AddToGenre(metadata, "Video Games");

	if (strstr(strGenre, "gospel"))
		AddToGenre(metadata, "Christian");

	if (strstr(strGenre, "hardcore"))
		AddToGenre(metadata, "Hardcore");

	if (strstr(strGenre, "hip hop"))
		AddToGenre(metadata, "Hip Hop");

	if (strstr(strGenre, "rap"))
		AddToGenre(metadata, "Rap");

	if (strstr(strGenre, "indie"))
		AddToGenre(metadata, "Indie");

	if (strstr(strGenre, "live"))
		AddToGenre(metadata, "Live");

	if (strstr(strGenre, "metal"))
		AddToGenre(metadata, "Metal");

	if (strstr(strGenre, "progressive"))
		AddToGenre(metadata, "Progressive");

	if (strstr(strGenre, "r&amp;b"))
		AddToGenre(metadata, "R&B");

	if (strstr(strGenre, "reggae"))
		AddToGenre(metadata, "Reggae");

	if (strstr(strGenre, "latin"))
		AddToGenre(metadata, "Latin");

	if (strstr(strGenre, "punk"))
		AddToGenre(metadata, "Punk");

	if (strstr(strGenre, "sport"))
		AddToGenre(metadata, "Sports");

	if (strstr(strGenre, "salsa"))
		AddToGenre(metadata, "Latin");

	if (strstr(strGenre, "c64"))
		AddToGenre(metadata, "Video Games");

	if (strstr(strGenre, "industrial"))
		AddToGenre(metadata, "Industrial");

	if (strstr(strGenre, "blues"))
		AddToGenre(metadata, "Blues");

	free(strGenre), strGenre = NULL;
}

void CMetaDataContainer::AddToGenre(MetaData *metadata, char *strGenre)
{
	map<string, list<MetaData>* >::iterator foundGenre;
	foundGenre = m_containerListMap.find(strGenre);
	
	if (foundGenre == m_containerListMap.end()) /** Didn't find genre, create it */
	{
		m_currentElementList = new list<MetaData>;
		/** Add new genre to the container map */
		m_containerListMap.insert( map<string, list<MetaData>* >::value_type(strGenre, m_currentElementList) );
	}
	else
	{
		m_currentElementList = foundGenre->second;
	}
	
	/** Search for the station in the metadata (by Title) in this genre list */
	list<MetaData>::iterator foundStation;
	for(foundStation = m_currentElementList->begin(); (foundStation != m_currentElementList->end()); foundStation++) 
	{
 		if (stricmp((*foundStation).strTitle, metadata->strTitle) == 0)
		{
			break;
		}
	};
	
	
	if (foundStation == m_currentElementList->end()) /** Didn't find station under this genre, add */
	{
		/** Insert stream information */
		metadata->iItemIndex = m_currentElementList->size(); /** jpf added unique id for list item */
		m_currentElementList->push_back(*metadata);
	}
}

/** This method is used by the LocalFilesScreen */
void CMetaDataContainer::LoadDirectory(char *strPath)
{
	char strFilename[MAXPATHLEN];
	int dfd;
	SceIoDirent direntry;
	
	Log(LOG_LOWLEVEL, "LoadDirectory: Reading '%s' Directory", strPath);
	dfd = sceIoDopen(strPath);
	
	/** Get all directories */
	if (dfd > 0)
	{
		//Log(LOG_LOWLEVEL, "CDirList::LoadDir(): Calling sceIoDread(%d,0x%x).",dfd, &direntry);
		/** RC 10-10-2005: The direntry has to be memset! Or else the app will/may crash! */
		memset(&direntry, 0, sizeof(SceIoDirent));
		while(sceIoDread(dfd, &direntry) > 0)
		{
			//Log(LOG_LOWLEVEL, "Processing '%s'", direntry.d_name)
			if((direntry.d_stat.st_attr & FIO_SO_IFDIR)) /** It's a file */
			{
				if (strcmp(direntry.d_name, ".") == 0)
					continue;
				else if (strcmp(direntry.d_name, "..") == 0)
					continue;
				memset(strFilename, 0, MAXPATHLEN);
				sprintf(strFilename, "%s/%s", strPath, direntry.d_name);
 				Log(LOG_LOWLEVEL, "LoadDirectory(): Adding '%s' to list of containers.", strFilename);
				//mydata->iItemIndex = m_dirlist.size(); /** jpf added unique id for list item */
				map<string, list<MetaData>* >::iterator found;
				found = m_containerListMap.find(strFilename);
				
				if (found == m_containerListMap.end()) /** Didn't find it */
				{
					m_currentElementList = new list<MetaData>;
					/** Add new genre to the container map */
					m_containerListMap.insert( map<string, list<MetaData>* >::value_type(strFilename, m_currentElementList) );
				}
				else
				{
					m_currentElementList = found->second;
				}

				/** Populate Listbox */
				LoadFilesIntoCurrentElementList(strFilename);
				
			}
		}
		sceIoDclose(dfd);
		
		/** If list is not empty: */
		if (false == m_containerListMap.empty())
		{
			m_currentContainerIterator = m_containerListMap.begin();
			m_currentElementList = m_currentContainerIterator->second;
			if (m_currentElementList)	
			{
				m_currentElementIterator = m_currentElementList->begin();
			}
		}
	}
	else
	{
		Log(LOG_ERROR, "Unable to open '%s' Directory!", strPath);
	}
}

void CMetaDataContainer::LoadFilesIntoCurrentElementList(char *dirname)
{
	char strFilename[MAXPATHLEN];
	int dfd;
	SceIoDirent direntry;
	MetaData *songdata;
	
	songdata = new MetaData;
	
	Log(LOG_LOWLEVEL, "LoadFilesIntoCurrentElementList: Reading '%s' Directory", dirname);
	dfd = sceIoDopen(dirname);
	
	/** Get all files */
	if (dfd > 0)
	{
		//Log(LOG_LOWLEVEL, "CDirList::LoadDir(): Calling sceIoDread(%d,0x%x).",dfd, &direntry);
		/** RC 10-10-2005: The direntry has to be memset! Or else the app will/may crash! */
		memset(&direntry, 0, sizeof(SceIoDirent));
		while(sceIoDread(dfd, &direntry) > 0)
		{
			if(false == (direntry.d_stat.st_attr & FIO_SO_IFDIR)) /** It's a file */
			{
				memset(strFilename, 0, MAXPATHLEN);
				sprintf(strFilename, "%s/%s", dirname, direntry.d_name);
 				Log(LOG_LOWLEVEL, "LoadFilesIntoCurrentElementList(): Adding '%s' to list.", strFilename);
				//mydata->iItemIndex = m_dirlist.size(); /** jpf added unique id for list item */
				memset(songdata, 0, sizeof(MetaData));
				memcpy(songdata->strURI,  strFilename,  256);
				songdata->iItemIndex = m_currentElementList->size(); /** jpf added unique id for list item */
				m_currentElementList->push_back(*songdata);

			}
		}
		sceIoDclose(dfd);
		
		/** Sort Element List */
		m_currentElementList->sort(SortMetaDataByURI);
		
		m_currentContainerIterator = m_containerListMap.begin();
		m_currentElementList = m_currentContainerIterator->second;
		if (m_currentElementList)	
		{
			m_currentElementIterator = m_currentElementList->begin();
		}
		delete(songdata); songdata = NULL;
	}
	else
	{
		Log(LOG_ERROR, "Unable to open '%s' Directory!", dirname);
	}
}

void CMetaDataContainer::LoadPlaylistsFromDirectory(char *strDirName)
{
	char strFilename[MAXPATHLEN];
	int dfd;
	SceIoDirent direntry;
	
	Log(LOG_LOWLEVEL, "Reading '%s' Directory", strDirName);
	dfd = sceIoDopen(strDirName);

	if (dfd > 0)
	{
		//Log(LOG_LOWLEVEL, "CDirList::LoadDir(): Calling sceIoDread(%d,0x%x).",dfd, &direntry);
		/** RC 10-10-2005: The direntry has to be memset! Or else the app will/may crash! */
		memset(&direntry, 0, sizeof(SceIoDirent));
		while(sceIoDread(dfd, &direntry) > 0)
		{
			if(false == (direntry.d_stat.st_attr & FIO_SO_IFDIR)) /** It's a file */
			{
				memset(strFilename, 0, MAXPATHLEN);
				sprintf(strFilename, "%s/%s", strDirName, direntry.d_name);
 				Log(LOG_LOWLEVEL, "CDirList::LoadDir(): Adding '%s' to list.", strFilename);
				//mydata->iItemIndex = m_dirlist.size(); /** jpf added unique id for list item */
				map<string, list<MetaData>* >::iterator found;
				found = m_containerListMap.find(strFilename);
				
				if (found == m_containerListMap.end()) /** Didn't find it */
				{
					m_currentElementList = new list<MetaData>;
					/** Add new genre to the container map */
					m_containerListMap.insert( map<string, list<MetaData>* >::value_type(strFilename, m_currentElementList) );
				}
				else
				{
					m_currentElementList = found->second;
				}
				//songdata->iItemIndex = m_currentElementList->size(); /** jpf added unique id for list item */

				/** Populate Listbox */
				LoadPlayListURIIntoCurrentElementList(strFilename);

			}
		}
		sceIoDclose(dfd);
		m_currentContainerIterator = m_containerListMap.begin();
		m_currentElementList = m_currentContainerIterator->second;
		if (m_currentElementList)	
		{
			m_currentElementIterator = m_currentElementList->begin();
		}
	}
	else
	{
		Log(LOG_ERROR, "Unable to open '%s' Directory!", strDirName);
	}
}

#define PLV2_NUMBER_OF_ENTRIES_TAG			"numberofentries="
#define PLV2_NUMBER_OF_ENTRIES_PARSING_STR	"numberofentries=%d"
#define PLV2_FILE_TAG						"FileX="
#define PLV2_FILE_PARSING_STR				"File%d=%s"
#define PLV2_TITLE_TAG						"TitleX="
#define PLV2_TITLE_PARSING_STR				"Title%d=%s"
#define PLV2_LENGTH_TAG						"LengthX="
#define PLV2_LENGTH_PARSING_STR				"Length%d=%d"
void CMetaDataContainer::LoadPlayListURIIntoCurrentElementList(char *strFileName)
{
	FILE *fd = NULL;
	char strLine[256];
	int iLines = 0;
	int iFormatVersion = 1;
	MetaData *songdata;
	bool fStopParsing = false;
	
	int iV2_numberofentries = 0;
	int iV2_IgnoredValue = 0; /** Used during parsing */
	int iV2_ParsingTemp = 0;
	char strV2_File[256];
	char strV2_Title[256];
	int  iV2_Length = -1;
	
	enum playlistv2_states
	{
		WAITING_FOR_NUM_OF_ENTRIES,
		WAITING_FOR_FILE,
		WAITING_FOR_TITLE,
		WAITING_FOR_LENGTH,
	} v2_state = WAITING_FOR_FILE;//WAITING_FOR_NUM_OF_ENTRIES;
	
	songdata = new MetaData;
	
	fd = fopen(strFileName, "r");
	
	if(fd != NULL)
	{
		while ( (!feof(fd)) && (false == fStopParsing) )
		{
			strLine[0] = 0;
			fgets(strLine, 256, fd);
			if (strlen(strLine) == 0 || strLine[0] == '\r' || strLine[0] == '\n')
			{
				continue;
			}
			if ((iLines == 0) && (strLine[0] == '['))
			{
				iFormatVersion = 2;
				
				continue;
			}
			if (strLine[0] == ';')
			{
				/** A comment!, ignore */
				continue;
			}
			
			strLine[strlen(strLine)-1] = 0; /** Remove LF 0D*/
			if (strLine[strlen(strLine)-1] == 0x0D) 
				strLine[strlen(strLine)-1] = 0; /** Remove CR 0A*/
			
			/** We have a line with data here */
			
			switch(iFormatVersion)
			{
				case 1:
					memset(songdata, 0, sizeof(MetaData));
					memcpy(songdata->strURI, strLine, 256);
					songdata->iItemIndex = m_currentElementList->size(); /** jpf added unique id for list item */					
					m_currentElementList->push_back(*songdata);
					Log(LOG_LOWLEVEL, "Adding '%s' to the list.", strLine);
					break;
				case 2:
					if ( ('n' == strLine[0]) || ('N' == strLine[0]) ) /** Number of entry entry */
						continue;
					if ( ('v' == strLine[0]) || ('V' == strLine[0]) ) /** Version entry */
						continue;
					//Log(LOG_VERYLOW, "Line=%d strLine='%s' v2_state=%d", iLines, strLine, v2_state);
					switch(v2_state)
					{
						case WAITING_FOR_NUM_OF_ENTRIES: /* ie numberofentries=5 */
							if (strlen(strLine) > strlen(PLV2_NUMBER_OF_ENTRIES_TAG))
							{
								iV2_ParsingTemp = sscanf(strLine, PLV2_NUMBER_OF_ENTRIES_PARSING_STR, &iV2_numberofentries);
								if (1 == iV2_ParsingTemp)
								{
									v2_state = WAITING_FOR_FILE;
								}
								else
								{
									fStopParsing = true;
								}
							}
							else
							{
								fStopParsing = true;
							}
							break;
						case WAITING_FOR_FILE: /* ie File1=http://64.236.34.196:80/stream/1040 */
							if (strlen(strLine) > strlen(PLV2_FILE_TAG))
							{
								strV2_File[0] = 0;
								//iV2_ParsingTemp = sscanf(strLine, PLV2_FILE_PARSING_STR, &iV2_IgnoredValue, strV2_File);
								if (strstr(strLine, "File"))
								{
									if (strchr(strLine, '='))
									{
										strncpy(strV2_File, strchr(strLine, '=') + 1, 256);
									}
								}
								if (strlen(strV2_File))
								{
									v2_state = WAITING_FOR_TITLE;
								}
								else
								{
									fStopParsing = true;
								}
							}
							else
							{
								fStopParsing = true;
							}
							break;
						case WAITING_FOR_TITLE: /* ie Title1=(#1 - 524/21672) CLUB 977 The 80s Channel */
							if (strlen(strLine) > strlen(PLV2_TITLE_TAG))
							{
								//iV2_ParsingTemp = sscanf(strLine, PLV2_TITLE_PARSING_STR, &iV2_IgnoredValue, strV2_Title);
								strV2_Title[0] = 0;
								if (strchr(strLine, '='))
								{
									strncpy(strV2_Title, strchr(strLine, '=') + 1, 256);
								}
								if (strlen(strV2_Title))
								{
									v2_state = WAITING_FOR_LENGTH;
								}
								else
								{
									fStopParsing = true;
								}
							}
							else
							{
								fStopParsing = true;
							}
							break;
						case WAITING_FOR_LENGTH: /* Length1=-1 */
							if (strlen(strLine) > strlen(PLV2_LENGTH_TAG))
							{
								iV2_ParsingTemp = sscanf(strLine, PLV2_LENGTH_PARSING_STR, &iV2_IgnoredValue, &iV2_Length);
								if (2 == iV2_ParsingTemp)
								{
									/** Good!, all fields for this entry aquired, let's insert in the list! */
									memset(songdata, 0, sizeof(MetaData));
									Log(LOG_LOWLEVEL, "Adding V2 Entry: File='%s' Title='%s' Length='%i' to the list.", 
										strV2_File, strV2_Title, iV2_Length);
									memcpy(songdata->strURI,  strV2_File,  256);
									memcpy(songdata->strTitle, strV2_Title, 256);
									songdata->iLength = iV2_Length;
									songdata->iItemIndex = m_currentElementList->size(); /** jpf added unique id for list item */
									m_currentElementList->push_back(*songdata);
									//Log(LOG_INFO, "Added V2 Entry: File='%s' Title='%s' Length='%s' to the list.", 
									//	strV2_File, strV2_Title, iV2_Length);
									iV2_numberofentries--;
									
									v2_state = WAITING_FOR_FILE;
								}
								else
								{
									fStopParsing = true;
								}
							}
							else
							{
								fStopParsing = true;
							}
							break;
					}
					if ( (true == fStopParsing) )//&& (iV2_numberofentries > 0) )
					{
						Log(LOG_ERROR, "Malformed playlist found. Error on Line %d (%s)", iLines+1, strLine);
					}
					break;
			}
			
			//if ( (2 == iFormatVersion) && (0 == iV2_numberofentries) ) /** Done! */
			//{
			//	break;
			//}
			
			iLines++;
		}
		fclose(fd), fd = NULL;
		
		m_currentElementIterator = m_currentElementList->begin();
		
		/** Sort Element List */
		m_currentElementList->sort(SortMetaDataByURI);
	}
	else
	{
		ReportError("Unable to open playlist '%s'", strFileName);
	}
	
	if (songdata)
	{
		delete(songdata), songdata = NULL;
	}
}

