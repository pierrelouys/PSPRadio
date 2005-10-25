#ifndef __PSPSOUND_DECODER__
	#define __PSPSOUND_DECODER__
	
	#include <unistd.h>
	#include <mad.h>
	#include "bstdfile.h"

	/** Fixed */
	#define MAX_METADATA_SIZE		4080
	
	
	class CPSPSoundStream
	{
	public:
		struct MetaData
		{
			char strURI[MAXPATHLEN]; /** Filename/URL -- 'user' populated */
			/** Decoder/playlist/xml populated */
			char strTitle[300];
			char strURL[MAXPATHLEN];
			char strArtist[300];
			char strGenre[128];
			int iLength;
			int iSampleRate;
			int iBitRate;
			int iNumberOfChannels;
			int iMPEGLayer;
			int iItemIndex; /** JPF added to be used as a unique id for each item in list */
		} *m_CurrentMetaData;
		/** MetaData Accessors */
		void SetURI(char *NewURI);
		char *GetURI(){ return m_CurrentMetaData->strURI; }
		
		void ClearMetadata();
		void SetTitle(char *Title) { strcpy(m_CurrentMetaData->strTitle, Title); }
		char *GetTitle(){ return m_CurrentMetaData->strTitle; }
		void SetURL(char *URL) { strcpy(m_CurrentMetaData->strURL, URL); }
		char *GetURL(){ return m_CurrentMetaData->strURL; }
		void SetArtist(char *Artist) { strcpy(m_CurrentMetaData->strArtist, Artist); }
		char *GetArtist(){ return m_CurrentMetaData->strArtist; }
		
		void SetLength(int Length){ m_CurrentMetaData->iLength = Length; }
		int  GetLength(){ return m_CurrentMetaData->iLength; }
		void SetSampleRate(int SampleRate);
		int  GetSampleRate(){ return m_CurrentMetaData->iSampleRate; }
		void SetBitRate(int BitRate){ m_CurrentMetaData->iBitRate = BitRate; }
		int  GetBitRate(){ return m_CurrentMetaData->iBitRate; }
		void SetNumberOfChannels(int NumberOfChannels){ m_CurrentMetaData->iNumberOfChannels = NumberOfChannels; }
		int  GetNumberOfChannels(){ return m_CurrentMetaData->iNumberOfChannels; }
		void SetMPEGLayer(int MPEGLayer){ m_CurrentMetaData->iMPEGLayer = MPEGLayer; }
		int  GetMPEGLayer(){ return m_CurrentMetaData->iMPEGLayer; }
		
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
			STREAM_CONTENT_AUDIO_AAC,
			STREAM_CONTENT_PLAYLIST,
			STREAM_CONTENT_TEXT,
		};
		
		CPSPSoundStream();
		~CPSPSoundStream();
		
		int Open();
		void Close();
		bool IsOpen();
		void SetContentType(content_types Type) { m_ContentType = Type; }
		void SetState(stream_states State) { m_State = State; }
		
		int GetMetaDataInterval() { return m_iMetaDataInterval; }
		content_types GetContentType() { return m_ContentType; }
		FILE *GetFileDescriptor() { return m_pfd; }
		int GetSocketDescriptor() { return m_fdSocket; }
		stream_types GetType() { return m_Type; }
		stream_states GetState() { return m_State; }
	private:
		int http_open(char *url);
		
		size_t m_iMetaDataInterval;
		enum content_types m_ContentType;
		FILE *m_pfd;
		int   m_fdSocket; /** Socket */		
		enum stream_types  m_Type;
		enum stream_states m_State;
	};
	
	extern CPSPSoundStream *CurrentSoundStream;
	
	class CPSPSoundStreamReader
	{
	public:
		
		CPSPSoundStreamReader();
		virtual ~CPSPSoundStreamReader();
		
		virtual void Close();
		virtual size_t Read(unsigned char *pBuffer, size_t SizeInBytes);
		virtual bool IsEOF();
	
	protected:
		int SocketRead(char *pBuffer, size_t LengthInBytes);
		char *GetMetadataValue(char *strMetadata, char *strTag);
		
		bstdfile_t *m_BstdFile; /** For MAD */
		bool m_eof;
		size_t m_iRunningCountModMetadataInterval;
		char bMetaData[MAX_METADATA_SIZE];
		char bPrevMetaData[MAX_METADATA_SIZE];
		
		/** Copied from CurrentSoundStream con contstruction: */
		size_t m_iMetaDataInterval;
		FILE *m_pfd;
		int   m_fdSocket; /** Socket */		
	};

	class CPSPSoundBuffer; /** Declared in PSPSound.h */
	
	class IPSPSoundDecoder
	{
	public:
		IPSPSoundDecoder(CPSPSoundBuffer *OutputBuffer)
			{	m_Buffer = OutputBuffer; }
			
		virtual ~IPSPSoundDecoder()
		{
			if (m_InputStreamReader)
			{
				Log(LOG_VERYLOW, "~IPSPSoundDecoder(): Destroying input stream object. ");
				delete(m_InputStreamReader); m_InputStreamReader = NULL;
			}
		}
	
		virtual void Initialize()
		{			
			m_InputStreamReader = new CPSPSoundStreamReader();

			if (!m_InputStreamReader)
			{
				Log(LOG_ERROR, "IPSPSoundDecoder::Memory allocation error instantiating m_InputStream");
				ReportError("IPSPSoundDecoder::Decoder Initialization Error");
			}
		}
		
		virtual bool Decode(){return true;} /** Returns true on end-of-stream or unrecoverable error */

	protected:
		CPSPSoundStreamReader *m_InputStreamReader;
		CPSPSoundBuffer *m_Buffer;
	};
	
#endif
