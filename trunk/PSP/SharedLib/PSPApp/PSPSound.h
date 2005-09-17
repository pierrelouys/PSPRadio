#ifndef PSPSOUND
#define __PSPSOUND__
	#define INPUT_BUFFER_SIZE		(5*8192)
	#define PSP_NUM_AUDIO_SAMPLES 	PSP_AUDIO_SAMPLE_ALIGN(8192)
	#define PSP_AUDIO_BUFFER_SIZE 	PSP_NUM_AUDIO_SAMPLES*2*16
	#define OUTPUT_BUFFER_SIZE 		PSP_NUM_AUDIO_SAMPLES*4
	#define NUM_BUFFERS 			10
	
	#include <list>
	#include <mad.h>
	
	
	class CPSPSound
	{
	private:
	
		struct audiobuffer
		{ 
			char buffer[OUTPUT_BUFFER_SIZE]; 
		};
		std::list<audiobuffer*> m_PCMBufferList;
		
		CPSPThread *m_thDecodeFile,*m_thPlayAudio;
		int m_audiohandle ;
		
		/** Accessors */
		int GetAudioHandle();
		std::list<audiobuffer*> *GetPCMBufferList(){ return &m_PCMBufferList; };
;
		char *GetFile();
		enum pspsound_state
		{
			PLAY,
			PAUSE,
			STOP,
		};
		
		pspsound_state m_CurrentState;
	
	public:
		CPSPSound();
		
		int Play();
		int Pause();
		int Stop();
	
		/** Threads */
		static int ThPlayAudio(SceSize args, void *argp);
		static int ThDecodeFile(SceSize args, void *argp);
		static signed int scale(mad_fixed_t &sample);
		static int PrintFrameInfo(struct mad_header *Header);
		static signed short MadFixedToSshort(mad_fixed_t Fixed);
	};
	
#endif
