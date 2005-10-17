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
#ifndef __PSPSOUND__
#define __PSPSOUND__
	#include "PSPEventQ.h"

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
	typedef u16 Sample;
	typedef u32 Frame;
	struct PCMFrameInHalfSamples
	{
		u8 RHalfSampleA;
		u8 RHalfSampleB;
		u8 LHalfSampleA;
		u8 LHalfSampleB;
	};
	
	/** Configurable */
	/* (frames are 2ch, 16bits, so 4096frames =16384bytes =8192samples-l-r-combined.)*/
	//#define PSP_BUFFER_SIZE_IN_FRAMES	PSP_AUDIO_SAMPLE_ALIGN(4096)	
	#define PSP_BUFFER_SIZE_IN_FRAMES	PSP_AUDIO_SAMPLE_ALIGN(2048)	
	//Now configurable: #define NUM_BUFFERS 			20	
	#define DEFAULT_NUM_BUFFERS		20		/** Default */
	
	#define INPUT_BUFFER_SIZE		16302
	
	/** Fixed */
	#define MAX_METADATA_SIZE		4080
	
	#include <list>
	#include <mad.h>
	#include "bstdfile.h"

	
	/** Message IDs */
	/** Messages from PSPSound to decode thread */
	enum MessageIDsFromPSPSoundToDecodeThread
	{
		MID_DECODER_START,
		MID_DECODER_STOP,
		MID_DECODER_THREAD_EXIT_NEEDOK
	};
	
	enum MessageIDsFromPSPSoundToPlayThread
	{
		MID_PLAY_START,
		MID_PLAY_STOP,
		MID_PLAY_THREAD_EXIT_NEEDOK
	};
	
	/** Other functions */
	int SocketRead(char *pBuffer, size_t LengthInBytes, int sock);
	
	class CPSPSoundStream
	{
	public:
		enum stream_types
		{
			STREAM_TYPE_NONE,
			STREAM_TYPE_FILE,
			STREAM_TYPE_URL
		};
		enum stream_states
		{
			STREAM_STATE_CLOSED,
			STREAM_STATE_OPEN
		};
		enum content_types
		{
			STREAM_CONTENT_NOT_DEFINED,
			STREAM_CONTENT_AUDIO_MPEG,
			STREAM_CONTENT_AUDIO_OGG,
			STREAM_CONTENT_AUDIO_AAC
		};
		
		CPSPSoundStream();
		~CPSPSoundStream();
		
		void SetFile(char *strName);
		char *GetFile() { return m_strFile; };
		int Open();
		void Close();
		size_t Read(unsigned char *pBuffer, size_t ElementSize, size_t ElementCount);
		bool IsOpen();
		bool IsEOF();
		stream_types GetType() { return m_Type; }
		stream_states GetState() { return m_State; }
		
		content_types GetContentType() { return m_ContentType; }
		void SetContentType(content_types Type) { m_ContentType = Type; }
		
		
	private:
		enum stream_types  m_Type;
		enum stream_states m_State;
		enum content_types m_ContentType;
		char m_strFile[256];
		FILE *m_pfd;
		bstdfile_t *m_BstdFile;
		int   m_fd;
		bool m_sock_eof;
		size_t m_iMetaDataInterval;
		size_t m_iRunningCountModMetadataInterval;
		char bMetaData[MAX_METADATA_SIZE];
		char bPrevMetaData[MAX_METADATA_SIZE];
	};
	
	
	/** Internal use */
	class CPSPSoundBuffer
	{
	public:
		CPSPSoundBuffer();	
		~CPSPSoundBuffer();
		void  ChangeBufferSize(size_t buffer_size); /*Takes the number of PSP sound buffers 20~100. If not changed, defaults to DEFAULT_NUM_BUFFERS.*/
		void  PushFrame(Frame frame); /* Takes 1 frame for any samplerate, set samplate first. */
		void  Push44Frame(Frame frame); /* Takes 1 frame for a 44100Hz stream */
		Frame PopFrame(); 			/* Returns 1 frame */
		
		Frame *PopBuffer();			/* Returns PSP_BUFFER_SIZE_IN_FRAMES frames */
		size_t GetBufferFillPercentage();
		void  Empty();
		void  Done();
		bool  IsDone();
		
		void  SetSampleRate(size_t samplerate);
		
	private:
		Frame 	*ringbuf_start, *pspbuf; /** Buffers */
		Frame 	*pushpos,*poppos, *m_lastpushpos, *ringbuf_end; /** Buffer Pointers */
		bool  	m_buffering;
		size_t 	m_samplerate, m_mult, m_div;
		size_t 	m_FrameCount; /** Used for buffer percentage */
		size_t	m_NumBuffers; /** Configurable via config-file. Should be from 20 - 100 or so.. test! */
		
		void AllocateBuffers(); /** Called by constructor/ChangeBufferSize() to (re)allocate buffers */

	};
	
	class IPSPSoundDecoder
	{
	public:
		IPSPSoundDecoder(CPSPSoundStream *InputStream, CPSPSoundBuffer *OutputBuffer)
			{m_InputStream = InputStream; m_Buffer = OutputBuffer;}
		virtual ~IPSPSoundDecoder(){}
		
		virtual void Initialize(){}
		virtual bool Decode(){return true;} /** Returns true on end-of-stream or unrecoverable error */

	protected:
		CPSPSoundStream *m_InputStream;
		CPSPSoundBuffer *m_Buffer;
	};
	
	class CPSPSound
	{
	public:
		enum pspsound_state
		{
			PLAY,
			PAUSE,
			STOP,
		};
		
	protected:
		CPSPSoundBuffer   Buffer;
		CPSPSoundStream  *m_InputStream;
		
	private:
		void Initialize();
		/** Accessors */
		int GetAudioHandle();
		
		CPSPThread *m_thDecode,*m_thPlayAudio;
		int m_audiohandle ;
		pspsound_state m_CurrentState;
		
		CPSPEventQ *m_EventToDecTh, *m_EventToPlayTh;
		
	protected:
		void SuspendDecodingThread(){m_thDecode->Suspend();};
		
	public:
		CPSPSound();
		virtual ~CPSPSound();
		
		/** Accessors */
		CPSPSoundStream *GetStream() { return m_InputStream; }
		
		int Play();
		int Pause();
		int Stop();
		pspsound_state GetPlayState() { return m_CurrentState; };
		
		int SendEvent(int iEventId, void *pData = NULL, int iSenderId = SID_PSPSOUND)
		{ 
			CPSPEventQ::QEvent event = { iSenderId, iEventId, pData };
			return (pPSPApp && pPSPApp->m_EventToPSPApp)?pPSPApp->m_EventToPSPApp->Send(event):-1;
		};

		size_t GetBufferFillPercentage() { return Buffer.GetBufferFillPercentage(); };
		void   ChangeBufferSize(size_t size) { Buffer.ChangeBufferSize(size); };
		
		void SetDecodeThreadPriority(int iNewPrio);
		void SetPlayThreadPriority(int iNewPrio);
		
		int GetEventToDecThSize() { return m_EventToDecTh->Size(); }
		int GetEventToPlayThSize() { return m_EventToPlayTh->Size(); }
		
		/** Threads */
		static int ThPlayAudio(SceSize args, void *argp);
		static int ThDecode(SceSize args, void *argp);
	};
	
	extern CPSPSound *pPSPSound;
	
#endif
