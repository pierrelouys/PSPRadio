/* 
	PSPApp Example 2
	Using MAD
*/

#include <list>
#include <PSPApp.h>

#include <stdio.h>
#include <unistd.h> 

#include <stdlib.h>
#include <string.h>
//#include <math.h>
#include <limits.h>
#include <mad.h>
#include "bstdfile.h"
#include <malloc.h>

using namespace std;

char *ProgName = "MADEXAMPLE";

/* Define the module info section */
PSP_MODULE_INFO("MADEXAMPLE", 0, 1, 1);
/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

int errno = 0;

#define INPUT_BUFFER_SIZE	(5*8192)
#define PSP_NUM_AUDIO_SAMPLES PSP_AUDIO_SAMPLE_ALIGN(8192)
#define PSP_AUDIO_BUFFER_SIZE PSP_NUM_AUDIO_SAMPLES*2*16
#define OUTPUT_BUFFER_SIZE PSP_NUM_AUDIO_SAMPLES*4
#define NUM_BUFFERS 10

class myPSPApp : public CPSPApp
{
public:

	struct audiobuffer
	{ 
	public:
		char buffer[OUTPUT_BUFFER_SIZE]; 
	};
	list<audiobuffer*> m_bufferlist;

	
	CPSPThread *m_thDecodeFile,*m_thPlayAudio;
	int m_audiohandle ;

	int GetAudioHandle()
	{
		return m_audiohandle;
	}
	
	list<audiobuffer*> *GetBufferList()
	{
		return &m_bufferlist;
	}
	
	int Setup()
	{
		printf("PSPApp MAD Example...\n");
		
		//bufferlist.empty();
		m_audiohandle = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, PSP_NUM_AUDIO_SAMPLES, PSP_AUDIO_FORMAT_STEREO);
		if ( m_audiohandle < 0 )
		{
			printf("Error getting a sound channel!\n");
			return 0;
		}
		else
		{
			printf("Retrieved handle=%d\n", m_audiohandle);
		}
	
		m_thDecodeFile = new CPSPThread("filedecode_thread", ThDecodeFile, 0x11, 80000);
		m_thPlayAudio = new CPSPThread("playaudio_thread", ThPlayAudio, 0x11, 80000);
		m_thDecodeFile->Start();
		sceKernelDelayThread(500000); /** 500ms */
		m_thPlayAudio->Start();
	
