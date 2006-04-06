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
#ifndef __PSPSOUNDDEVICEBUFFER__
	#define __PSPSOUNDDEVICEBUFFER__

	#include <pspaudio.h>

	/** Not configurable */
	#define PSP_SAMPLERATE			44100
	#define NUM_CHANNELS			2	/** L and R */
	#define BYTES_PER_SAMPLE		2	/** 16bit sound */
	/** Useful macros */
	/** Convert number of frames/samples/bytes to eachother */
	#define FRAMES_TO_SAMPLES(f)	(f*NUM_CHANNELS)
	#define SAMPLES_TO_BYTES(s)		(s*BYTES_PER_SAMPLE)
	#define BYTES_TO_FRAMES(b)		(b/(NUM_CHANNELS*BYTES_PER_SAMPLE))
	#define BYTES_TO_SAMPLES(b)		(b/(BYTES_PER_SAMPLE))
	#define FRAMES_TO_BYTES(f)		(f*(NUM_CHANNELS*BYTES_PER_SAMPLE))
	typedef s16 Sample;
	typedef u32 Frame;
	typedef struct
	{
		u8 RHalfSampleA;
		u8 RHalfSampleB;
		u8 LHalfSampleA;
		u8 LHalfSampleB;
	} PCMFrameInHalfSamples;
	typedef struct
	{
		Sample RSample;
		Sample LSample;
	} PCMFrameInSamples;

	/** Configurable */
	/* (frames are 2ch, 16bits, so 4096frames =16384bytes =8192samples-l-r-combined.)*/
	#define PSP_BUFFER_SIZE_IN_FRAMES	PSP_AUDIO_SAMPLE_ALIGN(2048)
	#define DEFAULT_NUM_BUFFERS		20		/** Default */

	#define INPUT_BUFFER_SIZE		16302

	typedef struct 
	{
		Frame data[PSP_BUFFER_SIZE_IN_FRAMES];
	} DeviceBuffer;

#endif
