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
#include "PSPSound_MP3.h"
using namespace std;

int errno = 0;

//#define Log(level, format, args...) pPSPApp->m_Log.Log("CPSPSound_MP3", level, format, ## args)
#define ReportError pPSPApp->ReportError

CPSPSound_MP3 *pPSPSound_MP3 = NULL;

CPSPSound_MP3::CPSPSound_MP3()
{
	Log(LOG_LOWLEVEL, "PSPSound_MP3 Constructor");
	pPSPSound_MP3 = this;
	m_strFile[0] = 0;
	
}

/** Accessors */
void CPSPSound_MP3::SetFile(char *strFile)
{
	if (strFile)
	{
		strncpy(m_strFile, strFile, 256);
		Log(LOG_LOWLEVEL, "PSPSound_MP3. SetFile(%s) called", strFile);
	}
}

/** Threads */
void CPSPSound_MP3::Decode()
{
	struct mad_frame	Frame;
	struct mad_stream	Stream;
	struct mad_synth	Synth;
	mad_timer_t			Timer;
	const unsigned char	*OutputBufferEnd = NULL;
	int					Status=0,
						i = 0;
	unsigned long		FrameCount=0;

	unsigned char		*pInputBuffer 	= NULL, /*[INPUT_BUFFER_SIZE+MAD_BUFFER_GUARD]*/
						*pOutputBuffer	= NULL, /*[OUTPUT_BUFFER_SIZE]*/
						*OutputPtr		= NULL,
						*GuardPtr		= NULL;
	int count = 0;
	int iSampleRatio = 1;
	
	pInputBuffer = (unsigned char*)malloc(INPUT_BUFFER_SIZE+MAD_BUFFER_GUARD);
	pOutputBuffer = (unsigned char*)malloc(OUTPUT_BUFFER_SIZE);

	if (!(pInputBuffer && pOutputBuffer))
	{
		ReportError("Memory allocation error!\n");
		Log(LOG_ERROR, "Memory allocation error!\n");
		return;
	}

	OutputPtr=pOutputBuffer;
	GuardPtr = NULL;
	OutputBufferEnd = pOutputBuffer+OUTPUT_BUFFER_SIZE;
	
	/* First the structures used by libmad must be initialized. */
	mad_stream_init(&Stream);
	mad_frame_init(&Frame);
	mad_synth_init(&Synth);
	mad_timer_reset(&Timer);

	CPSPSoundStream *InputStream = new CPSPSoundStream();
	
	pPSPSound_MP3->SendMessage(MID_DECODE_STREAM_OPENING);
	Log(LOG_INFO, "MP3 Decode(): Calling Open For '%s'", pPSPSound_MP3->GetFile());
	InputStream->Open(pPSPSound_MP3->GetFile());
	if (InputStream->IsOpen() == TRUE)
	{
		Log(LOG_INFO, "MP3 Decode(): Stream Opened Successfully.");
		
		pPSPSound_MP3->SendMessage(MID_DECODE_STREAM_OPEN);
		
		/** Main decoding loop */
		/* pPSPSound_MP3 is the decoding loop. */
		while (pPSPApp->m_Exit == FALSE && pPSPSound_MP3->GetPlayState() != STOP)
		{
			/* The input bucket must be filled if it becomes empty or if
			 * it's the first execution of the loop.
			 */
			if(Stream.buffer==NULL || Stream.error==MAD_ERROR_BUFLEN)
			{
				size_t			ReadSize,
								Remaining;
				unsigned char	*ReadStart;
	
				if(Stream.next_frame!=NULL)
				{
					Remaining=Stream.bufend-Stream.next_frame;
					memmove(pInputBuffer,Stream.next_frame,Remaining);
					ReadStart=pInputBuffer+Remaining;
					ReadSize=INPUT_BUFFER_SIZE-Remaining;
				}
				else
					ReadSize=INPUT_BUFFER_SIZE,
						ReadStart=pInputBuffer,
						Remaining=0;
	
				ReadSize = InputStream->Read(ReadStart,1,ReadSize);
				if(ReadSize<=0)
				{
					//if(InputStream->GetError())
					//{
					//	ReportError("%s: read error on bit-stream (%s)\n",
					//			pPSPApp->GetProgramName(),strerror(errno));
					//	Status=1;
					//}
					//if(feof(InputFp))
					//	ReportError("%s: end of input stream\n",pPSPApp->GetProgramName());
					//else
						ReportError("Read error (End of stream)...\n");
					break;
				}
				else if(pPSPSound_MP3->GetPlayState() == STOP)
				{
					break;
				}
				
	
				if(InputStream->IsEOF())
				{
					GuardPtr=ReadStart+ReadSize;
					memset(GuardPtr,0,MAD_BUFFER_GUARD);
					ReadSize+=MAD_BUFFER_GUARD;
				}
	
				/* Pipe the new buffer content to libmad's stream decoder
	             * facility.
				 */
				mad_stream_buffer(&Stream,pInputBuffer,ReadSize+Remaining);
				Stream.error=(mad_error)0;
			}
	
			/* Decode the next MPEG frame. */
			if(mad_frame_decode(&Frame,&Stream))
			{
				if(MAD_RECOVERABLE(Stream.error))
				{
					if(Stream.error!=MAD_ERROR_LOSTSYNC ||
					   Stream.this_frame!=GuardPtr)
					{
						/** Don't log if recoverable. */
						//ReportError("%s: recoverable frame level error. \n",
						//		pPSPApp->GetProgramName());
						Log(LOG_INFO,"Recoverable frame level error. (Garbage in the stream).");

					}
					continue;
				}
				else
					if(Stream.error==MAD_ERROR_BUFLEN)
						continue;
					else
					{
						ReportError("Unrecoverable frame level error.");
						Status=1;
						break;
					}
			}
			else
	
			/* The characteristics of the stream's first frame is printed
			 * on stderr. The first frame is representative of the entire
			 * stream.
			 */
			if(FrameCount==0)
			{
				if(PrintFrameInfo(&Frame.header))
				{
					Status=1;
					ReportError("Error in Frame info.");
					break;
				}
				iSampleRatio = PSP_SAMPLERATE / Frame.header.samplerate;
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
			FrameCount++;
			mad_timer_add(&Timer,Frame.header.duration);
	
			//if(DoFilter)
			//	ApplyFilter(&Frame);
	
			/* Once decoded the frame is synthesized to PCM samples. No errors
			 * are reported by mad_synth_frame();
			 */
			mad_synth_frame(&Synth,&Frame);
	
			/* Synthesized samples must be converted from libmad's fixed
			 * point number to the consumer format. Here we use unsigned
			 * 16 bit little endian integers on two channels. Integer samples
			 * are temporarily stored in a buffer that is flushed when
			 * full.
			 */
			for(i=0;i<Synth.pcm.length;i++)
			{
				signed short	SampleL, SampleR;
				
				/* Left channel */
				SampleL = scale(Synth.pcm.samples[0][i]); 
				//Sample=MadFixedToSshort(Synth.pcm.samples[0][i]);
				/* Right channel. If the decoded stream is monophonic then
				 * the right output channel is the same as the left one.
				 */
				if(MAD_NCHANNELS(&Frame.header)==2)
				{
				//	Sample=MadFixedToSshort(Synth.pcm.samples[1][i]);
					SampleR = scale(Synth.pcm.samples[1][i]); 
				}
				else
				{
					SampleR = SampleL;
				}
				for (int i = 0 ; i < iSampleRatio ; i++)
				{
					*(OutputPtr++)=((SampleL >> 0) & 0xff);
					*(OutputPtr++)=((SampleL >> 8) & 0xff);
					*(OutputPtr++)=((SampleR >> 0) & 0xff);
					*(OutputPtr++)=((SampleR >> 8) & 0xff);
				}
				
				/* Queue the output buffer if it is full. */
				if(OutputPtr==OutputBufferEnd)
				{
					pPSPSound_MP3->Buffer.Push((char*)pOutputBuffer);

					if (count++ % 5 == 0)
					{
						pPSPSound_MP3->SendMessage(MID_DECODE_BUFCYCLE);
					}
						
					OutputPtr=pOutputBuffer;
				}

				if (pPSPApp->m_Exit == TRUE || pPSPSound_MP3->GetPlayState() == STOP)
				{
					break;
				}

			}
			sceKernelDelayThread(10); /** 100us */

		};
		//ReportError("Done.\n");
		Log(LOG_INFO, "Done decoding stream.");
		pPSPSound_MP3->SendMessage(MID_DECODE_DONE);
		pPSPSound_MP3->Buffer.Done();
		
		/* The input file was completely read; the memory allocated by our
		 * reading module must be reclaimed.
		 */
		delete(InputStream);
	
		/* Mad is no longer used, the structures that were initialized must
	     * now be cleared.
		 */
		mad_synth_finish(&Synth);
		mad_frame_finish(&Frame);
		mad_stream_finish(&Stream); 

	}
	else
	{
		pPSPSound_MP3->SendMessage(MID_DECODE_STREAM_OPEN_ERROR);
		Log(LOG_ERROR, "Unable to open stream.");
	}

}

signed int CPSPSound_MP3::scale(mad_fixed_t &sample)
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
int CPSPSound_MP3::PrintFrameInfo(struct mad_header *Header)
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
	pPSPSound_MP3->SendMessage(MID_DECODE_FRAME_INFO_HEADER, Header);
	pPSPSound_MP3->SendMessage(MID_DECODE_FRAME_INFO_LAYER, (char*)Layer);
	return(0);
}

/****************************************************************************
 * Converts a sample from libmad's fixed point number format to a signed	*
 * short (16 bits).															*
 ****************************************************************************/
signed short CPSPSound_MP3::MadFixedToSshort(mad_fixed_t Fixed)
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
