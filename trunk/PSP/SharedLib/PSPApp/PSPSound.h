#ifndef PSPSOUND
#define __PSPSOUND__
	#define INPUT_BUFFER_SIZE		(5*8192)
	#define PSP_NUM_AUDIO_SAMPLES 	PSP_AUDIO_SAMPLE_ALIGN(8192)
	#define PSP_AUDIO_BUFFER_SIZE 	PSP_NUM_AUDIO_SAMPLES*2*16
	#define OUTPUT_BUFFER_SIZE 		PSP_NUM_AUDIO_SAMPLES*4
	#define NUM_BUFFERS 			100
	
	#include <list>
	#include <mad.h>
	#include "bstdfile.h"

	/** From httpget.c */
	
	class CPSPSoundStream
	{
	public:
		CPSPSoundStream();
		~CPSPSoundStream();
		int OpenFile(char *filename);
		int OpenURL(char *strURL);
		size_t Read(void *pBuffer, size_t ElementSize, size_t ElementCount);
		BOOLEAN IsOpen();
		BOOLEAN IsEOF();
	private:
		enum stream_types
		{
			STREAM_TYPE_CLOSED,
			STREAM_TYPE_FILE,
			STREAM_TYPE_URL
		};
		enum stream_types m_Type;
		FILE *m_pfd;
		bstdfile_t *m_BstdFile;
		int   m_fd;
	};
	
	
	/** Internal use */
	//extern CPSPSound *pPSPSound = NULL;
	class CPSPSoundBuffer
	{
	public:
		CPSPSoundBuffer();
		void  Push(char *buf);
		char *Pop();
		int   GetPushPos();
		void  Empty();
		void  Done();
		int   IsDone();
	
	private:
		#if 0
		struct audiobuffer
		{ 
			char buffer[OUTPUT_BUFFER_SIZE]; 
		};
		std::list<audiobuffer*> m_PCMBufferList;
		#endif
		int pushpos,poppos,m_lastpushpos;
		char *ringbuf;
	};
	
	class CPSPSound
	{
	protected:
		enum pspsound_state
		{
			PLAY,
			PAUSE,
			STOP,
		};
		
		CPSPSoundBuffer Buffer;
		
	private:
		void Initialize();
		
		
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
