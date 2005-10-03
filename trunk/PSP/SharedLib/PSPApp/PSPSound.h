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
	//#define INPUT_BUFFER_SIZE		(5*8192) //original
	#define INPUT_BUFFER_SIZE		(16302)
	//#define PSP_NUM_AUDIO_SAMPLES 	PSP_AUDIO_SAMPLE_ALIGN(8192) //original
	#define PSP_NUM_AUDIO_SAMPLES 	PSP_AUDIO_SAMPLE_ALIGN(4096)
	#define PSP_AUDIO_BUFFER_SIZE 	PSP_NUM_AUDIO_SAMPLES*2*16
	#define OUTPUT_BUFFER_SIZE 		PSP_NUM_AUDIO_SAMPLES*4
	#define NUM_BUFFERS 			100
	#define PSP_SAMPLERATE			44100
	#define MAX_METADATA_SIZE		4080
	
	#include <list>
	#include <mad.h>
	#include "bstdfile.h"

	


	/** From httpget.c */
	/* ------ Declarations from "httpget.c" (From mpg123) ------ */
	//extern char *proxyurl;
	//extern unsigned long proxyip;
	extern int http_open (char *url, size_t &iMetadataInterval);
	int SocketRead(char *pBuffer, size_t LengthInBytes, int sock);
	//extern char *httpauth; 
	
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
	//extern CPSPSound *pPSPSound = NULL;
	class CPSPSoundBuffer
	{
	public:
		CPSPSoundBuffer();
		~CPSPSoundBuffer();
		void  Push(char *buf);
		char *Pop();
		int   GetPushPos();
		int   GetPopPos(){return poppos;};
		void  Empty();
		void  Done();
		int   IsDone();
		
		void  SetSampleRate(size_t samplerate);
	
	private:
		#if 0
		struct audiobuffer
		{ 
			char buffer[OUTPUT_BUFFER_SIZE]; 
		};
		std::list<audiobuffer*> m_PCMBufferList;
		#endif
		int pushpos,poppos,m_lastpushpos;
		char *ringbuf, *upsampled_buffer;
		bool buffering;
		size_t m_samplerate, m_dUpsampledbuffersize;
		
		size_t UpSample(short *bOut, short *bIn);

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

		int GetBufferPopPos()  { return Buffer.GetPopPos(); };
		int GetBufferPushPos() { return Buffer.GetPushPos(); };
		/** Threads */
		static int ThPlayAudio(SceSize args, void *argp);
		static int ThDecode(SceSize args, void *argp);
	};
	
	extern CPSPSound *pPSPSound;
	
#endif
