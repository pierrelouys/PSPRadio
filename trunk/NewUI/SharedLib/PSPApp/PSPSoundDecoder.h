#ifndef __PSPSOUND_DECODER__
	#define __PSPSOUND_DECODER__
	
	#include "PSPStream.h"

	class CPSPSoundBuffer; /** Declared in PSPSound.h */
	
	class IPSPSoundDecoder
	{
	public:
		IPSPSoundDecoder(CPSPSoundBuffer *OutputBuffer, CPSPStream *InputStream)
			{	m_Buffer = OutputBuffer; m_InputStream = InputStream; }
			
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
			m_InputStreamReader = new CPSPStreamReader(m_InputStream);

			if (!m_InputStreamReader)
			{
				Log(LOG_ERROR, "IPSPSoundDecoder::Memory allocation error instantiating m_InputStream");
				ReportError("IPSPSoundDecoder::Decoder Initialization Error");
			}
		}
		
		virtual bool Decode(){return true;} /** Returns true on end-of-stream or unrecoverable error */

	protected:
		CPSPStream *m_InputStream;
		CPSPStreamReader *m_InputStreamReader;
		CPSPSoundBuffer *m_Buffer;
	};
	
#endif
