#ifndef __PSPSTREAM__
	#define __PSPSTREAM__
	
	#include <unistd.h>
	#include <mad.h>
	#include "bstdfile.h"

	/** Fixed */
	#define MAX_METADATA_SIZE		4080
	
	struct MetaData
	{
		enum content_types
		{
			CONTENT_NOT_DEFINED,
			CONTENT_AUDIO_MPEG,
			CONTENT_AUDIO_OGG,
			CONTENT_AUDIO_AAC,
			CONTENT_PLAYLIST,
			CONTENT_TEXT,
		};
		char strURI[MAXPATHLEN];
		char strTitle[300];
		char strURL[MAXPATHLEN];
		char strArtist[300];
		char strGenre[128];
		int iLength;
		int iSampleRate;
		int iBitRate;
		int iNumberOfChannels;
		int iMPEGLayer;
		content_types ContentType;
		int iItemIndex; /** JPF added to be used as a unique id for each item in list */
	};
			
	class CPSPStream
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
		
		CPSPStream();
		~CPSPStream();
		
		int Open();
		void Close();
		bool IsOpen();

		bool DownloadToFile(char *strFilename, size_t &bytesDownloaded);
		
		int GetMetaDataInterval() { return m_iMetaDataInterval; }
		FILE *GetFileDescriptor() { return m_pfd; }
		int GetSocketDescriptor() { return m_fdSocket; }
		
		stream_types GetType() { return m_Type; }
		stream_states GetState() { return m_State; }
		
		/** MetaData Accessors/Mutators */
		MetaData *GetMetaData(){ return m_MetaData; }
		void ClearMetadata();
		void SetURI(char *NewURI);
		char *GetURI(){ return m_MetaData->strURI; }
		void SetTitle(char *Title) { strcpy(m_MetaData->strTitle, Title); }
		char *GetTitle(){ return m_MetaData->strTitle; }
		void SetURL(char *URL) { strcpy(m_MetaData->strURL, URL); }
		char *GetURL(){ return m_MetaData->strURL; }
		void SetArtist(char *Artist) { strcpy(m_MetaData->strArtist, Artist); }
		char *GetArtist(){ return m_MetaData->strArtist; }
		void SetLength(int Length){ m_MetaData->iLength = Length; }
		int  GetLength(){ return m_MetaData->iLength; }
		void SetSampleRate(int SampleRate);
		int  GetSampleRate(){ return m_MetaData->iSampleRate; }
		void SetBitRate(int BitRate){ m_MetaData->iBitRate = BitRate; }
		int  GetBitRate(){ return m_MetaData->iBitRate; }
		void SetNumberOfChannels(int NumberOfChannels){ m_MetaData->iNumberOfChannels = NumberOfChannels; }
		int  GetNumberOfChannels(){ return m_MetaData->iNumberOfChannels; }
		void SetMPEGLayer(int MPEGLayer){ m_MetaData->iMPEGLayer = MPEGLayer; }
		int  GetMPEGLayer(){ return m_MetaData->iMPEGLayer; }
		void SetContentType(MetaData::content_types Type) { m_MetaData->ContentType = Type; }
		MetaData::content_types GetContentType() { return m_MetaData->ContentType; }
		
		/** Move to protected when integrated with sound stream reader */
		void SetState(stream_states State) { m_State = State; }
	protected:
		int http_open(char *url);
		
		MetaData *m_MetaData;
		size_t m_iMetaDataInterval;
		FILE *m_pfd;
		int   m_fdSocket; /** Socket */		
		stream_types  m_Type;
		stream_states m_State;
	};
	
	class CPSPStreamReader
	{
	public:
		
		CPSPStreamReader(CPSPStream *CurrentSoundStream);
		virtual ~CPSPStreamReader();
		
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
		CPSPStream *m_CurrentStream;
		size_t m_iMetaDataInterval;
		FILE *m_pfd;
		int   m_fdSocket; /** Socket */		
	};

#endif
