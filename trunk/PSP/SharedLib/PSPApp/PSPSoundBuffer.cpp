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
#include <list>
#include <PSPApp.h>
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <mad.h>
#include <malloc.h>
#include <errno.h>
#include "PSPSound.h"
#include "PSPSoundBuffer.h"

using namespace std;

/** Sound buffer class implementation */
CPSPSoundBuffer::CPSPSoundBuffer()
{
	/** Initialize */
	m_RingBuffer = NULL;
	m_DeviceBuffer = NULL; /** One PSP sound buffer */
	m_PopIndex = m_PushIndex = 0;
	m_bBuffering = true;
	m_mult = m_div = 1;
	m_FrameCount = 0; /** Used for buffer percentage */
	m_NumBuffers = DEFAULT_NUM_BUFFERS; /** Configurable via config-file. Should be from 20 - 100 or so.. test! */
	m_RingBufferSize = (PSP_BUFFER_SIZE_IN_FRAMES*m_NumBuffers*15); /** Number of frames in the ring buffer */
	memset(&m_EmptyFrame, 0, sizeof(Frame));
	
	AllocateBuffers();
	
}

CPSPSoundBuffer::~CPSPSoundBuffer()
{
	if (m_RingBuffer)
	{
		delete[] m_RingBuffer, m_RingBuffer=NULL;
	}
	if (m_DeviceBuffer)
	{
		free(m_DeviceBuffer), m_DeviceBuffer = NULL;
	}
	
}


void CPSPSoundBuffer::AllocateBuffers()
{
	Log(LOG_VERYLOW, "AllocateBuffers() called");
	if (m_RingBuffer)
	{
		delete[] m_RingBuffer, m_RingBuffer=NULL;
	}
	if (m_DeviceBuffer)
	{
		free(m_DeviceBuffer), m_DeviceBuffer = NULL;
	}
	
	m_RingBufferSize = (PSP_BUFFER_SIZE_IN_FRAMES*m_NumBuffers);//*15); /** Number of frames in the ring buffer; 15 to accomodate for different samplerates */
	
	m_RingBuffer = new FrameTransport[m_RingBufferSize]; 
	
	m_DeviceBuffer = (Frame *)memalign(64, FRAMES_TO_BYTES(PSP_BUFFER_SIZE_IN_FRAMES));
	
	Empty();
}

/*Takes the number of PSP sound buffers 20~100. If not changed, defaults to DEFAULT_NUM_BUFFERS.*/
void CPSPSoundBuffer::ChangeBufferSize(size_t buffer_size) 
{
	Log(LOG_VERYLOW, "ChangeBufferSize(%d) Called.", buffer_size);
	m_NumBuffers = buffer_size;
	AllocateBuffers();
}

void  CPSPSoundBuffer::Empty() 
{ 
	Log(LOG_VERYLOW, "CPSPSoundBuffer::Empty() Called.");
	for (size_t i = 0; i < m_RingBufferSize ; i++)
	{
		memset(&(m_RingBuffer[i].frame), 0, sizeof(Frame));
		m_RingBuffer[i].bIsLastFrame = false;
	}
	m_PopIndex = m_PushIndex = 0;
	m_bBuffering = true;
	m_mult = m_div = 1;
	m_FrameCount = 0; /** Used for buffer percentage */
	
}

size_t CPSPSoundBuffer::GetBufferFillPercentage() 
{ 
	
	return 100*m_FrameCount/m_RingBufferSize;
};
		
void CPSPSoundBuffer::SampleRateChange(int newRate)
{
	if (newRate > 0)
	{
		m_mult = 1, m_div = 1;
		switch(newRate)
		{
		case 8000:
			m_mult=11;
			m_div = 2;
			break;
		case 11025:
			m_mult=4;
			m_div =1;
			break;
		case 16000:
			m_mult=11;
			m_div = 4;
			break;
		case 22050:
			m_mult=2;
			m_div =1;
			break;
		case 24000:
			m_mult=11;
			m_div =6;
			break;
		case 32000:
			m_mult=11;
			m_div = 8;
			break;
		case 44100:
			m_mult=1;
			m_div =1;
			break;
		case 47250:
			break;
		case 48000:
			m_mult=11;
			m_div=12;
			break;
		}
	}
}

