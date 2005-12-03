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
#ifndef __PSPSOUNDBUFFER__
	#define __PSPSOUNDBUFFER__
	
	#include "PSPSound.h"
	
	/** Configurable */
	/* (frames are 2ch, 16bits, so 4096frames =16384bytes =8192samples-l-r-combined.)*/
	//#define PSP_BUFFER_SIZE_IN_FRAMES	PSP_AUDIO_SAMPLE_ALIGN(4096)	
	#define PSP_BUFFER_SIZE_IN_FRAMES	PSP_AUDIO_SAMPLE_ALIGN(2048)	
	//Now configurable: #define NUM_BUFFERS 			20	
	#define DEFAULT_NUM_BUFFERS		20		/** Default */
	
	#define INPUT_BUFFER_SIZE		16302
	
	/** Internal use */
	struct FrameTransport
	{
		Frame frame;
		bool  bIsLastFrame;
	};
	
	class CPSPSoundBuffer
	{
	public:
		CPSPSoundBuffer();	
		~CPSPSoundBuffer();
		/*Takes the number of PSP sound buffers 20~100. If not changed, defaults to DEFAULT_NUM_BUFFERS.*/
		void  ChangeBufferSize(size_t buffer_size); 
		void  PushFrame(Frame &frame); /* Takes 1 frame for any samplerate, set samplate first. */
		void  Push44Frame(Frame &frame); /* Takes 1 frame for a 44100Hz stream */
		Frame PopFrame(); 			/* Returns 1 frame */
		
		Frame *PopDeviceBuffer();			/* Returns PSP_BUFFER_SIZE_IN_FRAMES frames */
		size_t GetBufferFillPercentage();
		void  Empty();
		void  Done();
		bool  IsDone();
		
		void SampleRateChange(int newRate);
		
	private:
		FrameTransport  *m_RingBuffer;
		Frame			*m_DeviceBuffer; /** One PSP sound buffer */
		size_t			m_PopIndex, m_PushIndex;
		bool  			m_bBuffering;
		size_t 	/*m_samplerate,*/ m_mult, m_div;
		size_t 	m_FrameCount; /** Used for buffer percentage */
		size_t	m_NumBuffers; /** Configurable via config-file. Should be from 20 - 100 or so.. test! */
		size_t  m_RingBufferSize; /** Number of frames in the ring buffer */
		Frame   m_EmptyFrame;
		void AllocateBuffers(); /** Called by constructor/ChangeBufferSize() to (re)allocate buffers */

	};
	
#endif
