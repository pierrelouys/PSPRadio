/* 
	PSP Tools. (Initial Release: Sept. 2005)
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
#include <string.h> 


char *dirname (char *strPath)
{
	if (strPath[0] == '/' && strPath[1] == 0)
		return strPath;
	
	if (strPath && strPath[0] != 0 && strlen(strPath) > 4)
	{
		if (strrchr(strPath, '/'))
			*(strrchr(strPath, '/')) = 0;
		else
			strcpy(strPath, ".");
	}
	else
	{
			strcpy(strPath, ".");
	}
	
	return strPath;
}

char *basename (char *strPath)
{
	static char *pRet;
	
	pRet = "";
	
	if (strPath && strlen(strPath) <  4) /** 4 = length of 'ms0:' */
	{
		pRet = "";
	}
	else
	{
		pRet = strrchr(strPath, '/') + 1;
		if (NULL == pRet)
		{
			pRet = "";
		}
	}
	
	return pRet;
}

/** NEED TO IMPLEMENT! */
char *getpass(const char *__prompt)
{
	printf(__prompt);
	return "password";
}

void herror (__const char *__str)
{
}
