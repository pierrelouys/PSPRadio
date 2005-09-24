#include <new>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "Logging.h"

CLogging::CLogging()
{
	m_strFilename = NULL;
	m_LogLevel = LOG_INFO;
}

CLogging::~CLogging()
{
	if (m_strFilename)
	{
		free(m_strFilename), m_strFilename = NULL;
	}
}

int CLogging::Set(char *strLogFilename, loglevel_enum iLogLevel)
{
	int iRes = 0;
	if (strLogFilename)
	{
		m_strFilename = strdup(strLogFilename);
		m_LogLevel = iLogLevel;
		iRes = 0;
	}
	else
	{
		iRes = -1;
	}
	
	return iRes;
	
}

void CLogging::SetLevel(loglevel_enum iNewLevel)
{
	m_LogLevel = iNewLevel;
}

int CLogging::Log(char *strModuleName, loglevel_enum LogLevel, char *strFormat, ...)
{
	
	return 0;
}


