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
#ifndef PSPSOUND
#define __PSPSOUND__
	/** Not configurable */
	#define PSP_SAMPLERATE			44100
	#define NUM_CHANNELS			2	/** L and R */
	#define BYTES_PER_SAMPLE		2	/** 16bit sound */
	/** Useful macros */
	#define FRAMES_TO_SAMPLES(f)	(f*NUM_CHANNELS)
	#define SAMPLES_TO_BYTES(s)		(s*BYTES_PER_SAMPLE)
	#define BYTES_TO_FRAMES(b)		(b/(NUM_CHANNELS*BYTES_PER_SAMPLE))
	#define BYTES_TO_SAMPLES(b)		(b/(BYTES_PER_SAMPLE))
	#define FRAMES_TO_BYTES(f)		(f*(NUM_CHANNELS*BYTES_PER_SAMPLE))
	typedef u16 Sample;
	typedef u32 Frame;
	
	/** Configurable */
	/* (frames are 2ch, 16bits, so 4096frames =16384bytes =8192samples-l-r-combined.)*/
	#define PSP_BUFFER_SIZE_IN_FRAMES	PSP_AUDIO_SAMPLE_ALIGN(4096)	
	#define NUM_BUFFERS 			50	/* 10 frames */
	
	#define INPUT_BUFFER_SIZE		16302
	
	/** Fixed */
	#define MAX_METADATA_SIZE		4080
	
	#include <list>
	#include <mad.h>
	#include "bstdfile.h"

	
	/* ------ Declarations from "httpget.c" (From mpg123) ------ */
	extern int http_open (char *url, size_t &iMetadataInterval);
	
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
		
		
	private:
		enum stream_types  m_Type;
		enum stream_states m_State;
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
		void  PushFrame(Frame frame); /* Takes 1 frame */
		Frame PopFrame(); 			/* Returns 1 frame */
		Frame *PopBuffer();			/* Returns PSP_BUFFER_SIZE_IN_FRAMES frames */
		int   GetPushPos(){return pushpos-ringbuf_start;};
		int   GetPopPos(){return poppos-ringbuf_start;};
		size_t GetBufferFillPercentage();
		void  Empty();
		void  Done();
		bool   IsDone();
		
		void  SetSampleRate(size_t samplerate);
	
	private:
		Frame *pushpos,*poppos,*m_lastpushpos, *ringbuf_end;
		Frame *ringbuf_start, *pspbuf;
		bool buffering;
		size_t m_samplerate;
		Frame *m_bUpsamplingTemp, *m_bUpsamplingOut;
		size_t UpSample(Frame *bOut, Frame *bIn, int mult, int div);
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
		CPSPSoundBuffer Buffer;
		CPSPSoundStream *m_InputStream;
		
	private:
		void Initialize();
		
		
		CPSPThread *m_thDecode,*m_thPlayAudio;
		int m_audiohandle ;
		
		/** Accessors */
		int GetAudioHandle();
		
		pspsound_state m_CurrentState;
	
	protected:
		virtual void Decode();
		void SuspendDecodingThread(){m_thDecode->Suspend();};
		
	public:
		CPSPSound();
		virtual ~CPSPSound();
		
		/** Accessors */
		//void SetFile(char *strName);
		//char *GetFile() { return m_strFile; };
		//do through Stream
		CPSPSoundStream *GetStream() { return m_InputStream; }
		
		int Play();
		int Pause();
		int Stop();
		pspsound_state GetPlayState() { return m_CurrentState; };
		
		int SendMessage(int iMessageId, void *pMessage = NULL, int iSenderId = SID_PSPSOUND)
			{ return pPSPApp->OnMessage(iMessageId, pMessage, iSenderId); };

		//int GetBufferPopPos()  { return Buffer.GetPopPos(); };
		//int GetBufferPushPos() { return Buffer.GetPushPos(); };
		size_t GetBufferFillPercentage() { return Buffer.GetBufferFillPercentage(); };
		/** Threads */
		static int ThPlayAudio(SceSize args, void *argp);
		static int ThDecode(SceSize args, void *argp);
	};
	
	extern CPSPSound *pPSPSound;
	
#endif
