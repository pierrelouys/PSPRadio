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
#include "bstdfile.h"
#include <malloc.h>
#include "PSPSoundDecoder_OGG.h"
using namespace std;

void CPSPSoundDecoder_OGG::Initialize()
{
	Log(LOG_LOWLEVEL, "CPSPSoundDecoder_OGG Initialize"); 
	//IPSPSoundDecoder::IPSPSoundDecoder(InputStream, OutputBuffer);
	
	/* First the structures used by libmad must be initialized. */
	mad_stream_init(&m_Stream);
	mad_frame_init(&m_Frame);
	mad_synth_init(&m_Synth);
	mad_timer_reset(&m_Timer);
	
	m_GuardPtr = NULL;
	m_FrameCount=0;	
	m_pInputBuffer = (unsigned char*)malloc(INPUT_BUFFER_SIZE+MAD_BUFFER_GUARD);
	if (!m_pInputBuffer)
	{
		ReportError("Memory allocation error!\n");
		Log(LOG_ERROR, "Memory allocation error!\n");
		return;
	}
}
			
CPSPSoundDecoder_OGG::~CPSPSoundDecoder_OGG()
{
	Log(LOG_VERYLOW, "~CPSPSoundDecoder_OGG start");
	
	mad_synth_finish(&m_Synth);
	mad_frame_finish(&m_Frame);
	mad_stream_finish(&m_Stream); 
	
	if (m_pInputBuffer)
	{
		free(m_pInputBuffer), m_pInputBuffer = NULL;
	}
	Log(LOG_VERYLOW, "~CPSPSoundDecoder_OGG end");
}

bool CPSPSoundDecoder_OGG::Decode()
{
	int	i = 0;
	bool bRet = false;
	
	PCMFrameInHalfSamples PCMOutputFrame;/** The output buffer holds one BUFFER */
	


	/* The input bucket must be filled if it becomes empty or if
	 * it's the first execution of the loop.
	 */
	if(m_Stream.buffer==NULL || m_Stream.error==MAD_ERROR_BUFLEN)
	{
		size_t			ReadSize,
						Remaining;
		unsigned char	*ReadStart;

		if(m_Stream.next_frame!=NULL)
		{
			Remaining=m_Stream.bufend-m_Stream.next_frame;
			memmove(m_pInputBuffer,m_Stream.next_frame,Remaining);
			ReadStart=m_pInputBuffer+Remaining;
			ReadSize=INPUT_BUFFER_SIZE-Remaining;
		}
		else
			ReadSize=INPUT_BUFFER_SIZE,
				ReadStart=m_pInputBuffer,
				Remaining=0;

		ReadSize = m_InputStream->Read(ReadStart,1,ReadSize);
		if(ReadSize<=0)
		{
			ReportError("(End of stream)...");
			bRet = true;
			m_FrameCount = 0;
			return bRet;
		}

		if(m_InputStream->IsEOF())
		{
			m_GuardPtr=ReadStart+ReadSize;
			memset(m_GuardPtr,0,MAD_BUFFER_GUARD);
			ReadSize+=MAD_BUFFER_GUARD;
		}

		/* Pipe the new buffer content to libmad's stream decoder
		 * facility.
		 */
		mad_stream_buffer(&m_Stream,m_pInputBuffer,ReadSize+Remaining);
		m_Stream.error=(mad_error)0;
	}

	/* Decode the next MPEG frame. */
	if(mad_frame_decode(&m_Frame,&m_Stream))
	{
		if(MAD_RECOVERABLE(m_Stream.error))
		{
			if(m_Stream.error!=MAD_ERROR_LOSTSYNC ||
			   m_Stream.this_frame!=m_GuardPtr)
			{
				Log(LOG_INFO,"Recoverable frame level error. (Garbage in the stream).");
			}
			bRet = false;
			return bRet;
		}
		else
			if(m_Stream.error==MAD_ERROR_BUFLEN)
			{
				bRet = false;
				return bRet;
			}
			else
			{
				ReportError("Unrecoverable frame level error.");
				bRet=true;
				m_FrameCount = 0;
				return bRet;
			}
	}
	else

	/* The characteristics of the stream's first frame is printed
	 * on stderr. The first frame is representative of the entire
	 * stream.
	 */
	if(m_FrameCount==0)
	{
		if(PrintFrameInfo(&m_Frame.header))
		{
			bRet=true;
			m_FrameCount = 0;
			ReportError("Error in m_Frame info.");
			return bRet;
		}
		m_Buffer->SetSampleRate(m_Frame.header.samplerate);
	}

	/* Accounting. The computed frame duration is in the frame
	 * header structure. It is expressed as a fixed point number
	 * whole data type is mad_timer_t. It is different from the
	 * samples fixed point format and unlike it, it can't directly
	 * be added or subtracted. The timer module provides several
	 * functions to operate on such numbers. Be careful there, as
	 * some functions of libmad's timer module receive some of
	 * their mad_timer_t arguments by value!
	 */
	m_FrameCount++;
	mad_timer_add(&m_Timer,m_Frame.header.duration);

	//if(DoFilter)
	//	ApplyFilter(&m_Frame);

	/* Once decoded the frame is synthesized to PCM samples. No errors
	 * are reported by mad_synth_frame();
	 */
	mad_synth_frame(&m_Synth,&m_Frame);

	/* Synthesized samples must be converted from libmad's fixed
	 * point number to the consumer format. Here we use unsigned
	 * 16 bit little endian integers on two channels. Integer samples
	 * are temporarily stored in a buffer that is flushed when
	 * full.
	 */
	for(i=0;i<m_Synth.pcm.length;i++)
	{
		signed short	SampleL, SampleR;
		
		/* Left channel */
		SampleL = scale(m_Synth.pcm.samples[0][i]); 
		//Sample=MadFixedToSshort(m_Synth.pcm.samples[0][i]);
		/* Right channel. If the decoded stream is monophonic then
		 * the right output channel is the same as the left one.
		 */
		if(MAD_NCHANNELS(&m_Frame.header)==2)
		{
		//	Sample=MadFixedToSshort(m_Synth.pcm.samples[1][i]);
			SampleR = scale(m_Synth.pcm.samples[1][i]); 
		}
		else
		{
			SampleR = SampleL;
		}
		
		PCMOutputFrame.RHalfSampleA = (SampleR) & 0xff;
		PCMOutputFrame.RHalfSampleB = (SampleR>>8) & 0xff;
		PCMOutputFrame.LHalfSampleA = (SampleL) & 0xff;
		PCMOutputFrame.LHalfSampleB = (SampleL>>8) & 0xff;
		
		
		m_Buffer->PushFrame(*((::Frame*)&PCMOutputFrame));
	}
	
	return false;
}

