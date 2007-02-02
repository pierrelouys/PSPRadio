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
#ifndef _PSPUSBSTORAGE_
	#define _PSPUSBSTORAGE_
	class CPSPUSBStorage
	{
		public:
			CPSPUSBStorage(CPSPApp *pspapp);
			~CPSPUSBStorage();
			int  EnableUSB();
			int  DisableUSB();
			bool IsUSBEnabled() { return m_USBEnabled; }
	
		private:
			bool m_USBEnabled;
			CPSPApp *m_PSPApp;
			CPSPThread *m_thDriverLoader;
	};
	
#endif
