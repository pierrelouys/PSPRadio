/* 
	Logging Library for the PSP. (Initial Release: Sept. 2005)
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
#include <new>
#include <stdio.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include "Logging.h"

CLogging Logging;
//static char *m_strFilename = NULL;
//static FILE *m_fp = NULL;

CLogging::CLogging()
{
	m_strFilename = NULL;
	m_LogLevel = LOG_INFO;
	m_fp = NULL;
	m_lock = new CLock("Logging_Lock");
}

CLogging::~CLogging()
{
	if (m_strFilename)
	{
		free(m_strFilename), m_strFilename = NULL;
	}
	if (m_fp)
	{
		fclose(m_fp), m_fp = NULL;
	}
	delete m_lock;
	
}

int CLogging::Set(char *strLogFilename, loglevel_enum iLogLevel)
{
	int iRes = 0;
	if (strLogFilename && m_fp == NULL)
	{
		m_strFilename = strdup(strLogFilename);
		m_LogLevel = iLogLevel;
		
		Open();
		if (m_fp)
		{
			//fprintf(m_fp, "File opened successfully!\n");
			Close();
			iRes = 0;
		}
		else
		{
			iRes = -1;
		}
	}
	else
	{
		iRes = -1;
	}
	
	return iRes;
	
}

void CLogging::Open()
{
	if (m_fp)
	{
		Close();
	}
	m_fp = fopen(m_strFilename, "a");
}

void CLogging::Close()
{
	if (m_fp)
	{
//		fflush(m_fp);
		fclose(m_fp);
		m_fp = NULL;
	}
}	

void CLogging::SetLevel(loglevel_enum iNewLevel)
{
	m_LogLevel = iNewLevel;
}

int CLogging::Log_(char *strModuleName, loglevel_enum LogLevel, char *strFormat, ...)
{
	va_list args;
	
	m_lock->Lock();
	if (LogLevel >= m_LogLevel)
	{
		va_start (args, strFormat);         /* Initialize the argument list. */
		
		Open();
		//m_fp = fopen("PSPRadio.log", "a");
		if (m_fp)
		{
			fprintf(m_fp, "%s<%d>: ", strModuleName, LogLevel);
			vfprintf(m_fp, strFormat, args);
			if (strFormat[strlen(strFormat) - 1] != '\n');
			{
				fprintf(m_fp, "\r\n");
			}
		}
		//fclose(m_fp);
		Close();
		
		va_end (args);                  /* Clean up. */
	}
	m_lock->Unlock();
	
	return 0;
}


