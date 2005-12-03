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
#include "PSPSoundDecoder_MAD.h"
#include "PSPSoundDecoder_OGG.h"

using namespace std;

CPSPSound *pPSPSound = NULL;

/** Accessors */
int CPSPSound::GetAudioHandle()
{
	return m_audiohandle;
}

CPSPSound::CPSPSound()
{
	Log(LOG_VERYLOW, "PSPSound Constructor");
	m_thDecode = NULL;
	m_thPlayAudio  = NULL;
	pPSPSound = this;
	m_audiohandle = -1;
	m_CurrentState = STOP;
	m_EventToDecTh = m_EventToPlayTh = NULL;
	Initialize();
}

void CPSPSound::Initialize()
{
	Log(LOG_VERYLOW, "PSPSound Initialize()");
	
	Buffer.Empty();
	
	m_audiohandle = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, 
									PSP_BUFFER_SIZE_IN_FRAMES, 
									PSP_AUDIO_FORMAT_STEREO);

	if ( m_audiohandle < 0 )
	{
		Log(LOG_ERROR, "Error getting a sound channel!");
		ReportError("Unable to aquire sound channel");
	}
	
	m_CurrentStream = new CPSPStream();
	
	m_EventToDecTh  = new CPSPEventQ("eventq2dec_th");
	m_EventToPlayTh = new CPSPEventQ("eventq2play_th");
	
	m_thDecode = new CPSPThread("decode_thread", ThDecode, 64, 80000);
	m_thPlayAudio = new CPSPThread("playaudio_thread", ThPlayAudio, 16, 80000);
	
	if (m_CurrentStream && m_EventToDecTh && m_EventToPlayTh && m_thDecode && m_thPlayAudio)
	{
		m_thPlayAudio->Start();
		m_thDecode->Start();
		
		CPSPEventQ::QEvent event = { 0, 0x0, NULL };
		event.EventId = MID_PLAY_START;
		m_EventToPlayTh->Send(event);

	}
	else
	{
		Log(LOG_ERROR, "Initialize(): Memory allocation error!");
	}
	
}

CPSPSound::~CPSPSound()
{
	CPSPEventQ::QEvent event = { 0, 0x0, NULL };
	Log(LOG_VERYLOW, "~CPSPSound(): pPSPApp->m_Exit=%d", pPSPApp->m_Exit);
	
	if (m_thDecode) 
	{ 
		/** Wake the decoding thread up, so it can exit*/
		Log(LOG_VERYLOW, "~CPSPSound(): Tell decode thread to exit.");
		event.EventId = MID_DECODER_THREAD_EXIT_NEEDOK;
		m_EventToDecTh->SendAndWaitForOK(event);
		
		Log(LOG_VERYLOW, "~CPSPSound(): Destroying decode thread. ");
		delete(m_thDecode), m_thDecode = NULL;
	}
	
	if (m_thPlayAudio) 
	{
		Log(LOG_VERYLOW, "~CPSPSound(): Tell play thread to exit. ");
		event.EventId = MID_PLAY_THREAD_EXIT_NEEDOK;
		m_EventToPlayTh->SendAndWaitForOK(event);
		
		Log(LOG_VERYLOW, "~CPSPSound(): Destroying play thread. ");
		delete(m_thPlayAudio), m_thPlayAudio = NULL;
	}
	
	if (m_EventToDecTh)
	{
		delete(m_EventToDecTh); m_EventToDecTh = NULL;
	}
	
	if (m_EventToPlayTh)
	{
		delete(m_EventToPlayTh); m_EventToPlayTh = NULL;
	}
	
	
	Log(LOG_VERYLOW, "~CPSPSound(): The End.");

}

void CPSPSound::SetDecodeThreadPriority(int iNewPrio)
{
	m_thDecode->SetPriority(iNewPrio);
}

void CPSPSound::SetPlayThreadPriority(int iNewPrio)
{
	m_thPlayAudio->SetPriority(iNewPrio);
}

