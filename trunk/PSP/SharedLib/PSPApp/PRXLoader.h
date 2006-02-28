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
#ifndef _PRXLOADER_
	#define _PRXLOADER_
	
	class CPRXLoader
	{
	public:
		CPRXLoader();
		~CPRXLoader();
		int Load(char *filename);
		int Unload();
		int Start(int argc = 0, char * const argv[] = NULL);
		bool IsLoaded()  { return m_ModId > 0; }
		bool IsStarted() { return m_IsStarted; }
		int  GetError()  { return m_error; }
	
	private:
		SceUID StartModuleWithArgs(char *filename, int modid, int argc, char * const argv[]);
		SceUID m_ModId;
		int  m_error;
		char *m_FileName;
		bool m_IsStarted;
	};

#endif
