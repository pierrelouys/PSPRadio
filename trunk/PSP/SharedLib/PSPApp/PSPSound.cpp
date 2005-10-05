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
#include <sys/socket.h>
#include "PSPSound.h"

using namespace std;

#define ReportError pPSPApp->ReportError

CPSPSound *pPSPSound = NULL;

/** Accessors */
int CPSPSound::GetAudioHandle()
{
	return m_audiohandle;
}

CPSPSound::CPSPSound()
{
	Log(LOG_VERYLOW, "PSPSound Constructor");
	Initialize();
}

void CPSPSound::Initialize()
{
	Log(LOG_VERYLOW, "PSPSound Initialize()");
	if (pPSPSound != NULL)
	{
		Log(LOG_ERROR, "Error!, only one instance of CPSPSound (including CPSPSound_*) permitted!");
	}
	pPSPSound = this;
	
	Buffer.Empty();
	
	m_InputStream = NULL;
	m_InputStream = new CPSPSoundStream();
	
	if (!m_InputStream)
	{
		Log(LOG_ERROR, "Memory allocation error instantiating m_InputStream");
		ReportError("Sound Initialization Error");
	}

	m_audiohandle = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, 
									PSP_BUFFER_SIZE_IN_FRAMES, 
									PSP_AUDIO_FORMAT_STEREO);

	if ( m_audiohandle < 0 )
	{
		Log(LOG_ERROR, "Error getting a sound channel!");
		ReportError("Unable to aquire sound channel");
	}
	
	m_CurrentState = STOP;
	m_thDecode = NULL;
	m_thPlayAudio  = NULL;
	
}

CPSPSound::~CPSPSound()
{
	Log(LOG_VERYLOW, "~CSPSSound(): pPSPApp->m_Exit=%d", pPSPApp->m_Exit);
	Stop();
	
	if (m_thDecode) 
	{ 
		/** Wake the decoding thread up, so it can exit*/
		Log(LOG_VERYLOW, "~CSPSSound(): Decode thread was asleep, waking up.");
		m_thDecode->WakeUp();
		sceKernelDelayThread(100000);
		
		Log(LOG_VERYLOW, "~CSPSSound(): Destroying decode thread. ");
		delete(m_thDecode), m_thDecode = NULL;
	}
	
	if (m_thPlayAudio) 
	{
		Log(LOG_VERYLOW, "~CSPSSound(): Destroying play thread. ");
		delete(m_thPlayAudio), m_thPlayAudio = NULL;
	}
	
	if (m_InputStream)
	{
		Log(LOG_VERYLOW, "~CSPSSound(): Destroying input stream object. ");
		delete(m_InputStream); m_InputStream = NULL;
	}

	Log(LOG_VERYLOW, "~CSPSSound(): The End.");

}

int CPSPSound::Play()
{
	switch(m_CurrentState)
	{
		case STOP:
			m_CurrentState = PLAY;
			if (!m_thDecode)
			{
				Log(LOG_LOWLEVEL, "Play(): Creating decode and play threads.");
				m_thDecode = new CPSPThread("decode_thread", ThDecode, 64, 80000);
				if (!m_thPlayAudio)
				{
					m_thPlayAudio = new CPSPThread("playaudio_thread", ThPlayAudio, 16, 80000);
					m_thPlayAudio->Start();
					m_thDecode->Start();
					sceKernelDelayThread(100000);
				}
			}
			/** Wake the decoding thread up*/
			m_thDecode->WakeUp();
			break;
		case PAUSE:
			m_CurrentState = PLAY;
			m_thDecode->Resume();
			m_thPlayAudio->Resume();
			break;
			
		case PLAY:
		default:
			break;
	}
	return m_CurrentState;
}

