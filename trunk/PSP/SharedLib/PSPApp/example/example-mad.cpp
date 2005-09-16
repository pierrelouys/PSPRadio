/* 
	PSPApp Example 2
	Using MAD
*/

#include <list>
#include <PSPApp.h>

#include <stdio.h>
#include <unistd.h> 
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
//#include <math.h>
#include <limits.h>
#include <mad.h>
#include "bstdfile.h"

char *ProgName = "MADEXAMPLE";
/* Define the module info section */
PSP_MODULE_INFO("MADEXAMPLE", 0, 1, 1);
/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

int errno = 0;
int g_playpos = 0;

/* DoFilter is non-nul when the Filter table defines a filter bank to
 * be applied to the decoded audio subbands.
 */
int			DoFilter=0;

#define INPUT_BUFFER_SIZE	(5*8192)
//#define OUTPUT_BUFFER_SIZE	(8192) /* Must be an integer multiple of 4. */ 
#define OUTPUT_BUFFER_SIZE PSP_AUDIO_BUFFER_SIZE
#if 0
unsigned char		InputBuffer[INPUT_BUFFER_SIZE+MAD_BUFFER_GUARD],
					OutputBuffer[OUTPUT_BUFFER_SIZE],
					*OutputPtr=OutputBuffer,
					*GuardPtr=NULL;
#endif
unsigned char		*pInputBuffer/*[INPUT_BUFFER_SIZE+MAD_BUFFER_GUARD]*/,
					*pOutputBuffer/*[OUTPUT_BUFFER_SIZE]*/,
					*pPlayBuffer,
					*OutputPtr=pOutputBuffer,
					*GuardPtr=NULL;
					
#define NUM_BUFFERS 50
//#define MIN_BUFFERS 15
class audiobuffer;
struct audiobuffer
{ 
	char buffer[OUTPUT_BUFFER_SIZE]; 
};
using namespace std;
list<audiobuffer*> bufferlist;
int currentpopbuffer = 0;
char *buffer = NULL;
int position = 0;

#define BUFFER_SIZE (OUTPUT_BUFFER_SIZE * (NUM_BUFFERS+1))

class myPSPApp : public CPSPApp
{
public:
	CPSPThread *m_thDecodeFile;
	int Setup()
	{
		printf("Loading mp3, and decoding...\n");
		
		pInputBuffer = (unsigned char*)malloc(INPUT_BUFFER_SIZE+MAD_BUFFER_GUARD);
		pOutputBuffer = (unsigned char*)malloc(OUTPUT_BUFFER_SIZE);
		buffer = (char*)memalign(64, BUFFER_SIZE );
		
		printf("buffer size = %d\n", BUFFER_SIZE);
		if (pInputBuffer && pOutputBuffer&&buffer)
		{
			OutputPtr=pOutputBuffer;
			//memset(silence, 0, 1024);
			memset(OutputPtr, 0, OUTPUT_BUFFER_SIZE);
			
			m_thDecodeFile = new CPSPThread("filedecode_thread", ThDecodeFile, 0x11, 100000);
		
			//EnableAudio(); /** Tells PSPApp to start the music! */
			m_thDecodeFile->Start();
			//FileDecode();
		}
		else
			printf("Memory allocation error!\n");
		
		return 0;
	}
	
