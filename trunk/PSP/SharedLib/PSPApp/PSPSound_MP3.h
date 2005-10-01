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
#ifndef PSPSOUND
#define __PSPSOUND__
	#include "PSPSound.h"
	#include <mad.h>
	
	//extern CPSPSound_MP3 *pPSPSound_MP3;
	
	class CPSPSound_MP3 : public CPSPSound
	{
	public:
		CPSPSound_MP3();
		
		
		int SendMessage(int iMessageId, void *pMessage = NULL, int iSenderId = SID_PSPSOUND_MP3)
			{ return pPSPApp->OnMessage(iMessageId, pMessage, iSenderId); };


	protected:
		virtual void Decode(); /** 'Thread' */

	private:
		static signed int scale(mad_fixed_t &sample);
		static int PrintFrameInfo(struct mad_header *Header);
		static signed short MadFixedToSshort(mad_fixed_t Fixed);
	};
	
#endif
