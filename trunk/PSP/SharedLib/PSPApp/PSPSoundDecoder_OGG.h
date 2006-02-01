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
#ifndef __PSPSOUNDDECODER_OGG__
	#define __PSPSOUNDDECODER_OGG__
	
	//#define USE_TREMOR
	
	#include "PSPSound.h"
	#include "PSPSoundDecoder.h"
	#ifdef USE_TREMOR
		#include <ivorbisfile.h>
	#else
		#include <vorbis/codec.h>
		#include <vorbis/vorbisfile.h>
	#endif
	
	class CPSPEventQ;
	class CPSPSoundStream;
	class CPSPSoundBuffer;
	
	class COGGStreamReader : public CPSPStreamReader
	{
	public:
		
		COGGStreamReader(CPSPStream *InputStream);//:CPSPStreamReader(InputStream);
		~COGGStreamReader();
		
		void Close();
		size_t Read(unsigned char *pBuffer, size_t SizeInBytes);
		void ReadComments(); /** Retrieve/Process metadata from the stream */
		void COGGStreamReader::ProcessInfo(); /** Get/Process Stream info */
		
	private:
		CPSPStream *m_InputStream;
		OggVorbis_File m_vf;
		int m_last_section;
		CLock *m_lock;
	};
	
	class CPSPSoundDecoder_OGG : public IPSPSoundDecoder
	{
	public:
		CPSPSoundDecoder_OGG(CPSPSoundBuffer *OutputBuffer, CPSPStream *InputStream) : IPSPSoundDecoder(OutputBuffer, InputStream) { Initialize(); }
		~CPSPSoundDecoder_OGG();
		
		void Initialize();
			
		bool Decode();

	private:
		unsigned char	*m_pInputBuffer;
		unsigned char	*m_GuardPtr;
		unsigned long	m_FrameCount;
	};
	
	#include "PSPSound.h"
#endif
