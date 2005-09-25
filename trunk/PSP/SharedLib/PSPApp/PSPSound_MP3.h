#ifndef PSPSOUND
#define __PSPSOUND__
	#include "PSPSound.h"
	#include <mad.h>
	
	//extern CPSPSound_MP3 *pPSPSound_MP3;
	
	class CPSPSound_MP3 : public CPSPSound
	{
	public:
		CPSPSound_MP3();
		
		/** Accessors */
		void SetFile(char *strName);
		char *GetFile() { return m_strFile; };
		
		int SendMessage(int iMessageId, void *pMessage = NULL, int iSenderId = SID_PSPSOUND_MP3)
			{ return pPSPApp->OnMessage(iMessageId, pMessage, iSenderId); };


	protected:
		virtual void Decode(); /** 'Thread' */

	private:
		char m_strFile[256];
		static signed int scale(mad_fixed_t &sample);
		static int PrintFrameInfo(struct mad_header *Header);
		static signed short MadFixedToSshort(mad_fixed_t Fixed);
	};
	
#endif
