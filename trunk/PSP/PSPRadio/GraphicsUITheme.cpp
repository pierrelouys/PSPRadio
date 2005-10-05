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
#include <PSPSound_MP3.h>
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <malloc.h>
#include <iniparser.h>
#include <Tools.h>
#include <stdarg.h>
#include <Logging.h>
#include "GraphicsUITheme.h"

CGraphicsUITheme::CGraphicsUITheme() : m_pIniTheme(NULL)
{
}

CGraphicsUITheme::~CGraphicsUITheme()
{
	Terminate();
}

int CGraphicsUITheme::Initialize(char *szFilename)
{
	if(NULL != m_pIniTheme)
	{
		Log(LOG_ERROR, "Initialize: Theme initialized try calling terminate first!");	
		return -1;
	}
	
	Log(LOG_LOWLEVEL, "Initialize: creating ini parser");
	m_pIniTheme = new CIniParser(szFilename);
	Log(LOG_LOWLEVEL, "Initialize: created ini parser");
		
	if(NULL == m_pIniTheme)
	{
		Log(LOG_ERROR, "Initialize: error creating theme!");	
		return -1;
	}
	
	return 0;
}

void CGraphicsUITheme::Terminate()
{
	if(NULL != m_pIniTheme)
	{
		delete m_pIniTheme;
		m_pIniTheme = NULL;
	}
}

int CGraphicsUITheme::GetItem(char *szIniTag, CGraphicsUIThemeItem *pItem)
{
	char szIniTagAndItem[50];
	char *szTemp = NULL;
	int nCount = 0;
	
	/** Make sure INI Parser is initialized **/
	if(NULL == m_pIniTheme)
	{
		Log(LOG_ERROR, "GetItem: error m_pIniThem is NULL!");	
		return -1;
	}
	
	/** Make sure pItem is valid **/
	if(NULL == pItem)
	{
		Log(LOG_ERROR, "GetItem: error pItem is NULL!");	
		return -1;
	}
	
	/** Get the Source Points **/
	sprintf(szIniTagAndItem, "%s:%s", szIniTag, "src");
	szTemp = m_pIniTheme->GetStr(szIniTagAndItem);	
	nCount = StringToPointList(szTemp, &pItem->m_pointSrcList);
	
	/** Get the Destination Point **/
	sprintf(szIniTagAndItem, "%s:%s", szIniTag, "dst");
	szTemp = m_pIniTheme->GetStr(szIniTagAndItem);	
	nCount = StringToPoint(szTemp, &pItem->m_pointDst);
	
	/** Get the Size Point **/
	sprintf(szIniTagAndItem, "%s:%s", szIniTag, "size");
	szTemp = m_pIniTheme->GetStr(szIniTagAndItem);	
	nCount = StringToPoint(szTemp, &pItem->m_pointSize);
	
	return 0;
}

int CGraphicsUITheme::GetImagePath(char *szImagePath, int nLength)
{
	if(NULL == szImagePath)
	{
		Log(LOG_ERROR, "GetImagePath: error szImagePath is NULL!");	
		return -1;
	}
	
	char *szTemp = m_pIniTheme->GetStr("image:path");
	
	if(strlen(szTemp) > nLength)
	{
		Log(LOG_ERROR, "GetImagePath: input string not long enough!");	
		return -1;
	}
	
	strcpy(szImagePath, szTemp);	

	return 0;
}

int CGraphicsUITheme::StringToPoint(char *szPoint, Point *pPoint)
{
	int x, y;
	int rc; 
	
	if(NULL == pPoint)
	{
		Log(LOG_ERROR, "StringToPoint: error pPoint is NULL");
		return -1;
	}
	
	rc = sscanf(szPoint, "[%d,%d]", &x, &y);		
	
	if(2 == rc)
	{
		pPoint->x = x;
		pPoint->y = y;
		return 1;
	}
	else
	{
		Log(LOG_ERROR, "StringToPoint: error scanning point %s", szPoint);
	}
	
	return 0;
}

int CGraphicsUITheme::StringToPointList(char *szPoint, map<int, Point> *pPointList)
{
	char *token = strtok(szPoint, " ");
	int nCount = 0;
	
	if(NULL == pPointList)
	{
		Log(LOG_ERROR, "StringToPointList: error pPointList is NULL");
		return -1;
	}
	
	while(NULL != token)
	{
		Point pointTemp;
		
		if(1 == StringToPoint(token, &pointTemp))
		{
			(*pPointList)[nCount] = pointTemp;
			//pPointList->push_back(pointTemp);
			nCount++;
		}	
			
		token = strtok(NULL, " ");
	}
	
	return nCount;
}




