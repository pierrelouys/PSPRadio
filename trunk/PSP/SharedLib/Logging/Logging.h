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
#ifndef _CLOGGINGH_
#define _CLOGGINGH_

#include <pspkernel.h>
#include <pspkerneltypes.h>


enum loglevel_enum
{
	LOG_LOWLEVEL = 10,
	LOG_INFO	 = 50,
	LOG_ERROR	 = 80,
	LOG_ALWAYS	 = 100
};

class CLock
{
public:
	CLock(char *strName){ m_mutex = sceKernelCreateSema(strName, 0, 1, 10, 0);}
	~CLock() { sceKernelDeleteSema(m_mutex); }
	
	void Lock() {	sceKernelSignalSema(m_mutex, -1); sceKernelWaitSema(m_mutex, 0, 0); };
	void Unlock() { 	sceKernelSignalSema(m_mutex, 1); }
private:
	int m_mutex;
};


class CLogging
{
public:
	CLogging();
	~CLogging();

	int Set(char *strLogFilename, loglevel_enum iLogLevel);
	void SetLevel(loglevel_enum iNewLevel);
	
	int Log_(char *strModuleName, loglevel_enum LogLevel, char *strFormat, ...);
	
private:
	char *m_strFilename;
	loglevel_enum m_LogLevel;
	FILE *m_fp;
	CLock *m_lock;
	
	/** fflush() doesn't work, so reopen for now */
	void Open();
	void Close();
	
};

//#define Log(level, format, args...) pPSPApp->m_Log.Log("PSPRadio", level, format, ## args)
extern CLogging Logging;
#define Log(level, format, args...) Logging.Log_(__FILE__, level, format, ## args)

#endif
