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
	ringbuf_start=NULL;
	pspbuf = NULL;
	/** **/
	//m_samplerate = PSP_SAMPLERATE;
	
	m_NumBuffers = DEFAULT_NUM_BUFFERS;
	
	memset(&m_EmptyFrame, 0, sizeof(Frame));
	
	AllocateBuffers();
	
}

CPSPSoundBuffer::~CPSPSoundBuffer()
{
	if (ringbuf_start)
	{
		delete[] ringbuf_start, ringbuf_start=NULL;
	}
	if (pspbuf)
	{
		free(pspbuf), pspbuf = NULL;
	}
	
}


void CPSPSoundBuffer::AllocateBuffers()
{
	if (ringbuf_start)
	{
		delete[] ringbuf_start, ringbuf_start = NULL;
	}
	if (pspbuf)
	{
		free (pspbuf), pspbuf = NULL;
	}
	
	m_FrameTransportSize = (PSP_BUFFER_SIZE_IN_FRAMES*m_NumBuffers);
	ringbuf_start = new FrameTransport[m_FrameTransportSize*15];
	
	ringbuf_end = &ringbuf_start[m_FrameTransportSize];
	pspbuf = (Frame *)memalign(64, FRAMES_TO_BYTES(PSP_BUFFER_SIZE_IN_FRAMES));
	
	Empty();
}

/*Takes the number of PSP sound buffers 20~100. If not changed, defaults to DEFAULT_NUM_BUFFERS.*/
void CPSPSoundBuffer::ChangeBufferSize(size_t buffer_size) 
{
	m_NumBuffers = buffer_size;
	AllocateBuffers();
}

void  CPSPSoundBuffer::Empty() 
{ 
	Log(LOG_VERYLOW, "CPSPSoundBuffer::Empty() Called.");
	for (size_t i = 0; i < m_FrameTransportSize ; i++)
	{
		memset(&(ringbuf_start[i].frame), 0, sizeof(Frame));
		ringbuf_start[i].bIsLastFrame = false;
	}
	pushpos = poppos = ringbuf_start;
	m_lastpushpos = 0;
	m_FrameCount = 0;
	m_mult = 1, m_div = 1;
	m_buffering = true;
}

size_t CPSPSoundBuffer::GetBufferFillPercentage() 
{ 
	
	return 100*m_FrameCount/(PSP_BUFFER_SIZE_IN_FRAMES*m_NumBuffers);
};
		
void CPSPSoundBuffer::SampleRateChange(int newRate)
{
	if (newRate > 0)
	{
		//Empty();
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

void CPSPSoundBuffer::PushFrame(Frame frame) /** Push a frame at an arbitrary samplerate */
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

void CPSPSoundBuffer::Push44Frame(Frame frame) /** Push a frame from a 44.1KHz stream */
{
	static int timeLastPercentEvent = 500; /** Initialize to something so the percentage event is sent the first time */
	
	
	while ( GetBufferFillPercentage() == 100 )
	{ 
		sceKernelDelayThread(50); /** 50us */
		if ( (pPSPApp->IsExiting() == true) )
			break;
	}
	
	pushpos++;
	if (pushpos > ringbuf_end)
		pushpos = ringbuf_start;
		
	m_FrameCount++;
	
	memcpy(&pushpos->frame, &frame, sizeof(Frame));
	
	if ((clock()*1000/CLOCKS_PER_SEC - timeLastPercentEvent) > 333) /** 3 times per sec */
	{
		pPSPSound->SendEvent(MID_BUFF_PERCENT_UPDATE);
		timeLastPercentEvent = clock()*1000/CLOCKS_PER_SEC;
	}
}


Frame *CPSPSoundBuffer::PopBuffer()
{
	for (int i = 0 ; (i < PSP_BUFFER_SIZE_IN_FRAMES); i++)
	{
		pspbuf[i] = PopFrame();
		//if (true == IsDone() || ( pPSPSound->GetEventToPlayThSize() > 0) ) /** Message Waiting */
		//{
			//pPSPSound->SendEvent(MID_THPLAY_DONE);
		//	break;
		//}
	}
	return pspbuf;
}

Frame CPSPSoundBuffer::PopFrame()
{
	if (true == IsDone())
	{
		pPSPSound->SendEvent(MID_THPLAY_DONE);
		poppos->bIsLastFrame = false; /** So we only send this once */
		Empty();
		pPSPSound->Stop();
		//CPSPEventQ::QEvent event = { 0, 0x0, NULL };
		//event.EventId = MID_PLAY_STOP;
		//m_EventToPlayTh->Send(event);
		return m_EmptyFrame; /** Don't pop more frames */
	}
	if (true == m_buffering)
	{
		while (GetBufferFillPercentage() != 100)
		{	/** Buffering!! */
			sceKernelDelayThread(50); /** 500us */
			if ( pPSPSound->GetEventToPlayThSize() > 0) /** Message Waiting */
				break;
		}
		m_buffering = false;
	}
	else
	{
		while (m_FrameCount <= 0)
		{	/** Buffer Empty!! */
			sceKernelDelayThread(50); /** 500us */
			if ( pPSPSound->GetEventToPlayThSize() > 0) /** Message Waiting */
				break;
		}
	}

	if (m_FrameCount > 0)
	{
		static Frame ret;
		memcpy(&ret, &poppos->frame, sizeof(Frame));
		poppos++;
		if (poppos > ringbuf_end)
			poppos = ringbuf_start;
	
		m_FrameCount--;

		return ret;
	}
	else
	{
		return m_EmptyFrame;
	}
}

void CPSPSoundBuffer::Done()
{
	pushpos->bIsLastFrame = true;
}

bool CPSPSoundBuffer::IsDone()
{
	return poppos->bIsLastFrame;
}
