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
	LOG_VERYLOW  = 10,
	LOG_LOWLEVEL = 20,
	LOG_INFO	 = 50,
	LOG_ERROR	 = 80,
	LOG_ALWAYS	 = 100
};

class CLock
{
public:
	CLock(char *strName){ m_mutex = sceKernelCreateSema(strName, 0, 1, 255, 0); }
	~CLock() { sceKernelDeleteSema(m_mutex); }
	
	int Lock() {	return sceKernelWaitSema(m_mutex, 1, NULL); };
	int Unlock() { 	return sceKernelSignalSema(m_mutex, 1); }
private:
	int m_mutex;
	SceUInt m_timeout;
};


class CSema
{
public:
	CSema(char *strName){ m_mutex = sceKernelCreateSema(strName, 0, 1, 255, 0); }
	~CSema() { sceKernelDeleteSema(m_mutex); }
	
	void Wait() {	sceKernelWaitSema(m_mutex, 1, NULL); };
	void Up() { 	sceKernelSignalSema(m_mutex, 1); }
	void Down() { 	sceKernelSignalSema(m_mutex, -1); }
private:
	int m_mutex;
};


/** Can't get the kernel semaphore to work right, so for now implementing in user-space.
	(This will only work for simple things like what m_ExitSema is doing, though, so
	 be careful)
*/
/*
class CSema
{
public:
	CSema(char *strName){ m_count = 0; m_timeout = 20*10;}//10 secs
	~CSema() { }
	
	void Wait() { while(m_count > 0 && m_timeout) { sceKernelDelayThread(50000); m_timeout--;}; m_timeout = 20*10;} //10 secs
	void Up()   { m_count++;  }
	void Down() { if (m_count > 0) m_count--; }
private:
	int m_count;
	int m_timeout;
};
*/

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