int CPSPSound::Pause()
{
	switch(m_CurrentState)
	{
		case PLAY:
			m_CurrentState = PAUSE;
			m_thDecode->Suspend();
			m_thPlayAudio->Suspend();
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
	switch(m_CurrentState)
	{
		case PAUSE:
			/** if we were paused, restart threads first! */
			m_CurrentState = STOP;
			Buffer.Empty();
			m_thDecode->Resume();
			m_thPlayAudio->Resume();
			break;
		case PLAY:
			m_CurrentState = STOP;
			Buffer.Empty();
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
		int count = 0;

		Log(LOG_INFO, "Starting Play Thread.");
		pPSPSound->SendMessage(MID_THPLAY_BEGIN);
		
		pPSPApp->CantExit(); /** This to prevent the app to exit while in this area */
		for(;;)
		{
			if (pPSPApp->m_Exit == true)
			{
				break;
			}
			mybuf = pPSPSound->Buffer.PopBuffer();
			if (mybuf)
			{
				sceAudioOutputPannedBlocking(ah, PSP_AUDIO_VOLUME_MAX, PSP_AUDIO_VOLUME_MAX, mybuf);
			}
			else
			{
				/** Buffer underrun! */
				ReportError("Buffer Underrun %03d   ", pPSPSound->Buffer.GetPushPos());
				sceKernelDelayThread(10); /** 10us */
			}
			
			if (pPSPSound->Buffer.IsDone())
			{
				//pPSPSound->Buffer.Empty(); /** This also clears the IsDone flag, so that the next song can keep playing */
				pPSPSound->SendMessage(MID_THPLAY_DONE);
			}
			
			if (count++ % 5 == 0)
			{
				pPSPSound->SendMessage(MID_THPLAY_BUFCYCLE);
			}
			
		}
		
		pPSPSound->SendMessage(MID_THPLAY_END);
		pPSPApp->CanExit(); /** OK, App can exit now. */

		sceKernelExitThread(0);
		return 0;
}

int CPSPSound::ThDecode(SceSize args, void *argp)
{
	//pspDebugScreenSetXY(0,4);
	Log(LOG_INFO,"Starting Decoding Thread; putting thread to sleep.");
	pPSPSound->SendMessage(MID_THDECODE_BEGIN);

	pPSPApp->CantExit(); /** This to prevent the app to exit while in this area */
	
	while (pPSPApp->m_Exit == FALSE)
	{
		/** Wait for the go-ahead: 
		 *  We put ourselves to sleep
		 *  When Play() is called, we are awaken
		 */
		pPSPSound->SendMessage(MID_THDECODE_ASLEEP);
		Sleep();
		if (pPSPApp->m_Exit == true)
		{
			Log(LOG_LOWLEVEL,"Awakening Decoding Thread; Application Exiting.");
			break;
		}
		else
		{
			Log(LOG_LOWLEVEL,"Awakening Decoding Thread; calling Decode().");
			pPSPSound->SendMessage(MID_THDECODE_AWOKEN);
			pPSPSound->Decode();
		}
	}
	pPSPSound->SendMessage(MID_THDECODE_END);
	
	pPSPApp->CanExit(); /** OK, App can exit now. */
	
	sceKernelExitThread(0);

	return 0;
}

void CPSPSound::Decode()
{
	ReportError("Decode() Not Implemented!");
	Log(LOG_ERROR, "Decode() Called, but not implemented.");
}

/** Sound buffer class implementation */
CPSPSoundBuffer::CPSPSoundBuffer()
{
	/** Initialize */
	m_bUpsamplingTemp = NULL;
	m_bUpsamplingOut = NULL;
	ringbuf_start=NULL;
	pspbuf = NULL;
	/** **/
	m_samplerate = PSP_SAMPLERATE;
	
	ringbuf_start = (Frame *)memalign(64, (FRAMES_TO_BYTES(PSP_BUFFER_SIZE_IN_FRAMES) * (NUM_BUFFERS + 5))*15/*padding for upsampling*/);
	ringbuf_end = ringbuf_start+(PSP_BUFFER_SIZE_IN_FRAMES*NUM_BUFFERS);
	pspbuf = (Frame *)memalign(64, FRAMES_TO_BYTES(PSP_BUFFER_SIZE_IN_FRAMES));
	//m_bUpsamplingTemp = (Frame*)memalign(64, (FRAMES_TO_BYTES(PSP_BUFFER_SIZE_IN_FRAMES)*11/** max multiplier = 11*/));
	//m_bUpsamplingOut = (Frame*)memalign(64, (FRAMES_TO_BYTES(PSP_BUFFER_SIZE_IN_FRAMES)*11/** max multiplier = 11*/));
	
	Empty();
	
}

CPSPSoundBuffer::~CPSPSoundBuffer()
{
	if (m_bUpsamplingTemp)
	{
		free(m_bUpsamplingTemp);
	}
	if (m_bUpsamplingOut)
	{
		free(m_bUpsamplingOut);
	}
	if (ringbuf_start)
	{
		free(ringbuf_start), ringbuf_start=NULL;
	}
	if (pspbuf)
	{
		free(pspbuf), pspbuf = NULL;
	}
	
}

void  CPSPSoundBuffer::Empty() 
{ 
	memset(ringbuf_start, 0, FRAMES_TO_BYTES(PSP_BUFFER_SIZE_IN_FRAMES) * NUM_BUFFERS);
	pushpos = poppos = ringbuf_start;
	m_lastpushpos = 0;
	buffering = true;
}

size_t CPSPSoundBuffer::GetBufferFillPercentage() 
{ 
	size_t diff = poppos - pushpos;
	
	if (diff < 0)
	{
		diff = (ringbuf_end - pushpos) + (poppos - ringbuf_start);
		if (diff > (PSP_BUFFER_SIZE_IN_FRAMES*NUM_BUFFERS))
		{
			diff = (ringbuf_end - poppos) + (pushpos - ringbuf_start);
		}
	}
	return 100*diff/(PSP_BUFFER_SIZE_IN_FRAMES*NUM_BUFFERS);
};
		
void CPSPSoundBuffer::SetSampleRate(size_t samplerate)
{
	if (samplerate > 0)
	{
		if (m_bUpsamplingTemp && m_bUpsamplingOut)
		{
			Empty();
			m_samplerate = samplerate;
			Log(LOG_VERYLOW, "Allocated m_bUpsamplingTemp, samplerate=%dHz.", samplerate);

		}
		else
		{
			Log(LOG_ERROR, "Memory allocation error creating an m_bUpsamplingTemp (samplerate=%dHz).", 
				samplerate);
			m_samplerate = 0;
		}
	}
}

void CPSPSoundBuffer::PushFrame(Frame frame)
{
	//size_t size = 0;
	static int count = 0;
	
	while((pushpos - poppos) > (NUM_BUFFERS*PSP_BUFFER_SIZE_IN_FRAMES)/2)
	{ /*Wait for pop to catch up, we keep a min space of NUM_BUFFERS/2 */
		sceKernelDelayThread(50); /** 50us */
		if (pPSPApp->IsExiting() == true)
			break;
	}
	
//	if (PSP_SAMPLERATE == m_samplerate)
	{
		//memcpy(ringbuf_start+(pushpos*FRAMES_TO_BYTES(PSP_BUFFER_SIZE_IN_FRAMES)), 
		//		buf, 
		//		FRAMES_TO_BYTES(PSP_BUFFER_SIZE_IN_FRAMES));
		*pushpos = frame;
		pushpos++;// = (pushpos + 1);// % (NUM_BUFFERS*PSP_BUFFER_SIZE_IN_FRAMES);
		if (pushpos > ringbuf_end)
			pushpos = ringbuf_start;
		
		if (count++ % (NUM_BUFFERS*PSP_BUFFER_SIZE_IN_FRAMES)*5 == 0)
		{
			pPSPSound->SendMessage(MID_DECODE_BUFCYCLE);
		}
	}
#if 0
	else /** We need to upsample */
	{
		if (m_bUpsamplingTemp && m_bUpsamplingOut)
		{
			int mult = 1, div = 1;
			switch(m_samplerate)
			{
			case 8000:
				mult=11;
				div = 2;
				break;
			case 11025:
				mult=4;
				div =1;
				break;
			case 16000:
				mult=11;
				div = 4;
				break;
			case 22050:
				mult=2;
				div =1;
				break;
			case 24000:
				mult=11;
				div =6;
				break;
			case 32000:
				mult=11;
				div = 8;
				break;
			case 44100:
				mult=1;
				div =1;
				break;
			case 47250:
				break;
			case 48000:
				mult=11;
				div=12;
				break;
			}
				
			size = UpSample(/*out*/(short*)m_bUpsamplingOut, /*in*/(short*)buf, mult, div);
			memcpy(ringbuf_start+(pushpos*size), m_bUpsamplingOut, size);
			//Log(LOG_LOWLEVEL, "size=%d out=%d s/o=%d", size, OUTPUT_BUFFER_SIZE, (int)size/(int)OUTPUT_BUFFER_SIZE);
			pushpos = (pushpos + mult/div) % NUM_BUFFERS;
		}
		else
		{
			Log(LOG_ERROR, "CPSPSoundBuffer::Push() Error. No upsampled_buffer.");
		}
	}
#endif
}

#if 0 
size_t CPSPSoundBuffer::UpSample(short *bOut, short *bIn, int mult, int div)
{
	int iCnt;
	size_t out_size_in_bytes;
	short *bUpsamplingTemp;
	size_t iInSamples;



	//Log(LOG_VERYLOW, "Upsample(): samplerate=%d mult=%d div=%d", m_samplerate, mult, div);
	
	//if (mult > 1)
	{
		bUpsamplingTemp = m_bUpsamplingTemp;
		for (iInSamples = 0 ; iInSamples < FRAMES_TO_SAMPLES(PSP_BUFFER_SIZE_IN_FRAMES) ; iInSamples++)
		{
			for (iCnt = 0 ; iCnt < mult ; iCnt++)
			{
				*(bUpsamplingTemp)=*(bIn);	/** Left sample  */
				*(bUpsamplingTemp+1)=*(bIn+1); /** Right sample */
			}
			bUpsamplingTemp+=(mult*2);
			bIn+=2;
		}
		out_size_in_bytes = (char*)bUpsamplingTemp - (char*)m_bUpsamplingTemp;
	}
	
	//if (div > 1)
	{
		int iAverageL = 0, iAverageR = 0;
		size_t iFrames = 0;
		size_t frames = PSP_BUFFER_SIZE_IN_FRAMES*mult;
		bUpsamplingTemp = m_bUpsamplingTemp;
		for (iFrames = 0; iFrames < frames; iFrames++)
		{
			iAverageL += *(bUpsamplingTemp);
			iAverageR += *(bUpsamplingTemp+1);
			if (iFrames % div == 0)
			{
				*(bOut)   = iAverageL/div;
				*(bOut+1) = iAverageR/div;
				iAverageL = iAverageR = 0;
			}
			bOut+=2;
			bUpsamplingTemp+=2;
		}
		out_size_in_bytes = (char*)bUpsamplingTemp - (char*)m_bUpsamplingTemp;
	}
	
	return out_size_in_bytes;
}
#endif

Frame *CPSPSoundBuffer::PopBuffer()
{
//FRAMES_TO_BYTES(PSP_BUFFERS_SIZE_IN_FRAMES)
	for (int i = 0 ; i < PSP_BUFFER_SIZE_IN_FRAMES; i++)
	{
		pspbuf[i] = PopFrame();
	}
	return pspbuf;
}

Frame CPSPSoundBuffer::PopFrame()
{
	if (buffering)
	{
		while (abs(pushpos - poppos) < (PSP_BUFFER_SIZE_IN_FRAMES*NUM_BUFFERS)/4) 
		{/** Buffering!! */
			sceKernelDelayThread(50); /** 500us */
			if (pPSPApp->IsExiting() == true)
				break;
		}
		buffering = false;
	}
	else
	{
		while (abs((int)(pushpos - poppos)) < PSP_BUFFER_SIZE_IN_FRAMES*1) 
		{/** Buffer almost empty!! */
			sceKernelDelayThread(50); /** 500us */
			if (pPSPApp->IsExiting() == true)
				break;
		}
	}
	Frame ret = *poppos;
	poppos++;// = (poppos + 1);// % (NUM_BUFFERS*PSP_BUFFER_SIZE_IN_FRAMES);
	if (poppos > ringbuf_end)
		poppos = ringbuf_start;

	return ret;
}

void CPSPSoundBuffer::Done()
{
	m_lastpushpos = pushpos;
}

bool CPSPSoundBuffer::IsDone()
{
	return (m_lastpushpos == poppos)?true:false;
}

/** class CPSPSoundStream */
CPSPSoundStream::CPSPSoundStream()
{
	m_Type = STREAM_TYPE_NONE;
	m_State = STREAM_STATE_CLOSED;
	m_pfd = NULL;
	m_BstdFile = NULL;
	m_fd = -1;
	m_sock_eof = true;
	m_iMetaDataInterval = 0;
	m_iRunningCountModMetadataInterval = 0;
	memset(bMetaData, 0, MAX_METADATA_SIZE);
 	memset(bPrevMetaData, 0, MAX_METADATA_SIZE);
 	m_strFile[0] = 0;
	
}

CPSPSoundStream::~CPSPSoundStream()
{
	Close();
}


/** Accessors */
void CPSPSoundStream::SetFile(char *strFile)
{
	if (strFile)
	{
		if (strlen(strFile) > 4)
		{
			strncpy(m_strFile, strFile, 256);
			if (memcmp(m_strFile, "http://", strlen("http://")) == 0)
			{
				Log(LOG_LOWLEVEL, "CPSPSoundStream::SetFile(%s) <URL> called", strFile);
				m_Type = STREAM_TYPE_URL;
			}
			else // It's a file!
			{
				Log(LOG_LOWLEVEL, "CPSPSoundStream::SetFile(%s) <FILE> called", strFile);
				m_Type = STREAM_TYPE_FILE;
			}
		}
		else
		{
			Log(LOG_ERROR, "CPSPSoundStream::SetFile(%s) BAD.", strFile);
			ReportError("CPSPSoundStream::OpenFile-Invalid filename '%s'", strFile);
		}
		
	}
}

void CPSPSoundStream::Close()
{
	if (STREAM_STATE_OPEN == m_State)
	{
		switch(m_Type)
		{
			case STREAM_TYPE_FILE:
				if (m_BstdFile)
				{
					BstdFileDestroy(m_BstdFile);
				}
				if (m_pfd)
				{
					fclose(m_pfd);
				}
				m_pfd = NULL;
				m_BstdFile = NULL;
				m_State = STREAM_STATE_CLOSED;
				break;
			case STREAM_TYPE_URL:
				if (m_fd >= 0)
				{
					sceNetInetClose(m_fd);
				}
				m_fd = -1;
				m_State = STREAM_STATE_CLOSED;
				break;
			case STREAM_TYPE_NONE:
				Log(LOG_ERROR, "SoundStream::Close(): Invalid State.");
				break;
		}
	}
	m_iMetaDataInterval = 0;
	m_iRunningCountModMetadataInterval = 0;
	memset(bMetaData, 0, MAX_METADATA_SIZE);
	memset(bPrevMetaData, 0, MAX_METADATA_SIZE);
}

int CPSPSoundStream::Open()
{
	if (STREAM_STATE_CLOSED == m_State)
	{
		switch(m_Type)
		{
			case STREAM_TYPE_URL:
				//ReportError ("Opening URL '%s'\n", filename);
				m_fd = http_open(m_strFile, m_iMetaDataInterval);
				if (m_fd < 0)
				{
					//Don't report again, because http_open will report.
					//ReportError("CPSPSoundStream::OpenFile-Error opening URL.\n");
					m_State = STREAM_STATE_CLOSED;
				}
				else
				{
					//ReportError("CPSPSoundStream::OpenFile-URL Opened. (handle=%d)\n", m_fd);
					//Log("Opened. MetaData Interval = %d\n", m_iMetaDataInterval);
					m_State = STREAM_STATE_OPEN;
					m_sock_eof = FALSE;
				}
				break;
			
			case STREAM_TYPE_FILE:
				m_pfd = fopen(m_strFile, "rb");
				if(m_pfd)
				{
					m_BstdFile=NewBstdFile(m_pfd);
					if(m_BstdFile != NULL)
					{
						m_State = STREAM_STATE_OPEN;
					}
					else
					{
						ReportError("CPSPSoundStream::OpenFile-Can't create a new bstdfile_t (%s).",
								strerror(errno));
						m_State = STREAM_STATE_CLOSED;
					}
				}
				else
				{
					ReportError("Unable to open file");
				}
				break;
			case STREAM_TYPE_NONE:
				ReportError("Calling OpenFile, but the set filename is invalid '%s'", m_strFile);
				break;
		}
	}
	else
	{
		ReportError("Calling OpenFile, but there is a file open already");
	}
	
	return m_State!=STREAM_STATE_CLOSED?0:-1;
}

size_t CPSPSoundStream::Read(unsigned char *pBuffer, size_t ElementSize, size_t ElementCount)
{
	size_t size = 0;
	size_t iBytesToRead = (ElementCount*ElementSize);
	char bMetaDataSize = 0;
	int iReadRet = -1;
	
	if (STREAM_STATE_OPEN == m_State)
	{
		switch(m_Type)
		{
			case STREAM_TYPE_FILE:
				size = BstdRead(pBuffer, ElementSize, ElementCount, m_BstdFile);
				break;
			case STREAM_TYPE_URL:
				if (m_iMetaDataInterval)
				{
					m_iRunningCountModMetadataInterval = (m_iRunningCountModMetadataInterval % m_iMetaDataInterval);
				}
				if (iBytesToRead + m_iRunningCountModMetadataInterval > m_iMetaDataInterval)
				{
					size = SocketRead((char*)pBuffer, m_iMetaDataInterval - m_iRunningCountModMetadataInterval, m_fd);
					if (size != (m_iMetaDataInterval - m_iRunningCountModMetadataInterval))
					{
						Close();
						m_sock_eof = true;
					}
					iReadRet = SocketRead(&bMetaDataSize, 1, m_fd);
					if (iReadRet > 0)
					{
						iReadRet = SocketRead(bMetaData, bMetaDataSize * 16, m_fd);
					}
					if (iReadRet != bMetaDataSize * 16)
					{
						Close();
						m_sock_eof = true;
					}
					else
					{
						/** If new data is received */
						if (memcmp(bPrevMetaData, bMetaData, MAX_METADATA_SIZE) != 0)
						{
							Log(LOG_INFO, "MetaData='%s'", bMetaData);
							pPSPSound->SendMessage(MID_DECODE_METADATA_INFO, bMetaData);
							memcpy(bPrevMetaData, bMetaData, MAX_METADATA_SIZE);
						}
					}
				}
				else
				{
					size = SocketRead((char*)pBuffer, iBytesToRead, m_fd);
					if (size != iBytesToRead)
					{
						Close();
						m_sock_eof = true;
					}
				}
				if (size > 0)
				{
					m_iRunningCountModMetadataInterval+=size;
				}
				break;
			case STREAM_TYPE_NONE:
				Log(LOG_ERROR, "Read() Called, but no stream set up.");
				break;
		}
	}
	else
	{
		Log(LOG_ERROR, "Read() Called but the stream is not open!");
	}
	
	return size;
}

bool CPSPSoundStream::IsEOF()
{
	int iseof = 0;
	
	if (STREAM_STATE_OPEN == m_State)
	{
		switch(m_Type)
		{
			case STREAM_TYPE_FILE:
				iseof = BstdFileEofP(m_BstdFile);
				break;
			case STREAM_TYPE_URL:
				iseof = m_sock_eof;
				break;
			case STREAM_TYPE_NONE:
				Log(LOG_ERROR, "IsEOF() Called but stream not setup");
				iseof = 1; /** Make them stop! */
				break;
		}
	}
	else
	{
		Log(LOG_ERROR, "IsEOF() Called but stream not open");
		iseof = 1; /** Make them stop! */
	}
	
	return iseof?true:false;
}

bool CPSPSoundStream::IsOpen()
{
	return (m_State==STREAM_STATE_CLOSED)?false:true;
}

int SocketRead(char *pBuffer, size_t LengthInBytes, int sock)
{
	size_t bytesread = 0, bytestoread = 0;
	size_t size = 0;
	for(;;) 
	{
		bytestoread = LengthInBytes-size;
		bytesread = recv(sock, pBuffer+size, bytestoread, 0);
		if (bytesread > 0)
			size += bytesread;
		if(bytesread == bytestoread) 
		{
			//done
			break;
		}
		else if (bytesread == 0)
		{
			ReportError("SocketRead(): Connection reset by peer!\n");
			//Close();
			//m_sock_eof = true;
			break;
		}
		if (pPSPSound->GetPlayState() == CPSPSound::STOP || pPSPApp->IsExiting() == true)
			break;
		//else if(error = sceNetInetGetErrno() && sceNetInetGetErrno() != EINTR) 
		//{
		//	ReportMessage ( "Error reading from socket or unexpected EOF.(0x%x, %d)\n",error, errno);
		//	m_sock_eof = true;
		//	Close();
		//	break;
		//}
	}
	return size;
}
