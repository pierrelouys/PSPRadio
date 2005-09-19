#include <list>
#include <PSPApp.h>
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <mad.h>
#include <malloc.h>
#include "PSPSound.h"

using namespace std;

CPSPSound *pPSPSound = NULL;

/** Accessors */
int CPSPSound::GetAudioHandle()
{
	return m_audiohandle;
}

CPSPSound::CPSPSound()
{
	Initialize();
}

void CPSPSound::Initialize()
{
	if (pPSPSound != NULL)
	{
		printf("Error!, only one instance of CPSPSound (including CPSPSound_*) permitted!");
	}
	pPSPSound = this;
	
	Buffer.Empty();
	
	m_audiohandle = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, PSP_NUM_AUDIO_SAMPLES, PSP_AUDIO_FORMAT_STEREO);
	if ( m_audiohandle < 0 )
	{
		printf("Error getting a sound channel!\n");
	}
	
	m_CurrentState = STOP;
	m_thDecode = NULL;
	m_thPlayAudio  = NULL;
}

CPSPSound::~CPSPSound()
{
	Stop();
}

int CPSPSound::Play()
{
	switch(m_CurrentState)
	{
		case STOP:
			if (m_thDecode)
			{
				delete(m_thDecode), m_thDecode = NULL;
			}
			if (m_thPlayAudio)
			{
				delete(m_thPlayAudio), m_thPlayAudio = NULL;
			}
			m_thDecode = new CPSPThread("decode_thread", ThDecode, 0x20, 80000);
			m_thPlayAudio = new CPSPThread("playaudio_thread", ThPlayAudio, 0x11, 80000);
			m_thDecode->Start();
			sceKernelDelayThread(500000); /** 500ms */
			m_thPlayAudio->Start();
			m_CurrentState = PLAY;
			break;
			
		case PAUSE:
			m_thDecode->Resume();
			sceKernelDelayThread(500000); /** 500ms */
			m_thPlayAudio->Resume();
			m_CurrentState = PLAY;
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
			m_thDecode->Suspend();
			m_thPlayAudio->Suspend();
			m_CurrentState = PAUSE;
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
		case PLAY:
		case PAUSE:
			Buffer.Empty();
			if (m_thDecode)
			{
				delete(m_thDecode), m_thDecode = NULL;
			}
			if (m_thPlayAudio)
			{
				delete(m_thPlayAudio), m_thPlayAudio = NULL;
			}
			m_CurrentState = STOP;
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
		char *mybuf = NULL;
		int ah = pPSPSound->GetAudioHandle();
		
		pspDebugScreenSetXY(0,15);
		printf ("Starting Play Thread (AudioHandle=%d)\n", ah);
		
		for(;;)
		{
			if (pPSPSound->Buffer.IsDone() || pPSPApp->m_Exit == TRUE)
			{
				break;
			}
			mybuf = pPSPSound->Buffer.Pop();
			if (mybuf)
			{
				sceAudioOutputPannedBlocking(ah, PSP_AUDIO_VOLUME_MAX, PSP_AUDIO_VOLUME_MAX, mybuf);
			}
			else
			{
				/** Buffer underrun! */
				pspDebugScreenSetXY(0,17);
				printf("! %03d   ", pPSPSound->Buffer.GetPushPos());
				sceKernelDelayThread(100000); /** 100ms */
			}
		}
		sceKernelExitThread(0);
		return 0;
}

int CPSPSound::ThDecode(SceSize args, void *argp)
{
	pPSPSound->Decode();
	sceKernelExitThread(0);

	return 0;
}

void CPSPSound::Decode()
{
	printf("Not Implemented!");
}

/** Sound buffer class implementation */
#if 0
CPSPSoundBuffer::CPSPSoundBuffer()
{
}
int   CPSPSoundBuffer::GetBufferSize() 
{ 
	return m_PCMBufferList.size(); 
}
void  CPSPSoundBuffer::Empty() 
{ 
	m_PCMBufferList.empty(); 
}

void CPSPSoundBuffer::PushBuffer(char *buf)
{
	audiobuffer *mybuffer = (audiobuffer*)(char*)memalign(64, sizeof(audiobuffer));
	memcpy(mybuffer->buffer, buf, OUTPUT_BUFFER_SIZE);
	m_PCMBufferList.push_back(mybuffer);
}

char * CPSPSoundBuffer::PopBuffer()
{
	static audiobuffer *sbuf = NULL;
	if (sbuf == NULL)
	{
		sbuf = (audiobuffer*)(char*)memalign(64, sizeof(audiobuffer));
	}
	audiobuffer *pBuf = NULL;
	pBuf = m_PCMBufferList.front();
	memcpy(sbuf, pBuf, sizeof(audiobuffer));
	free(pBuf), pBuf = NULL;
	m_PCMBufferList.pop_front();
	
	return sbuf->buffer;
}
#endif
CPSPSoundBuffer::CPSPSoundBuffer()
{
	ringbuf = (char*)memalign(64, OUTPUT_BUFFER_SIZE * (NUM_BUFFERS + 5));
	Empty();
}
int CPSPSoundBuffer::GetPushPos() 
{ 
	return pushpos; 
}
void  CPSPSoundBuffer::Empty() 
{ 
	memset(ringbuf, 0, OUTPUT_BUFFER_SIZE * NUM_BUFFERS);
	pushpos = poppos = 0;
	m_lastpushpos = -1;
}

void CPSPSoundBuffer::Push(char *buf)
{
	while(pushpos - poppos > NUM_BUFFERS/2)
	{
		sceKernelDelayThread(500); /** 500us */
	}
	memcpy(ringbuf+(pushpos*OUTPUT_BUFFER_SIZE), buf, OUTPUT_BUFFER_SIZE);
	pushpos = (pushpos + 1) % NUM_BUFFERS;
}

char * CPSPSoundBuffer::Pop()
{
	char *ret = ringbuf+(poppos*OUTPUT_BUFFER_SIZE);
	poppos = (poppos + 1) % NUM_BUFFERS;
	return ret;
}

void CPSPSoundBuffer::Done()
{
	m_lastpushpos = pushpos;
}

int CPSPSoundBuffer::IsDone()
{
	if (m_lastpushpos == poppos)
	{
		return 1;
	}
	
	return 0;
}

/** class CPSPSoundStream */
CPSPSoundStream::CPSPSoundStream()
{
	m_Type = STREAM_TYPE_CLOSED;
	m_pfd = NULL;
	m_BstdFile = NULL;
	m_fd = -1;
	m_sock_eof = TRUE;
}

CPSPSoundStream::~CPSPSoundStream()
{
	switch(m_Type)
	{
		case STREAM_TYPE_FILE:
			BstdFileDestroy(m_BstdFile);
			fclose(m_pfd);
			m_Type = STREAM_TYPE_CLOSED;
			break;
		case STREAM_TYPE_URL:
			sceNetInetClose(m_fd);
			m_Type = STREAM_TYPE_CLOSED;
			break;
		case STREAM_TYPE_CLOSED:
			break;
	}	
}

int CPSPSoundStream::Open(char *filename)
{
	switch(m_Type)
	{
		case STREAM_TYPE_CLOSED:
			if (filename && strlen(filename) > 4)
			{
				if (memcmp(filename, "http://", strlen("http://")) == 0)
				{
					printf ("Opening URL '%s'\n", filename);
					m_fd = http_open(filename);
					if (m_fd < 0)
					{
						printf("CPSPSoundStream::OpenFile-Error opening URL.\n");
						m_Type = STREAM_TYPE_CLOSED;
					}
					else
					{
						printf("CPSPSoundStream::OpenFile-URL Opened. (handle=%d)\n", m_fd);
						m_Type = STREAM_TYPE_URL;
						m_sock_eof = FALSE;
					}
				}
				else
				{
					m_pfd = fopen(filename, "rb");
					m_BstdFile=NewBstdFile(m_pfd);
					if(m_BstdFile != NULL)
					{
						m_Type = STREAM_TYPE_FILE;
					}
					else
					{
						printf("CPSPSoundStream::OpenFile-Can't create a new bstdfile_t (%s).\n",
								strerror(errno));
					}
				}
			}
			else
				printf("CPSPSoundStream::OpenFile-Invalid filename '%s'\n", filename);
			break;
		case STREAM_TYPE_FILE:
		case STREAM_TYPE_URL:
			printf("Calling OpenFile, but there is a file open already\n");
			break;
	}
	
	return m_Type!=STREAM_TYPE_CLOSED?0:-1;
}

//#define write sceIoWrite
//#define read  sceIoRead
size_t CPSPSoundStream::Read(unsigned char *pBuffer, size_t ElementSize, size_t ElementCount)
{
	size_t size = 0;
	size_t bytesread = 0;
	
	switch(m_Type)
	{
		case STREAM_TYPE_FILE:
			size = BstdRead(pBuffer, ElementSize, ElementCount, m_BstdFile);
			break;
		case STREAM_TYPE_URL:
			for(;;) 
			{
				bytesread = recv(m_fd, pBuffer+size, ElementCount-size, 0);
				if (bytesread > 0)
					size += bytesread;
				if(bytesread == ElementCount-size) 
				{
					break;
				}
				else if (bytesread == 0)
				{
					printf ( "Connection closed by peer!\n");
					m_sock_eof = TRUE;
					break;
				}
				//else if(errno != EINTR) 
				//{
				//	printf ( "Error reading from socket or unexpected EOF.\n");
				//	m_sock_eof = TRUE;
				//	break;
				//}
			}		
			break;
		case STREAM_TYPE_CLOSED:
			break;
	}
	
	return size;
}

BOOLEAN CPSPSoundStream::IsEOF()
{
	int iseof = 0;
	
	switch(m_Type)
	{
		case STREAM_TYPE_FILE:
			iseof = BstdFileEofP(m_BstdFile);
			break;
		case STREAM_TYPE_URL:
			iseof = m_sock_eof;
			break;
		case STREAM_TYPE_CLOSED:
			break;
	}
	
	return iseof?TRUE:FALSE;
}

BOOLEAN CPSPSoundStream::IsOpen()
{
	return (m_Type==STREAM_TYPE_CLOSED)?FALSE:TRUE;
}
