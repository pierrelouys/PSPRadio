#include <list>
#include <PSPApp.h>
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <mad.h>
#include "bstdfile.h"
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
	
	GetPCMBufferList()->empty();
	
	m_audiohandle = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, PSP_NUM_AUDIO_SAMPLES, PSP_AUDIO_FORMAT_STEREO);
	if ( m_audiohandle < 0 )
	{
		printf("Error getting a sound channel!\n");
	}
	else
	{
		printf("Retrieved handle=%d\n", m_audiohandle);
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
			m_thDecode = new CPSPThread("decode_thread", ThDecode, 0x11, 80000);
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
			GetPCMBufferList()->empty();
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
		//static char outbuf[OUTPUT_BUFFER_SIZE];
		audiobuffer *mybuf = NULL;
		//myPSPApp *pPSPSound = (myPSPApp*)pPSPApp;
		int ah = pPSPSound->GetAudioHandle();
		list<audiobuffer*> *PCMBufferList = pPSPSound->GetPCMBufferList();
		
		pspDebugScreenSetXY(0,15);
		printf ("Starting Play Thread (AudioHandle=%d)\n", ah);
		
		for(;;)
		{
			mybuf = PCMBufferList->front();
			if (mybuf)
			{
				//memcpy(outbuf, mybuf->buffer, OUTPUT_BUFFER_SIZE);
				sceAudioOutputPannedBlocking(ah, PSP_AUDIO_VOLUME_MAX,PSP_AUDIO_VOLUME_MAX,mybuf->buffer);
				free(mybuf), mybuf = NULL;
				PCMBufferList->pop_front();
				pspDebugScreenSetXY(0,16);
			}
			else
			{
				/** Buffer underrun! */
				pspDebugScreenSetXY(0,17);
				printf("! %03d   ", PCMBufferList->size());
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
