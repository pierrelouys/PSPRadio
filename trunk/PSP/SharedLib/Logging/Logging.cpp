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
#include <time.h>
#include <pspkernel.h>
#include <psprtc.h>
#include <psputility_sysparam.h>
#include "PSPApp.h"
#include "Logging.h"

CLogging *pLogging = NULL;

void InstantiateLogging()
{
	pLogging = new CLogging;
}

CLogging::CLogging()
{
	m_strFilename = NULL;
	m_LogLevel = LOG_INFO;
	m_fp = NULL;
	m_lock = new CLock("LogLock");
	m_msg = (char *) malloc(4096); /** A message this big would fill up the whole screen of the psp if sent to the screen. */
	m_sock = -1;
	m_pPSPApp = NULL;
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
	if(m_lock)
	{
		delete m_lock;
	}
	if (m_msg)
	{
		free (m_msg), m_msg = NULL;
	}
	if (m_sock != -1)
	{
		close(m_sock);
		m_sock = -1;
	}
}

int CLogging::Set(char *strLogFilename, loglevel_enum iLogLevel)
{
	int iRes = 0;
	if (strLogFilename && (NULL == m_strFilename) ) //m_fp == NULL)
	{
		m_strFilename = strdup(strLogFilename);
		m_LogLevel = iLogLevel;

		Open();
		if (m_fp)
		{
			//fprintf(m_fp, "File opened successfully!\n");
			Close();

			/** Remove log file - so we start with a fresh one every time */
			sceIoRemove(m_strFilename);
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

void CLogging::EnableWiFiLogging(char *server, char *port)
{
	in_addr addr;
	int rc = 0;

	Log_(__FILE__, __LINE__, LOG_INFO, "WifiLog:Enabling WiFi logging.");

	if (m_sock != -1)
	{
		close(m_sock);
		m_sock = -1;
	}

	memset(&addr, 0, sizeof(in_addr));

	Log_(__FILE__, __LINE__, LOG_INFO, "WifiLog:Resolving server='%s'", server);
	if (m_pPSPApp)
	{
		rc = m_pPSPApp->ResolveHostname(server, &addr);
	}
	else
	{
		Log_(__FILE__, __LINE__, LOG_ERROR, "WifiLog:Could not resolve server. PSPApp not set.!\n");
		return;
	}
	if (rc < 0)
	{
		Log_(__FILE__, __LINE__, LOG_ERROR, "WifiLog:Could not resolve server!\n");
		return;
	}
	Log_(__FILE__, __LINE__, LOG_INFO, "WifiLog:aton/ntoa succeeded, returned addr='0x%x'", addr);

	if ((m_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		Log_(__FILE__, __LINE__, LOG_ERROR, "WifiLog:Couldn't create socket");
	}
	else
	{
		u32  timeo;
		timeo = 3 *1000*1000; /** timeout is in microseconds */
		if (setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(timeo)) < 0)
		{
			Log_(__FILE__, __LINE__, LOG_ERROR, "WiFiLog:setsockopt SO_RCVTIMEO Failed");
		}
		if (setsockopt(m_sock, SOL_SOCKET, SO_SNDTIMEO, &timeo, sizeof(timeo)) < 0)
		{
			Log_(__FILE__, __LINE__, LOG_ERROR, "WiFiLog:setsockopt SO_SNDTIMEO Failed");
		}

		memset(&m_sin, 0, sizeof(struct sockaddr_in));
		m_sin.sin_family = AF_INET;
		m_sin.sin_len = sizeof(struct sockaddr_in);
		memcpy(&m_sin.sin_addr, &addr, sizeof(in_addr));
        m_sin.sin_port = htons(atoi(port));
	}
}

void CLogging::DisableWiFiLogging()
{
	Log_(__FILE__, __LINE__, LOG_INFO, "WifiLog:Disabling WiFi logging.");
	if (m_sock != -1)
	{
		close(m_sock);
		m_sock = -1;
	}
}

void CLogging::WifiLog(char *message)
{
	int	 length = strlen(message);

	if (sendto(m_sock, message, length, 0, (struct sockaddr *) &m_sin, sizeof(sockaddr_in)) != length)
	{
		Log_(__FILE__, __LINE__, LOG_ERROR, "WiFiLog:Couldn't send datagram");
	}
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

int CLogging::Log_(char *strModuleName, int iLineNo, loglevel_enum LogLevel, char *strFormat, ...)
{
	va_list args;
	char logmessage[1024];

	m_lock->Lock();
	if (m_strFilename && LogLevel >= m_LogLevel) /** Log only if Set() was called and loglevel is correct */
	{
		va_start (args, strFormat);         /* Initialize the argument list. */
		pspTime local_time;

		sceRtcGetCurrentClockLocalTime(&local_time);

		/* create message to log */
		sprintf(logmessage, "%02d:%02d:%02d.%03d:%s@%d<%d>: ",
							local_time.hour,
							local_time.minutes,
							local_time.seconds,
							(int)(local_time.microseconds/1000),
							strModuleName, iLineNo, LogLevel);

		vsprintf(m_msg, strFormat, args);
		if (m_msg[strlen(m_msg)-1] == 0x0A)
			m_msg[strlen(m_msg)-1] = 0; /** Remove LF 0D*/
		if (m_msg[strlen(m_msg)-1] == 0x0D)
			m_msg[strlen(m_msg)-1] = 0; /** Remove CR 0A*/

		strcat(logmessage, m_msg);

		if (m_sock == -1)
		{
			Open();
			if (m_fp)
			{
				fprintf(m_fp, "%s\r\n", logmessage);
			}
			Close();
		}
		else
		{
			WifiLog(logmessage);
		}
		va_end (args);                  /* Clean up. */
	}
	m_lock->Unlock();

	return 0;
}
