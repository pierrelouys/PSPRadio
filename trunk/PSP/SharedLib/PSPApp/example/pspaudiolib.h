/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * pspaudiolib.h - Audio library build on top of sceAudio, but to provide
 *                 multiple thread usage and callbacks.
 *
 * Copyright (c) 2005 Adresd
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 *
 * $Id: pspaudiolib.h 540 2005-07-08 19:35:10Z warren $
 */
#ifndef __AUDIOLIB_H__
#define __AUDIOLIB_H__

#ifdef __cplusplus
extern "C" {
#endif

//#define PSP_NUM_AUDIO_CHANNELS 4
//#define PSP_NUM_AUDIO_SAMPLES 1024
#define PSP_NUM_AUDIO_SAMPLES PSP_AUDIO_SAMPLE_ALIGN(8192)
#define PSP_AUDIO_BUFFER_SIZE PSP_NUM_AUDIO_SAMPLES*2*16
#define PSP_VOLUME_MAX 0x8000

typedef void (*audiocallback)(void *buf, unsigned int len);
typedef struct {
  int threadhandle;
  int handle;
  int volumeleft;
  int volumeright;
  audiocallback callback;
} psp_audio_channelinfo;

typedef int (* pspAudioThreadfunc_t)(int args, void *argp);

int  pspAudioInit();
void pspAudioEndPre();
void pspAudioEnd();

void pspAudioSetVolume(int left, int right);
void pspAudioChannelThreadCallback(void *buf, unsigned int reqn);
void pspAudioSetChannelCallback(void *callback);
int  pspAudioOutBlocking(unsigned int vol1, unsigned int vol2, void *buf);

#ifdef __cplusplus
}
#endif

#endif
