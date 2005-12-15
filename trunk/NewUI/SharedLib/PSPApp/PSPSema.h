/* 
	PSPApp C++ OO Application Framework. (Initial Release: Sept. 2005)
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
#ifndef __PSPAPP_SEMA__
	#define __PSPAPP_SEMA__
	
	class CSema
	{
	public:
		CSema(char *strName, int iInitialValue=1, int iMaxCount=255, int iWaitVal = 1)
				{ m_mutex = sceKernelCreateSema(strName, 0, iInitialValue, iMaxCount, 0); 
				  m_iWait = iWaitVal; }
		~CSema() { sceKernelDeleteSema(m_mutex); }
		
		void Wait() {	sceKernelWaitSema(m_mutex, m_iWait, NULL); }
		void Up() { 	sceKernelSignalSema(m_mutex, 1); }
		void Down() { 	sceKernelSignalSema(m_mutex, -1); }
		
		int GetMutex() { return m_mutex; }
		
	protected:
		int m_mutex;
		int m_iWait;
	};
	
	class CLock : public CSema
	{
	public:
		CLock(char *strName):CSema(strName, 1, 255,1){}
		
		void Lock()   {	Wait(); };
		void Unlock() { Up(); }
	};
	
	class CBlocker : public CSema
	{
	public:
		CBlocker(char *strName):CSema(strName, 0,1,1){}
		
		void Block() {	Wait(); };
		void UnBlock() { Up(); }
	};

#endif
