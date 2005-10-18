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
#ifndef __PSPSOUNDDECODER_MAD__
	#define __PSPSOUNDDECODER_MAD__
	
	#include "PSPSound.h"
	#include "PSPSoundDecoder.h"
	#include <mad.h>
	
	class CPSPEventQ;
	class CPSPSoundBuffer;
	
	class CPSPSoundDecoder_MAD : public IPSPSoundDecoder
	{
	public:
		CPSPSoundDecoder_MAD(CPSPSoundBuffer *OutputBuffer)
			:IPSPSoundDecoder(OutputBuffer){ Initialize();}
		~CPSPSoundDecoder_MAD();
		
		void Initialize();
			
		bool Decode();

	private:
		struct mad_frame	m_Frame;
		struct mad_stream	m_Stream;
		struct mad_synth	m_Synth;
		mad_timer_t			m_Timer;
		unsigned char		*m_pInputBuffer;
		unsigned char		*m_GuardPtr;
		unsigned long		m_FrameCount;

		
		static signed int scale(mad_fixed_t &sample);
		static int PrintFrameInfo(struct mad_header *Header);
		static signed short MadFixedToSshort(mad_fixed_t Fixed);
	};
	
	#include "PSPSound.h"
#endif
