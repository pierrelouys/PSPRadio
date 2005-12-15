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
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <malloc.h>
#include <Tools.h>
#include <PSPApp.h>
#include <Logging.h>
#include <pspiofilemgr_dirent.h>
#include "DirList.h"

#define ReportError pPSPApp->ReportError

CDirList::CDirList()
{
	m_diriterator = m_dirlist.begin();
};

CDirList::~CDirList()
{
	Clear();
};

void CDirList::Next()
{
	m_diriterator++;
	if (m_diriterator == m_dirlist.end())
	{
		m_diriterator = m_dirlist.begin();
	}
}

void CDirList::Prev()
{
	if (m_diriterator == m_dirlist.begin())
	{
		m_diriterator = m_dirlist.end();
	}
	m_diriterator--;
}

int CDirList::Size()
{
	return m_dirlist.size();
}

void CDirList::InsertURI(char *strFileName)
{
	directorydata dirdata;
	
	Log(LOG_INFO, "Adding '%s' to the list.", strFileName);
	memset(&dirdata, 0, sizeof(dirdata));
	strncpy(dirdata.strURI, strFileName, MAXPATHLEN);
	dirdata.iItemIndex = m_dirlist.size(); /** jpf added unique id for list item */
	m_dirlist.push_back(dirdata);
	
	m_diriterator = m_dirlist.begin();
}

void CDirList::LoadDirectory(char *strDirName)
{
	directorydata *mydata = NULL;
	int dfd;
	SceIoDirent direntry;
	
	mydata = new directorydata;
	Log(LOG_LOWLEVEL, "Reading '%s' Directory", strDirName);
	dfd = sceIoDopen(strDirName);

	if (dfd > 0)
	{
		//Log(LOG_LOWLEVEL, "CDirList::LoadDir(): Calling sceIoDread(%d,0x%x).",dfd, &direntry);
		/** RC 10-10-2005: The direntry has to be memset! Or else the app will/may crash! */
		memset(&direntry, 0, sizeof(SceIoDirent));
		while(sceIoDread(dfd, &direntry) > 0)
		{
			memset(mydata, 0, sizeof(directorydata));
			if(false == (direntry.d_stat.st_attr & FIO_SO_IFDIR)) /** It's a file */
			{
				sprintf(mydata->strURI, "%s/%s", strDirName, direntry.d_name);
 				Log(LOG_LOWLEVEL, "CDirList::LoadDir(): Adding '%s' to list.", mydata->strURI);
				mydata->iItemIndex = m_dirlist.size(); /** jpf added unique id for list item */
				m_dirlist.push_back(*mydata);
			}
		}
		sceIoDclose(dfd);
		m_diriterator = m_dirlist.begin();	
	}
	else
	{
		Log(LOG_ERROR, "Unable to open '%s' Directory!", strDirName);
	}
	
	delete (mydata);
}

void CDirList::Clear()
{
	while(m_dirlist.size())
	{
		m_dirlist.pop_front();
	}
	m_diriterator = m_dirlist.begin();
};