/** This is called by the decoder for every decoded PCM frame */
void CPSPSoundBuffer::PushFrame(Frame &frame) /** Push a frame at an arbitrary samplerate */
{
	static size_t iDiv = 0;
	for (size_t iMult = 0; iMult < m_mult; iMult++)
	{
		iDiv++;
		if ((iDiv % m_div) == 0)
		{
			Push44Frame(frame);
		}
	}
	
}

void CPSPSoundBuffer::Push44Frame(Frame &frame) /** Push a frame from a 44.1KHz stream (PSP's samplerate) */
{
	static int timeLastPercentEvent = 500; /** Initialize to something so the percentage event is sent the first time */
	
	while ( (100 == GetBufferFillPercentage()) && (0 == pPSPSound->GetEventToDecThSize()) )
	{ 
		sceKernelDelayThread(50); /** 50us */
	}
	
	m_FrameCount++;
	
	m_PushIndex++;
	if (m_PushIndex > m_RingBufferSize)
		m_PushIndex = 0;
		
	
	memcpy(&(m_RingBuffer[m_PushIndex].frame), &frame, sizeof(Frame));
	m_RingBuffer[m_PushIndex].bIsLastFrame = false;
	
	if ((clock()*1000/CLOCKS_PER_SEC - timeLastPercentEvent) > 333) /** 3 times per sec */
	{
		pPSPSound->SendEvent(MID_BUFF_PERCENT_UPDATE);
		timeLastPercentEvent = clock()*1000/CLOCKS_PER_SEC;
	}
}


Frame *CPSPSoundBuffer::PopDeviceBuffer()
{
	for (int i = 0 ; i < PSP_BUFFER_SIZE_IN_FRAMES; i++)
	{
		m_DeviceBuffer[i] = PopFrame();
	}
	return m_DeviceBuffer;
}

Frame CPSPSoundBuffer::PopFrame()
{
	static Frame sFrame;
	
	if (true == IsDone())
	{
		Log(LOG_VERYLOW, "PopFrame() IsDone is true");
		//pPSPSound->SendEvent(MID_THPLAY_DONE);
		m_RingBuffer[m_PopIndex].bIsLastFrame = false; /** So we only send this once */
		//Empty();
		Log(LOG_VERYLOW, "PopFrame() Calling Stop()");
		pPSPSound->Stop();
		Log(LOG_VERYLOW, "PopFrame() Sending MID_THPLAY_EOS");
		pPSPSound->SendEvent(MID_THPLAY_EOS);
		return m_EmptyFrame; /** Don't pop more frames */
	}
	if (true == m_bBuffering)
	{
		//while ( (100 !=GetBufferFillPercentage()) && (0 == pPSPSound->GetEventToPlayThSize()))
		while ( (m_FrameCount < PSP_BUFFER_SIZE_IN_FRAMES*10) && (0 == pPSPSound->GetEventToPlayThSize()))
		{	/** Buffering!! */
			sceKernelDelayThread(50); /** 500us */
		}
		m_bBuffering = false;
	}
	else
	{
		while ( (m_FrameCount <= 0) && (0 == pPSPSound->GetEventToPlayThSize()) )
		{	/** Buffer Empty!! */
			sceKernelDelayThread(50); /** 500us */
			m_bBuffering = true;
		}
	}

	if (m_FrameCount > 0)
	{
		memcpy(&sFrame, &(m_RingBuffer[m_PopIndex].frame), sizeof(Frame));
		m_PopIndex++;
		if (m_PopIndex > m_RingBufferSize)
			m_PopIndex = 0;
	
		m_FrameCount--;

		return sFrame;
	}
	else
	{
		return m_EmptyFrame;
	}
}

void CPSPSoundBuffer::Done()
{
	m_RingBuffer[m_PushIndex].bIsLastFrame = true;
}

bool CPSPSoundBuffer::IsDone()
{
	return m_RingBuffer[m_PopIndex].bIsLastFrame;
}
