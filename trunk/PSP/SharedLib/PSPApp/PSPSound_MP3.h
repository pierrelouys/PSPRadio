#ifndef PSPSOUND
#define __PSPSOUND__
	#include "PSPSound.h"
	#include <mad.h>
	
	class CPSPSound_MP3 : public CPSPSound
	{
	public:
		CPSPSound_MP3();
		
		/** Accessors */
		void SetFile(char *strName);
		char *GetFile() { return m_strFile; };

	protected:
		virtual void Decode(); /** 'Thread' */

	private:
		char m_strFile[256];
		static signed int scale(mad_fixed_t &sample);
		static int PrintFrameInfo(struct mad_header *Header);
		static signed short MadFixedToSshort(mad_fixed_t Fixed);
	};
	
#endif
