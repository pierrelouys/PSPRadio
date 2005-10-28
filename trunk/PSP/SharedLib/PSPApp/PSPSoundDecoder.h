#ifndef __PSPSOUND_DECODER__
	#define __PSPSOUND_DECODER__
	
	//#include <unistd.h>
	//#include <mad.h>
	//#include "bstdfile.h"
	#include "PSPStream.h"
	
	class CPSPSoundBuffer; /** Declared in PSPSound.h */
	
	class IPSPSoundDecoder
	{
	public:
		IPSPSoundDecoder(CPSPSoundBuffer *OutputBuffer, CPSPStream *InputStream)
			{	m_Buffer = OutputBuffer; m_InputStream = InputStream; }
			
		virtual ~IPSPSoundDecoder(){}
	
		virtual void Initialize(CPSPStream *InputStream){};
		
		virtual bool Decode(){return true;} /** Returns true on end-of-stream or unrecoverable error */

	protected:
		CPSPStream *m_InputStream;
		CPSPSoundBuffer *m_Buffer;
	};
	
#endif
