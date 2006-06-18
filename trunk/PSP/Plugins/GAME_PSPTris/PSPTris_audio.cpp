/*
	PSPTris - The game - Audio library
	Copyright (C) 2006  Jesper Sandberg

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

#include "PSPTris_audio.h"
#include <pspkernel.h>
#include <mikmod.h>

/* Thread ID for the audiochannel thread */
int mikModThreadID = -1;
static	MODULE *module = NULL;
static	bool	mikmod_done = false;

static int PSPTris_audio_thread(SceSize args, void *argp)
{
	while (!mikmod_done)
		{
		MikMod_Update();
		sceKernelDelayThread(1);
		}
	return (0);
}

static void PSPTris_audio_error_handler(void)
{
}

void PSPTris_audio_shutdown(void)
{
	mikmod_done = true;

	PSPTris_audio_stop_module();

	/*  Kill audiochannel thread */
	if (mikModThreadID > 0)
		{
		SceUInt timeout = 100000;
		sceKernelWaitThreadEnd(mikModThreadID, &timeout);
		sceKernelDeleteThread(mikModThreadID);
		}

	MikMod_Exit();
}

int PSPTris_audio_init(void)
{

	if (!MikMod_InitThreads())
		{
		printf("MikMod_InitThreads failed..\n");
		}

	/* Register error handler for MikMod */
	MikMod_RegisterErrorHandler(PSPTris_audio_error_handler);
	/* Register all the drivers */
	MikMod_RegisterAllDrivers();
	/* Register all the module loaders */
	MikMod_RegisterAllLoaders();

	/* Initialize the library */
	md_mode = DMODE_16BITS | DMODE_STEREO | DMODE_SOFT_SNDFX | DMODE_SOFT_MUSIC;

	if (MikMod_Init(""))
		{
		printf("Could not initialize sound, reason: %s.\n", MikMod_strerror(MikMod_errno));
		return PSPTRIS_AUDIO_ERR_SOUND;
   		}

	/* Get ready for playing */
	MikMod_SetNumVoices(-1, 8);
	MikMod_EnableOutput();

	/* Create the audiochannel thread */
	if ((mikModThreadID = sceKernelCreateThread("PSPTrisAudio" , &PSPTris_audio_thread, 0x12, 0x10000, 0, NULL)) > 0)
		{
		sceKernelStartThread(mikModThreadID, 0 , NULL);
		}
	else
		{
		printf("Could not create thread.\n");
		return PSPTRIS_AUDIO_ERR_THREAD;
		}

	return PSPTRIS_AUDIO_ERR_NONE;
}

void PSPTris_audio_stop_module(void)
{
	if (module != NULL)
		{
		Player_Stop();
		Player_Free(module);
		module = NULL;
		}
}

int PSPTris_audio_play_module(char *modname)
{
	PSPTris_audio_stop_module();

	/* Load module */
	module = Player_Load(modname, 64, 0);
	if (module)
		{
		/* Start module */
		Player_Start(module);
		}
	else
		{
		printf("Could not load module :  %s\n", modname);
		return PSPTRIS_AUDIO_ERR_MOD_LOAD;
		}
	return PSPTRIS_AUDIO_ERR_NONE;
}

int PSPTris_audio_load_sample(char *samplename, SAMPLE **sample)
{
	*sample = Sample_Load(samplename);
	MikMod_errno = 0;
	if (*sample == NULL)
		{
		printf("%s\n", MikMod_strerror(MikMod_errno));
		return PSPTRIS_AUDIO_ERR_SOUND;
		}
	return PSPTRIS_AUDIO_ERR_NONE;
}

void PSPTris_audio_play_sample(SAMPLE *sample)
{
	int voice = 0;
	voice = Sample_Play(sample,0,0);
	Voice_SetVolume(voice, 255);
}

void PSPTris_audio_free_sample(SAMPLE *sample)
{
	Sample_Free(sample);
}