		return 0;
	}
	
	char *GetFile() { return "ms0:/sparkless.mp3"; }
	
	static int ThDecodeFile(SceSize args, void *argp)
	{
		struct mad_frame	Frame;
		struct mad_stream	Stream;
		struct mad_synth	Synth;
		mad_timer_t			Timer;
		const unsigned char	*OutputBufferEnd;
		int					Status=0,
							i;
		unsigned long		FrameCount=0;
		bstdfile_t			*BstdFile = NULL;
		//char *pPlayBuffer,
		unsigned char		*pInputBuffer/*[INPUT_BUFFER_SIZE+MAD_BUFFER_GUARD]*/,
							*pOutputBuffer/*[OUTPUT_BUFFER_SIZE]*/;
		unsigned char		*OutputPtr;
		unsigned char		*GuardPtr;
		
		myPSPApp *This = (myPSPApp*)pPSPApp;
		list<audiobuffer*> *bufferlist = This->GetBufferList();
	
		printf ("Starting Decoding Thread\n");
		
		pInputBuffer = (unsigned char*)malloc(INPUT_BUFFER_SIZE+MAD_BUFFER_GUARD);
		pOutputBuffer = (unsigned char*)malloc(OUTPUT_BUFFER_SIZE);

		if (!(pInputBuffer && pOutputBuffer))
		{
			printf("Memory allocation error!\n");
			sceKernelExitThread(0);
			return 0;
		}
	
		OutputPtr=pOutputBuffer;
		GuardPtr = NULL;
		OutputBufferEnd = pOutputBuffer+OUTPUT_BUFFER_SIZE;
		
		/* First the structures used by libmad must be initialized. */
		mad_stream_init(&Stream);
		mad_frame_init(&Frame);
		mad_synth_init(&Synth);
		mad_timer_reset(&Timer);

		FILE *InputFp = fopen(This->GetFile(), "rb");
		if (InputFp != NULL)
		{
			printf("'%s' Opened Successfully\n", This->GetFile());
			BstdFile=NewBstdFile(InputFp);
			if(BstdFile==NULL)
			{
				printf("%s: can't create a new bstdfile_t (%s).\n",
						ProgName,strerror(errno));
				return(1);
			} 
			
			/** Main decoding loop */
			/* This is the decoding loop. */
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
		
				/* Decode the next MPEG frame. */
				if(mad_frame_decode(&Frame,&Stream))
				{
					if(MAD_RECOVERABLE(Stream.error))
					{
						if(Stream.error!=MAD_ERROR_LOSTSYNC ||
						   Stream.this_frame!=GuardPtr)
						{
							printf("%s: recoverable frame level error. \n",
									ProgName);
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
					
					/* Queue the output buffer if it is full. */
					if(OutputPtr==OutputBufferEnd)
					{
						//printf("+");
						audiobuffer *mybuffer = (audiobuffer*)(char*)memalign(64, sizeof(audiobuffer));
						memcpy(mybuffer->buffer, pOutputBuffer, OUTPUT_BUFFER_SIZE);
						/*This->*/bufferlist->push_back(mybuffer);
						//printf("+2");

						pspDebugScreenSetXY(0,10);
						printf("Buffers: %03d/%03d   ", bufferlist->size(), NUM_BUFFERS);
						if (/*This->*/bufferlist->size() >= NUM_BUFFERS)
						{
							pspDebugScreenSetXY(0,11);
							printf("+");							
							//static int audiostarted = 0;
							//if (audiostarted == 0)
							//{
							//	printf("Done Buffering.\n");
							//	This->m_thPlayAudio->Start(1, This);
							//	audiostarted = 1;
							//}
							sceKernelDelayThread(100000); /** 100ms */
						}
							
						OutputPtr=pOutputBuffer;
					}
					
				}
				sceKernelDelayThread(5000); /** 5ms */

			};
			printf("Done.\n");
			
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
			printf("Error opening '%s'.\n", This->GetFile());

		}
		sceKernelExitThread(0);
		
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
		pspDebugScreenSetXY(0,25);
		if (iButtonMask & PSP_CTRL_CROSS)
		{
			printf ("CROSS    ");
		}
		else if (iButtonMask & PSP_CTRL_SQUARE)
		{
			printf ("SQUARE   ");
		}
		else if (iButtonMask & PSP_CTRL_TRIANGLE)
		{
			printf ("TRIANGLE ");
		}
		else if (iButtonMask & PSP_CTRL_CIRCLE)
		{
			printf ("CIRCLE   ");
		}
	};

	static int ThPlayAudio(SceSize args, void *argp)
	{
			//static char outbuf[OUTPUT_BUFFER_SIZE];
			audiobuffer *mybuf = NULL;
			myPSPApp *This = (myPSPApp*)pPSPApp;
			int ah = This->GetAudioHandle();
			list<audiobuffer*> *bufferlist = This->GetBufferList();
			
			pspDebugScreenSetXY(0,15);
			printf ("Starting Play Thread (AudioHandle=%d)\n", ah);
			
			for(;;)
			{
				mybuf = bufferlist->front();
				if (mybuf)
				{
					//printf("Playing %03d (handle=%d)  ", bufferlist.size(), This->m_audiohandle);
					//memcpy(outbuf, mybuf->buffer, OUTPUT_BUFFER_SIZE);
					sceAudioOutputPannedBlocking(ah, PSP_AUDIO_VOLUME_MAX,PSP_AUDIO_VOLUME_MAX,mybuf->buffer);
					free(mybuf), mybuf = NULL;
					bufferlist->pop_front();
					pspDebugScreenSetXY(0,16);
				}
				else
				{
					/** Buffer underrun! */
					//printf("Buffer Underrun!\n");
					pspDebugScreenSetXY(0,17);
					printf("! %03d   ", bufferlist->size());
					sceKernelDelayThread(100000); /** 100ms */
				}
			}
			sceKernelExitThread(0);
			return 0;
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