signed int CPSPSoundDecoder_OGG::scale(mad_fixed_t &sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}


/****************************************************************************
 * Print human readable informations about an audio MPEG frame.				*
 ****************************************************************************/
int CPSPSoundDecoder_OGG::PrintFrameInfo(struct mad_header *Header)
{
	const char	*Layer,
				*Mode,
				*Emphasis;

	/* Convert the layer number to it's printed representation. */
	switch(Header->layer)
	{
		case MAD_LAYER_I:
			Layer="I";
			break;
		case MAD_LAYER_II:
			Layer="II";
			break;
		case MAD_LAYER_III:
			Layer="III";
			break;
		default:
			Layer="(unexpected layer value)";
			break;
	}

	/* Convert the audio mode to it's printed representation. */
	switch(Header->mode)
	{
		case MAD_MODE_SINGLE_CHANNEL:
			Mode="single channel";
			break;
		case MAD_MODE_DUAL_CHANNEL:
			Mode="dual channel";
			break;
		case MAD_MODE_JOINT_STEREO:
			Mode="joint (MS/intensity) stereo";
			break;
		case MAD_MODE_STEREO:
			Mode="normal LR stereo";
			break;
		default:
			Mode="(unexpected mode value)";
			break;
	}

	/* Convert the emphasis to it's printed representation. Note that
	 * the MAD_EMPHASIS_RESERVED enumeration value appeared in libmad
	 * version 0.15.0b.
	 */
	switch(Header->emphasis)
	{
		case MAD_EMPHASIS_NONE:
			Emphasis="no";
			break;
		case MAD_EMPHASIS_50_15_US:
			Emphasis="50/15 us";
			break;
		case MAD_EMPHASIS_CCITT_J_17:
			Emphasis="CCITT J.17";
			break;
			#if (MAD_VERSION_MAJOR>=1) || \
				((MAD_VERSION_MAJOR==0) && (MAD_VERSION_MINOR>=15))
					case MAD_EMPHASIS_RESERVED:
						Emphasis="reserved(!)";
						break;
			#endif
			default:
			Emphasis="(unexpected emphasis value)";
			break;
	}

	/**
	ReportError("%lu kb/s Audio MPEG layer %s stream %s CRC, "
			"%s with %s emphasis at %d Hz sample rate\n",
			Header->bitrate,Layer,
			Header->flags&MAD_FLAG_PROTECTION?"with":"without",
			Mode,Emphasis,Header->samplerate);
	*/
	pPSPSound->SendEvent(MID_DECODE_FRAME_INFO_HEADER, Header);
	pPSPSound->SendEvent(MID_DECODE_FRAME_INFO_LAYER, (char*)Layer);
	return(0);
}

/****************************************************************************
 * Converts a sample from libmad's fixed point number format to a signed	*
 * short (16 bits).															*
 ****************************************************************************/
signed short CPSPSoundDecoder_OGG::MadFixedToSshort(mad_fixed_t Fixed)
{
	/* Clipping */
	if(Fixed>=MAD_F_ONE)
		return(SHRT_MAX);
	if(Fixed<=-MAD_F_ONE)
		return(-SHRT_MAX);

	/* Conversion. */
	Fixed=Fixed>>(MAD_F_FRACBITS-15);
	return((signed short)Fixed);
} 