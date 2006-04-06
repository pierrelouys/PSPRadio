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
	#include "PSPSoundDeviceBuffer.h"

	class CPSPSoundBuffer
	{
	public:
		CPSPSoundBuffer();
		~CPSPSoundBuffer();
		/*Takes the number of PSP sound buffers 20~100. If not changed, defaults to DEFAULT_NUM_BUFFERS.*/
		void  ChangeBufferSize(size_t buffer_size);
		size_t GetBufferFillPercentage();
		void  Empty();
		void  SampleRateChange(int newRate);

		/** Pushed by Decoder */
		void  PushPCMFrame(Frame &frame); /* Takes 1 frame for any samplerate, set samplate first. */

	protected:
		friend class CPSPSound;
		DeviceBuffer *PopDeviceBuffer();

	private:
		std::list<DeviceBuffer> m_DeviceBufferList;
		DeviceBuffer		*m_DeviceBuffer; /** One PSP sound buffer */
		DeviceBuffer   		*m_EmptyDeviceBuffer;
		bool  				m_bBuffering;
		size_t 	/*m_samplerate,*/ m_mult, m_div;
		size_t 	m_DeviceBufferCount; /** Used for buffer percentage */
		size_t	m_LastDeviceBufferCount; /** Last buffer count reported to PSPApp */
		size_t	m_NumBuffers; /** Configurable via config-file. Should be from 20 - 100 or so.. test! */
		size_t  m_BufferListMaxSize; /** Number of frames in the ring buffer */
	};

#endif