int CPSPSound::Play()
{
	CPSPEventQ::QEvent event = { 0, 0x0, NULL };
	Log(LOG_LOWLEVEL, "Play('%s'): m_CurrentState=%s", 
		m_CurrentStream->GetURI(),
		m_CurrentState==PLAY?"PLAY":(m_CurrentState==STOP?"STOP":"PAUSE"));
	switch(m_CurrentState)
	{
		case STOP:
			event.EventId = MID_DECODER_START;
			m_EventToDecTh->Send(event);
			break;
		case PAUSE:
			//event.EventId = MID_DECODER_START;
			//m_EventToDecTh->Send(event);
			event.EventId = MID_PLAY_START;
			m_EventToPlayTh->Send(event);
			break;
			
		case PLAY:
			/** Shouldn't get here, let's restart */
			Log(LOG_ERROR, "Play and state was already playing, restarting decoding/playing");
			//event.EventId = MID_DECODER_STOP;
			//m_EventToDecTh->Send(event);
			//event.EventId = MID_PLAY_STOP;
			//m_EventToPlayTh->Send(event);
		
			event.EventId = MID_DECODER_START;
			m_EventToDecTh->Send(event);
			break;
		default:
			break;
	}
	m_CurrentState = PLAY;
	return m_CurrentState;
}

int CPSPSound::Pause()
{
	CPSPEventQ::QEvent event = { 0, 0x0, NULL };
	Log(LOG_LOWLEVEL, "Pause(): Called. m_CurrentState=%s", 
		m_CurrentState==PLAY?"PLAY":(m_CurrentState==STOP?"STOP":"PAUSE"));
	switch(m_CurrentState)
	{
		case PLAY:
			m_CurrentState = PAUSE;
			//event.EventId = MID_DECODER_STOP;
			//m_EventToDecTh->Send(event);
			event.EventId = MID_PLAY_STOP;
			m_EventToPlayTh->Send(event);
			break;
			
		case STOP:
		case PAUSE:
		default:
			break;
	}
	return m_CurrentState;
}

int CPSPSound::Stop()
{
	CPSPEventQ::QEvent event = { 0, 0x0, NULL };
	Log(LOG_LOWLEVEL, "Stop(): Called. m_CurrentState=%s", 
		m_CurrentState==PLAY?"PLAY":(m_CurrentState==STOP?"STOP":"PAUSE"));
	switch(m_CurrentState)
	{
		case PAUSE:
			/** if we were paused, restart threads first! */
			m_CurrentState = STOP;
			event.EventId = MID_DECODER_STOP;
			m_EventToDecTh->SendAndWaitForOK(event);
			event.EventId = MID_PLAY_START;
			m_EventToPlayTh->Send(event);
			//event.EventId = MID_PLAY_STOP;
			//m_EventToPlayTh->SendAndWaitForOK(event);
			break;
		case PLAY:
			m_CurrentState = STOP;
			event.EventId = MID_DECODER_STOP;
			Log(LOG_VERYLOW, "Stop(): Was Playing. Calling 'm_EventToDecTh->SendAndWaitForOK(STOP)'");
			m_EventToDecTh->SendAndWaitForOK(event);
			//event.EventId = MID_PLAY_STOP;
			//Log(LOG_VERYLOW, "Stop(): Was Playing. Calling 'm_EventToPlayTh->SendAndWaitForOK(STOP)'");
			//m_EventToPlayTh->SendAndWaitForOK(event);
			break;
			
		case STOP:
		default:
			break;
	}
	return m_CurrentState;
}

