/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * pspaudiolib.c - Audio library build on top of sceAudio, but to provide
 *                 multiple thread usage and callbacks.
 *
 * Copyright (c) 2005 Adresd
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 *
 * $Id: pspaudiolib.c 645 2005-07-13 22:53:15Z warren $
 */
#include <stdlib.h>
#include <string.h>
#include <pspthreadman.h>
#include <pspaudio.h>
#include <malloc.h>
#include "pspaudiolib.h"

static int audio_ready=0;
static psp_audio_channelinfo AudioStatus;

static volatile int audio_terminate=0;

void pspAudioSetVolume(int left, int right)
{
  AudioStatus.volumeright = right;
  AudioStatus.volumeleft  = left;
}

void pspAudioChannelThreadCallback(void *buf, unsigned int reqn)
{
	audiocallback cb;
	cb=AudioStatus.callback;
}


void pspAudioSetChannelCallback(void *callback)
{
	AudioStatus.callback=(audiocallback)callback;
}

int pspAudioOutBlocking(unsigned int vol1, unsigned int vol2, void *buf)
{
	if (!audio_ready) return -1;
	if (vol1>PSP_VOLUME_MAX) vol1=PSP_VOLUME_MAX;
	if (vol2>PSP_VOLUME_MAX) vol2=PSP_VOLUME_MAX;
	return sceAudioOutputPannedBlocking(AudioStatus.handle,vol1,vol2,buf);
}

static int AudioChannelThread(int args, void *argp)
{
	
	unsigned char *bufptr = (unsigned char*)memalign(64, PSP_AUDIO_BUFFER_SIZE);
	if (bufptr)
	{
		while (audio_terminate==0) {
			//void *bufptr=&audio_sndbuf[channel][bufidx];
			if (AudioStatus.callback) {
				AudioStatus.callback(bufptr, PSP_NUM_AUDIO_SAMPLES);
			} else {
				//unsigned int *ptr=(unsigned int*)bufptr;
				//int i;
				//for (i=0; i<PSP_NUM_AUDIO_SAMPLES; ++i) *(ptr++)=0;
				memset(bufptr, 0, PSP_AUDIO_BUFFER_SIZE);
			}
			sceAudioOutputPannedBlocking(AudioStatus.handle,AudioStatus.volumeleft,AudioStatus.volumeright,bufptr);
			//memset(bufptr, 0, PSP_NUM_AUDIO_SAMPLES);
			//bufidx=(bufidx?0:1);
		}
	}
	free (bufptr);
	sceKernelExitThread(0);
	return 0;
}



/******************************************************************************/



int pspAudioInit()
{
	int ret;
	int failed=0;

	audio_terminate=0;
	audio_ready=0;

    AudioStatus.handle = -1;
    AudioStatus.threadhandle = -1;
    AudioStatus.volumeright = PSP_VOLUME_MAX;
    AudioStatus.volumeleft  = PSP_VOLUME_MAX;
    AudioStatus.callback = NULL;

    if ((AudioStatus.handle = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, PSP_NUM_AUDIO_SAMPLES, PSP_AUDIO_FORMAT_STEREO))<0) 
    {
      failed=1;
	}
	if (failed) 
	{
		if (AudioStatus.handle != -1) 
		{
        	sceAudioChRelease(AudioStatus.handle);
			AudioStatus.handle = -1;
		}
		return -1;
	}
	audio_ready = 1;
	AudioStatus.threadhandle = sceKernelCreateThread("audiothread",(SceKernelThreadEntry)&AudioChannelThread,0x11,0x10000,0,NULL);
	if (AudioStatus.threadhandle < 0) 
	{
		AudioStatus.threadhandle = -1;
		failed=1;
	}
	else
	{
		ret=sceKernelStartThread(AudioStatus.threadhandle,0,NULL);
		if (ret!=0) 
		{
			failed=1;
		}
	}
	if (failed) 
	{
		audio_terminate=1;
		if (AudioStatus.threadhandle != -1) 
		{
			//sceKernelWaitThreadEnd(AudioStatus[i].threadhandle,NULL);
			sceKernelDeleteThread(AudioStatus.threadhandle);
		}
		AudioStatus.threadhandle = -1;
		audio_ready=0;
		return -1;
	}
	return 0;
}


void pspAudioEndPre()
{
	audio_ready=0;
	audio_terminate=1;
}


void pspAudioEnd()
{
	audio_ready=0;
	audio_terminate=1;

	if (AudioStatus.threadhandle != -1) 
	{
		//sceKernelWaitThreadEnd(AudioStatus[i].threadhandle,NULL);
		sceKernelDeleteThread(AudioStatus.threadhandle);
	}
	AudioStatus.threadhandle = -1;

	if (AudioStatus.handle != -1) 
	{
		sceAudioChRelease(AudioStatus.handle);
		AudioStatus.handle = -1;
	}
}


