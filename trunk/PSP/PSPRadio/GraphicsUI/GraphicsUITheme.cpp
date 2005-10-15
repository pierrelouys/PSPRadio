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
	
	Log(LOG_VERYLOW, "Initialize: creating ini parser");
	m_pIniTheme = new CIniParser(szFilename);
	Log(LOG_VERYLOW, "Initialize: created ini parser");
		
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
	if(0 < strlen(szTemp))
	{
		nCount = StringToPointMap(szTemp, &pItem->m_pointSrcMap);
	}
	
	/** Get the Destination Point **/
	sprintf(szIniTagAndItem, "%s:%s", szIniTag, "dst");
	szTemp = m_pIniTheme->GetStr(szIniTagAndItem);	
	if(0 < strlen(szTemp))
	{
		nCount = StringToPoint(szTemp, &pItem->m_pointDst);
	}
	
	/** Get the Size Point **/
	sprintf(szIniTagAndItem, "%s:%s", szIniTag, "size");
	szTemp = m_pIniTheme->GetStr(szIniTagAndItem);	
	if(0 < strlen(szTemp))
	{
		nCount = StringToPoint(szTemp, &pItem->m_pointSize);
	}
	
	/** Get the Key Index Map **/
	sprintf(szIniTagAndItem, "%s:%s", szIniTag, "keys");
	szTemp = m_pIniTheme->GetStr(szIniTagAndItem);	
	if(0 < strlen(szTemp))
	{
		nCount = StringToKeyIndexMap(szTemp, &pItem->m_keyToIndexMap);	
		Log(LOG_VERYLOW, "GetItem: Keys = %s", szTemp);
	}
	
	return 0;
}

int CGraphicsUITheme::GetPosItem(char *szIniTag, CGraphicsUIPosItem *pItem)
{
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
	szTemp = m_pIniTheme->GetStr(szIniTag);	
	if(0 < strlen(szTemp))
	{
		nCount = StringToPosItem(szTemp, pItem);
	}
	
	/** Make sure we read 2 Points from string */
	if(nCount == 2)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}


int CGraphicsUITheme::GetLettersAndNumbers(char *szIniTagLetters, 
											char *szIniTagNumbers,
											CGraphicsUIThemeItem *pItem)
{
	CGraphicsUIThemeItem itemNumbers;
	int nCount = 0;
	
	/** Get the letters first */
	if(-1 == GetItem(szIniTagLetters, pItem))
	{
		Log(LOG_ERROR, "GetLettersAndNumbers: error getting base tag for %s", szIniTagLetters);
		return -1;
	}
	
	/** Get the numbers */
	if(-1 == GetItem(szIniTagNumbers, &itemNumbers))
	{
		Log(LOG_ERROR, "GetLettersAndNumbers: error getting base tag for %s", szIniTagNumbers);
		return -1;
	}
	
	/** Check to make sure the size of our numbers is the same as our letters */
	if((itemNumbers.m_pointSize.x != pItem->m_pointSize.x) ||
		(itemNumbers.m_pointSize.y != pItem->m_pointSize.y))
	{
		Log(LOG_ERROR, "GetLettersAndNumbers: error GUI does not support size diffs for letters and numbers");
		return -1;
	}	
		
	/** Greate the remaining letter items */
	Point baseSrc = pItem->GetSrc(0);
	Point baseSize = pItem->m_pointSize;
	
	for(size_t x = 0; x != pItem->m_keyToIndexMap.size(); x++)
	{
		Point newSrc;
		newSrc.x = baseSrc.x + (baseSize.x * x);
		newSrc.y = baseSrc.y;
		
		pItem->m_pointSrcMap[nCount] = newSrc;
		nCount++;
	}	
	
	/** Greate the remaining number items */
	baseSrc = itemNumbers.GetSrc(0);
	
	for(size_t x = 0; x != itemNumbers.m_keyToIndexMap.size(); x++)
	{
		Point newSrc;
		newSrc.x = baseSrc.x + (baseSize.x * x);
		newSrc.y = baseSrc.y;
		
		pItem->m_pointSrcMap[nCount] = newSrc;
		
		nCount++;
	}	
	
	/** Update the indexes in the numbers key map */
	map<char, int>::iterator iter = itemNumbers.m_keyToIndexMap.begin();;
	
	while(iter != itemNumbers.m_keyToIndexMap.end())
	{
		int index = iter->second;		
		iter->second = index + pItem->m_keyToIndexMap.size();	
		iter++;
	}
	
	/** Add the numbers keys to the number/letters key map */
	pItem->m_keyToIndexMap.insert(itemNumbers.m_keyToIndexMap.begin(), itemNumbers.m_keyToIndexMap.end());
	
	return 0;
}

int CGraphicsUITheme::GetImagePath(char *szImagePath, int nLength)
{
	if(NULL == szImagePath)
	{
		Log(LOG_ERROR, "GetImagePath: error szImagePath is NULL!");	
		return -1;
	}
	
	char *szTemp = m_pIniTheme->GetStr("image:file");
	
	if(strlen(szTemp) > (size_t)nLength)
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

int CGraphicsUITheme::StringToPointMap(char *szPoint, map<int, Point> *pPointMap)
{
	char *token = strtok(szPoint, " ");
	int nCount = 0;
	
	if(NULL == pPointMap)
	{
		Log(LOG_ERROR, "StringToPointMap: error pPointMap is NULL");
		return -1;
	}
	
	while(NULL != token)
	{
		Point pointTemp;
		
		if(1 == StringToPoint(token, &pointTemp))
		{
			(*pPointMap)[nCount] = pointTemp;
			nCount++;
		}	
			
		token = strtok(NULL, " ");
	}
	
	return nCount;
}

int CGraphicsUITheme::StringToKeyIndexMap(char *szKey, map<char, int> *pKeyMap)
{
	int nCount = strlen(szKey);
	
	if(NULL == pKeyMap)
	{
		Log(LOG_ERROR, "StringToKeyIndexMap: error pKeyMap is NULL");
		return -1;
	}
	
	for(int x = 0; x != nCount; x++)
	{
		Log(LOG_VERYLOW, "StringToKeyIndexMap: adding (%c) to %d", szKey[x], x);		
		(*pKeyMap)[szKey[x]] = x;
	}
	
	return nCount;
}

int CGraphicsUITheme::StringToPosItem(char *szPos, CGraphicsUIPosItem *pPosItem)
{
	char *token = strtok(szPos, " ");
	int nCount = 0;
	
	if(NULL == pPosItem)
	{
		Log(LOG_ERROR, "StringToPosItem: error pPosItem is NULL");
		return -1;
	}
	
	if(NULL != token)
	{
		Point pointTemp;
		if(1 == StringToPoint(token, &pointTemp))
		{
			memcpy(&pPosItem->m_pointDst, &pointTemp, sizeof(Point));
			nCount++;			
		}	
		
		token = strtok(NULL, "");
		
		if(NULL != token)
		{
			if(1 == StringToPoint(token, &pointTemp))
			{
				memcpy(&pPosItem->m_pointSize, &pointTemp, sizeof(Point));
				nCount++;			
			}	
		}		
	}
	
	return nCount;
}