/** Threads */
int CPSPSound::ThPlayAudio(SceSize args, void *argp)
{
		Frame *mybuf = NULL;
		int ah = pPSPSound->GetAudioHandle();
		CPSPEventQ *m_EventToPlayTh = pPSPSound->m_EventToPlayTh;
		CPSPEventQ::QEvent event = { 0, 0, NULL };
		int rret = 0;
		int timeLastPercentEvent = 500; /** Initialize to something so the percentage event is sent the first time */
		
		Log(LOG_INFO, "Starting Play Thread.");
		pPSPSound->SendEvent(MID_THPLAY_BEGIN);
		
		for(;;)
		{
			Log(LOG_VERYLOW, "ThPlay::Calling Receive. %d Messages in Queue", m_EventToPlayTh->Size());
			rret = m_EventToPlayTh->Receive(event);
			Log(LOG_VERYLOW, "ThPlay::Receive Ret=%d. eventid=0x%08x.", rret, event.EventId);
			switch (event.EventId)
			{
			case MID_PLAY_THREAD_EXIT_NEEDOK:
				Log(LOG_VERYLOW, "ThPlay:: Thread Exit message received.");
				pPSPSound->SendEvent(MID_THPLAY_END);
				m_EventToPlayTh->SendReceiveOK();
				sceKernelExitThread(0);
				break;
			
			case MID_PLAY_START:
			{
				Log(LOG_VERYLOW, "ThPlay:: Thread Play message received.");
				pPSPSound->SendEvent(MID_THPLAY_PLAYING);
				while (0 == m_EventToPlayTh->Size())
				{
					mybuf = pPSPSound->Buffer.PopDeviceBuffer();
					sceAudioOutputPannedBlocking(ah, PSP_AUDIO_VOLUME_MAX, PSP_AUDIO_VOLUME_MAX, mybuf);
					
					if ((clock()*1000/CLOCKS_PER_SEC - timeLastPercentEvent) > 333) /** 3 times per sec */
					{
						pPSPSound->SendEvent(MID_BUFF_PERCENT_UPDATE);
						timeLastPercentEvent = clock() * 1000 / CLOCKS_PER_SEC;
					}
				}
				Log(LOG_VERYLOW, "ThPlay:: Thread Play exiting play area - message waiting.");
				break;
			}	
			case MID_PLAY_STOP:
				if (STOP == pPSPSound->GetPlayState())
				{
					Log(LOG_VERYLOW, "ThPlay:: Stop message received.");
					pPSPSound->Buffer.Empty();
					pPSPSound->SendEvent(MID_THPLAY_DONE);
				}
				else
				{
					Log(LOG_ERROR, "ThPlay:: Stop message received. (But stream not stopped)");
				}
				break;
			}
		}
		return 0;
}

