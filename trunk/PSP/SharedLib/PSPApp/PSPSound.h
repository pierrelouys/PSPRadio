#ifndef PSPSOUND
#define __PSPSOUND__
	#define INPUT_BUFFER_SIZE		(5*8192)
	#define PSP_NUM_AUDIO_SAMPLES 	PSP_AUDIO_SAMPLE_ALIGN(8192)
	#define PSP_AUDIO_BUFFER_SIZE 	PSP_NUM_AUDIO_SAMPLES*2*16
	#define OUTPUT_BUFFER_SIZE 		PSP_NUM_AUDIO_SAMPLES*4
	#define NUM_BUFFERS 			10
	
	#include <list>
	#include <mad.h>
	
	/** Internal use */
	//extern CPSPSound *pPSPSound = NULL;
	
	class CPSPSound
	{
	protected:
		struct audiobuffer
		{ 
			char buffer[OUTPUT_BUFFER_SIZE]; 
		};
		enum pspsound_state
		{
			PLAY,
			PAUSE,
			STOP,
		};
		std::list<audiobuffer*> *GetPCMBufferList(){ return &m_PCMBufferList; };
		
	private:
		void Initialize();
		
		std::list<audiobuffer*> m_PCMBufferList;
		
		CPSPThread *m_thDecode,*m_thPlayAudio;
		int m_audiohandle ;
		
		/** Accessors */
		int GetAudioHandle();
		
		pspsound_state m_CurrentState;
	
	protected:
		virtual void Decode();
	
	public:
		CPSPSound();
		virtual ~CPSPSound();
		
		int Play();
		int Pause();
		int Stop();
	
		/** Threads */
		static int ThPlayAudio(SceSize args, void *argp);
		static int ThDecode(SceSize args, void *argp);
	};
	
#endif
