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

#ifndef _PSPTRIS_AUDIO_H_
#define _PSPTRIS_AUDIO_H_

#include <mikmod.h>

/* Errorcodes returned from library */
enum PSPTRIS_AUDIO_ERRORS
	{
	PSPTRIS_AUDIO_ERR_NONE = 1,
	PSPTRIS_AUDIO_ERR_THREAD,
	PSPTRIS_AUDIO_ERR_INIT,
	PSPTRIS_AUDIO_ERR_SOUND,
	PSPTRIS_AUDIO_ERR_MOD_LOAD,
	} ;


/* Prototypes */
void PSPTris_audio_shutdown(void);
int PSPTris_audio_init(void);
int PSPTris_audio_play_module(char *modname);
void PSPTris_audio_stop_module(void);

int PSPTris_audio_load_sample(char *samplename, SAMPLE **sample);
void PSPTris_audio_play_sample(SAMPLE *sample);
void PSPTris_audio_free_sample(SAMPLE *sample);

#endif /* _PSPTRIS_AUDIO_H_ */