	static int ThDecodeFile(SceSize args, void *argp)
	//int FileDecode()
	{
		struct mad_frame	Frame;
		struct mad_stream	Stream;
		struct mad_synth	Synth;
		mad_timer_t			Timer;
		//unsigned char		InputBuffer[INPUT_BUFFER_SIZE+MAD_BUFFER_GUARD],
		//					OutputBuffer[OUTPUT_BUFFER_SIZE],
		//					*OutputPtr=OutputBuffer,
		//					*GuardPtr=NULL;
		const unsigned char	*OutputBufferEnd;
		int					Status=0,
							i;
		unsigned long		FrameCount=0;
		bstdfile_t			*BstdFile = NULL;
		int iBufferReady = 0;
		printf ("Decoding Started!\n");
		printf ("Buffering...\n");
		
		OutputBufferEnd = pOutputBuffer+OUTPUT_BUFFER_SIZE;
		
		/* First the structures used by libmad must be initialized. */
		mad_stream_init(&Stream);
		mad_frame_init(&Frame);
		mad_synth_init(&Synth);
		mad_timer_reset(&Timer);
		
		// Initialize channel and allocate buffer (PSP_AUDIO_BUFFER_SIZE)
		int OutputChannel = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, PSP_NUM_AUDIO_SAMPLES, PSP_AUDIO_FORMAT_STEREO);
		if ( OutputChannel < 0 )
		{
			printf("Error getting a sound channel!\n");
			return 0;
		}

	
		/* Decoding options can here be set in the options field of the
		 * Stream structure.
		 */
	
			
		FILE *InputFp = fopen("ms0:/whisper.mp3", "rb");
		if (InputFp != NULL)
		{
			/* {1} When decoding from a file we need to know when the end of
			 * the file is reached at the same time as the last bytes are read
			 * (see also the comment marked {3} bellow). Neither the standard
			 * C fread() function nor the POSIX read() system call provides
			 * this feature. We thus need to perform our reads through an
			 * interface having this feature, this is implemented here by the
			 * bstdfile.c module.
			 */
			BstdFile=NewBstdFile(InputFp);
			if(BstdFile==NULL)
			{
				printf("%s: can't create a new bstdfile_t (%s).\n",
						ProgName,strerror(errno));
				return(1);
			} 
			
			/** Main decoding loop */
			/* This is the decoding loop. */
			audiobuffer *mybuffer = NULL;
			for(;;)
			{
				/* The input bucket must be filled if it becomes empty or if
				 * it's the first execution of the loop.
				 */
				if(Stream.buffer==NULL || Stream.error==MAD_ERROR_BUFLEN)
				{
					size_t			ReadSize,
									Remaining;
					unsigned char	*ReadStart;
		
					/* {2} libmad may not consume all bytes of the input
					 * buffer. If the last frame in the buffer is not wholly
					 * contained by it, then that frame's start is pointed by
					 * the next_frame member of the Stream structure. This
					 * common situation occurs when mad_frame_decode() fails,
					 * sets the stream error code to MAD_ERROR_BUFLEN, and
					 * sets the next_frame pointer to a non NULL value. (See
					 * also the comment marked {4} bellow.)
					 *
					 * When this occurs, the remaining unused bytes must be
					 * put back at the beginning of the buffer and taken in
					 * account before refilling the buffer. This means that
					 * the input buffer must be large enough to hold a whole
					 * frame at the highest observable bit-rate (currently 448
					 * kb/s). XXX=XXX Is 2016 bytes the size of the largest
					 * frame? (448000*(1152/32000))/8
					 */
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
		
					/* Fill-in the buffer. If an error occurs print a message
					 * and leave the decoding loop. If the end of stream is
					 * reached we also leave the loop but the return status is
					 * left untouched.
					 */
					ReadSize=BstdRead(ReadStart,1,ReadSize,BstdFile);
					if(ReadSize<=0)
					{
						if(ferror(InputFp))
						{
							printf("%s: read error on bit-stream (%s)\n",
									ProgName,strerror(errno));
							Status=1;
						}
						if(feof(InputFp))
							printf("%s: end of input stream\n",ProgName);
						break;
					}
		
					/* {3} When decoding the last frame of a file, it must be
					 * followed by MAD_BUFFER_GUARD zero bytes if one wants to
					 * decode that last frame. When the end of file is
					 * detected we append that quantity of bytes at the end of
					 * the available data. Note that the buffer can't overflow
					 * as the guard size was allocated but not used the the
					 * buffer management code. (See also the comment marked
					 * {1}.)
					 *
					 * In a message to the mad-dev mailing list on May 29th,
					 * 2001, Rob Leslie explains the guard zone as follows:
					 *
					 *    "The reason for MAD_BUFFER_GUARD has to do with the
					 *    way decoding is performed. In Layer III, Huffman
					 *    decoding may inadvertently read a few bytes beyond
					 *    the end of the buffer in the case of certain invalid
					 *    input. This is not detected until after the fact. To
					 *    prevent this from causing problems, and also to
					 *    ensure the next frame's main_data_begin pointer is
					 *    always accessible, MAD requires MAD_BUFFER_GUARD
					 *    (currently 8) bytes to be present in the buffer past
					 *    the end of the current frame in order to decode the
					 *    frame."
					 */
					if(BstdFileEofP(BstdFile))
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
		
				/* Decode the next MPEG frame. The streams is read from the
				 * buffer, its constituents are break down and stored the the
				 * Frame structure, ready for examination/alteration or PCM
				 * synthesis. Decoding options are carried in the Frame
				 * structure from the Stream structure.
				 *
				 * Error handling: mad_frame_decode() returns a non zero value
				 * when an error occurs. The error condition can be checked in
				 * the error member of the Stream structure. A mad error is
				 * recoverable or fatal, the error status is checked with the
				 * MAD_RECOVERABLE macro.
				 *
				 * {4} When a fatal error is encountered all decoding
				 * activities shall be stopped, except when a MAD_ERROR_BUFLEN
				 * is signaled. This condition means that the
				 * mad_frame_decode() function needs more input to complete
				 * its work. One shall refill the buffer and repeat the
				 * mad_frame_decode() call. Some bytes may be left unused at
				 * the end of the buffer if those bytes forms an incomplete
				 * frame. Before refilling, the remaining bytes must be moved
				 * to the beginning of the buffer and used for input for the
				 * next mad_frame_decode() invocation. (See the comments
				 * marked {2} earlier for more details.)
				 *
				 * Recoverable errors are caused by malformed bit-streams, in
				 * this case one can call again mad_frame_decode() in order to
				 * skip the faulty part and re-sync to the next frame.
				 */
				if(mad_frame_decode(&Frame,&Stream))
				{
					if(MAD_RECOVERABLE(Stream.error))
					{
						/* Do not print a message if the error is a loss of
						 * synchronization and this loss is due to the end of
						 * stream guard bytes. (See the comments marked {3}
						 * supra for more informations about guard bytes.)
						 */
						if(Stream.error!=MAD_ERROR_LOSTSYNC ||
						   Stream.this_frame!=GuardPtr)
						{
							printf("%s: recoverable frame level error. \n",
									ProgName);
							//fflush(stderr);
						}
						continue;
					}
					else
						if(Stream.error==MAD_ERROR_BUFLEN)
							continue;
						else
						{
							printf("%s: unrecoverable frame level error.\n",
									ProgName);
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
					if(PrintFrameInfo(&Frame.header))
					{
						Status=1;
						break;
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
		
				/* Between the frame decoding and samples synthesis we can
				 * perform some operations on the audio data. We do this only
				 * if some processing was required. Detailed explanations are
				 * given in the ApplyFilter() function.
				 */
				//if(DoFilter)
				//	ApplyFilter(&Frame);
		
				/* Once decoded the frame is synthesized to PCM samples. No errors
				 * are reported by mad_synth_frame();
				 */
				mad_synth_frame(&Synth,&Frame);
		
				/* Synthesized samples must be converted from libmad's fixed
				 * point number to the consumer format. Here we use unsigned
				 * 16 bit big endian integers on two channels. Integer samples
				 * are temporarily stored in a buffer that is flushed when
				 * full.
				 */
				for(i=0;i<Synth.pcm.length;i++)
				{
					signed short	Sample;
		
					/* little endian */
					/* Left channel */
					Sample = scale(Synth.pcm.samples[0][i]); 
					//Sample=MadFixedToSshort(Synth.pcm.samples[0][i]);
					*(OutputPtr++)=((Sample >> 0) & 0xff);
					*(OutputPtr++)=((Sample >> 8) & 0xff);
		
					/* Right channel. If the decoded stream is monophonic then
					 * the right output channel is the same as the left one.
					 */
					if(MAD_NCHANNELS(&Frame.header)==2)
					{
					//	Sample=MadFixedToSshort(Synth.pcm.samples[1][i]);
						Sample = scale(Synth.pcm.samples[1][i]); 
					}
					*(OutputPtr++)=((Sample >> 0) & 0xff);
					*(OutputPtr++)=((Sample >> 8) & 0xff);
					
					/* Flush the output buffer if it is full. */
					if(OutputPtr==OutputBufferEnd)
					{
						//mybuffer = (audiobuffer*)malloc(sizeof(audiobuffer));
						//memcpy(mybuffer->buffer, pOutputBuffer, OUTPUT_BUFFER_SIZE);
						//bufferlist.push_back(mybuffer);
						memcpy(buffer+position, pOutputBuffer, OUTPUT_BUFFER_SIZE);
						position += OUTPUT_BUFFER_SIZE;
						switch (iBufferReady)
						{
							case 0: //buffering
								if (position < BUFFER_SIZE)
								{
									printf(".");
								}
								else
								{
									printf("\nPlaying Now...\n");
									iBufferReady = 1;
									//EnableAudio(); // start sound
									char *pBuffer = (char*)memalign(64, OUTPUT_BUFFER_SIZE);

									//audiobuffer *mybuf;
									//position = 0;
									
									for (int i = 0 ; i < BUFFER_SIZE ; i+=PSP_NUM_AUDIO_SAMPLES*4)
									{
										//mybuf = bufferlist.front();
										//memcpy(pBuffer, buffer+position, OUTPUT_BUFFER_SIZE);
										//free(mybuf), mybuf = NULL;
										//bufferlist.pop_front();
										sceAudioOutputPannedBlocking( OutputChannel, PSP_AUDIO_VOLUME_MAX, PSP_AUDIO_VOLUME_MAX, buffer+i); 
										//position+=OUTPUT_BUFFER_SIZE;
									}
									printf("Done\n");
									free(pBuffer);
									sceKernelExitThread(0);

								}
								break;
							case 1: //playing
								//sceDisplayWaitVblankStart();
								sceKernelDelayThread(100); //don't run as intensively
								break;
						}
							
						OutputPtr=pOutputBuffer;
					}
				}
			}
			
			/* The input file was completely read; the memory allocated by our
			 * reading module must be reclaimed.
			 */
			BstdFileDestroy(BstdFile);
		
			/* Mad is no longer used, the structures that were initialized must
		     * now be cleared.
			 */
			mad_synth_finish(&Synth);
			mad_frame_finish(&Frame);
			mad_stream_finish(&Stream); 
 
		}
		else
		{
			printf("Error opening file.\n");
		}
		
		return 0;
	}
	
	static inline
	signed int scale(mad_fixed_t &sample)
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
	static int PrintFrameInfo(struct mad_header *Header)
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
	
		printf("%s: %lu kb/s audio MPEG layer %s stream %s CRC, "
				"%s with %s emphasis at %d Hz sample rate\n",
				ProgName,Header->bitrate,Layer,
				Header->flags&MAD_FLAG_PROTECTION?"with":"without",
				Mode,Emphasis,Header->samplerate);
		return(0);
	}
	
	/****************************************************************************
	 * Converts a sample from libmad's fixed point number format to a signed	*
	 * short (16 bits).															*
	 ****************************************************************************/
	static signed short MadFixedToSshort(mad_fixed_t Fixed)
	{
		/* A fixed point number is formed of the following bit pattern:
		 *
		 * SWWWFFFFFFFFFFFFFFFFFFFFFFFFFFFF
		 * MSB                          LSB
		 * S ==> Sign (0 is positive, 1 is negative)
		 * W ==> Whole part bits
		 * F ==> Fractional part bits
		 *
		 * This pattern contains MAD_F_FRACBITS fractional bits, one
		 * should alway use this macro when working on the bits of a fixed
		 * point number. It is not guaranteed to be constant over the
		 * different platforms supported by libmad.
		 *
		 * The signed short value is formed, after clipping, by the least
		 * significant whole part bit, followed by the 15 most significant
		 * fractional part bits. Warning: this is a quick and dirty way to
		 * compute the 16-bit number, madplay includes much better
		 * algorithms.
		 */
	
		/* Clipping */
		if(Fixed>=MAD_F_ONE)
			return(SHRT_MAX);
		if(Fixed<=-MAD_F_ONE)
			return(-SHRT_MAX);
	
		/* Conversion. */
		Fixed=Fixed>>(MAD_F_FRACBITS-15);
		return((signed short)Fixed);
	} 
 
	void OnButtonPressed(int iButtonMask)
	{
		if (iButtonMask & PSP_CTRL_CROSS)
		{
			printf ("CROSS\n");
		}
		else if (iButtonMask & PSP_CTRL_SQUARE)
		{
			printf ("SQUARE\n");
		}
		else if (iButtonMask & PSP_CTRL_TRIANGLE)
		{
			printf ("TRIANGLE\n");
		}
		else if (iButtonMask & PSP_CTRL_CIRCLE)
		{
			printf ("CIRCLE\n");
		}
	};

//	void OnVBlank()
//	{

//	}
	
	
	
	/* This function gets called by pspaudiolib every time the
    audio buffer needs to be filled. The sample format is
    16-bit, stereo. */
	void OnAudioBufferEmpty(void* outbuf, unsigned int num_samples) 
	{
		//int ibz = bufferlist.size();
		//if (ibz > 0)
		//{
			//char buffer[OUTPUT_BUFFER_SIZE];
			//char *buffer = *bufferlist.pop_front();
			audiobuffer *mybuf = bufferlist.front();
			if (mybuf)
			{
				memcpy(outbuf, mybuf->buffer, OUTPUT_BUFFER_SIZE);
				free(mybuf), mybuf = NULL;
				bufferlist.pop_front();
				//printf("%d ", bufferlist.size());
			}
			else
			{
				// buffer underrun
				//printf("-");
				//m_thDecodeFile->WakeUp();
				/** Buffer underrun! */
				memset(outbuf, 0, OUTPUT_BUFFER_SIZE);
				//m_thDecodeFile->WakeUp();
				printf("!");
			}
	}
};
	
/** main */
int main(void) 
{
	myPSPApp *PSPApp  = new myPSPApp();
	PSPApp->Setup();
	PSPApp->Run();
	
	return 0;
}