int CPSPSound::ThDecode(SceSize args, void *argp) 
{
	IPSPSoundDecoder *Decoder = NULL;
	
	Log(LOG_INFO,"Starting Decoding Thread.");
	pPSPSound->SendEvent(MID_THDECODE_BEGIN);

	CPSPEventQ *m_EventToDecTh = pPSPSound->m_EventToDecTh;
	CPSPEventQ::QEvent event = { 0, 0, NULL };
	int rret = 0;
	for(;;)
	{
		Log(LOG_VERYLOW, "ThDecode::Calling Receive");
		rret = m_EventToDecTh->Receive(event);
		Log(LOG_VERYLOW, "ThDecode::Receive Ret=%d. event=0x%08x.", rret, event.EventId);
		switch (event.EventId)
		{
			case MID_DECODER_START:
				Log(LOG_VERYLOW, "ThDecode:: Start Decoder message received.");
				
				if (Decoder)
				{
					delete Decoder, Decoder = NULL;
				}
				
				pPSPSound->SendEvent(MID_DECODE_STREAM_OPENING);
				Log(LOG_INFO, "ThDecode:: Calling Open For '%s'", 
					pPSPSound->m_CurrentStream->GetURI());
				pPSPSound->m_CurrentStream->Open();
				
				if (true == pPSPSound->m_CurrentStream->IsOpen())
				{
					bool bDecoderCreated = false;
					switch (pPSPSound->m_CurrentStream->GetContentType())
					{
						case MetaData::CONTENT_NOT_DEFINED:
							Log(LOG_INFO, "ThDecode:: Content type not defined. Defaulting to MPEG.");
							//pPSPSound->SendEvent(MID_DECODE_STREAM_OPEN_ERROR);
							//bDecoderCreated = false;
							//break;
							//fall through (some servers don't define stream type - asume MPEG)
						case MetaData::CONTENT_AUDIO_MPEG:
							Log(LOG_INFO, "ThDecode:: MPEG Stream Opened Successfully.");
							pPSPSound->SendEvent(MID_DECODE_STREAM_OPEN);
							Decoder = new CPSPSoundDecoder_MAD(&pPSPSound->Buffer, pPSPSound->m_CurrentStream);
							bDecoderCreated = true;
							break;
						case MetaData::CONTENT_AUDIO_OGG:
							Log(LOG_INFO, "ThDecode:: OGG Stream Opened Successfully.");
							pPSPSound->SendEvent(MID_DECODE_STREAM_OPEN);
							Decoder = new CPSPSoundDecoder_OGG(&pPSPSound->Buffer, pPSPSound->m_CurrentStream);
							bDecoderCreated = true;
							break;
						case MetaData::CONTENT_AUDIO_AAC:
							Log(LOG_INFO, "ThDecode:: AAC Stream Not supported.");
							pPSPSound->SendEvent(MID_DECODE_STREAM_OPEN_ERROR);
							bDecoderCreated = false;
							break;
						
						case MetaData::CONTENT_PLAYLIST:
						{
							Log(LOG_INFO, "ThDecode: This is a playlist.. downloading..");
							CPSPStreamReader *PLReader = new CPSPStreamReader(pPSPSound->m_CurrentStream);
							char strPlayListBuf[256];
							strPlayListBuf[0]=0;
							PLReader->Read((u8*)strPlayListBuf, 256);
							strPlayListBuf[255] =0;
							Log(LOG_INFO, "ThDecode: playlist contents: '%s'", strPlayListBuf);
							char *strURI = "";
							if (strstr(strPlayListBuf, "File1="))
							{
								strURI = strstr(strPlayListBuf, "File1=")+strlen("File1=");
								*strchr(strURI, 0xA) = 0;
								pPSPSound->m_CurrentStream->SetURI(strURI);
								/** We send an event to ourselves */
								event.EventId = MID_DECODER_START;
								m_EventToDecTh->Send(event);
							}
							delete (PLReader);
							bDecoderCreated = false;
							break;
						}
						default:
							Log(LOG_ERROR, "ThDecode:: Content type not recognized.");
							bDecoderCreated = false;
							break;
						
					}

					if (true == bDecoderCreated)
					{
						/** Start play thread */
						//CPSPEventQ::QEvent event = { 0, 0, NULL };
						//event.EventId = MID_PLAY_START;
						//pPSPSound->m_EventToPlayTh->Send(event);
						
						pPSPSound->SendEvent(MID_THDECODE_DECODING);
						/** Main decoding loop */
						/* pPSPSound is the decoding loop. */
						while (m_EventToDecTh->Size() == 0)
						{
							if (true == Decoder->Decode())
							{
								//pPSPSound->Buffer.Done();
								break;
							}
							sceKernelDelayThread(10); /** 100us */
						}
						
						pPSPSound->SendEvent(MID_THDECODE_DECODING_DONE);
						
						delete Decoder, Decoder = NULL;
					}
					else
					{
						/** Close if no decoder instantiated */
						pPSPSound->m_CurrentStream->Close();
					}
				}
				else
				{
					Log(LOG_ERROR, "ThDecode:: Unable to open stream '%s'.", pPSPSound->m_CurrentStream->GetURI());
					pPSPSound->SendEvent(MID_DECODE_STREAM_OPEN_ERROR);
				}
				
				break;
			case MID_DECODER_STOP:
				Log(LOG_VERYLOW, "ThDecode:: Stop Decoder message received.");
				m_EventToDecTh->SendReceiveOK();
				break;
			case MID_DECODER_THREAD_EXIT_NEEDOK:
				Log(LOG_VERYLOW, "ThDecode:: Thread Exit message received.");
				pPSPSound->SendEvent(MID_THDECODE_END);
				m_EventToDecTh->SendReceiveOK();
				sceKernelExitThread(0);
				break;
		}
	}

	return 0;
}
