#ifndef __SCREEN_HANDLER__
	#define __SCREEN_HANDLER__
	
	#include "IPSPRadio_UI.h"
	#include <iniparser.h>
	
	class CScreenHandler
	{
	public:
		enum Screen
		{
			PSPRADIO_SCREEN_PLAYLIST,
			PSPRADIO_SCREEN_OPTIONS
		};
		
		CScreenHandler(IPSPRadio_UI *UI, CIniParser *Config, CPSPSound *Sound);
		
		void SetUp(IPSPRadio_UI *UI, CIniParser *Config, CPSPSound *Sound, 
					CPlayList *CurrentPlayList, CDirList  *CurrentPlayListDir, CPlayList::songmetadata *CurrentMetaData);

		int  Setup_Network();
		void DisplayCurrentNetworkSelection();
		void PlayListScreenInputHandler(int iButtonMask);
		void OnPlayStateChange(CPSPSound::pspsound_state NewPlayState)		;
		
		Screen GetCurrentScreen(){return m_CurrentScreen;}
		int GetCurrentNetworkProfile() { return m_iNetworkProfile; }
		
	private:
		Screen m_CurrentScreen;
		IPSPRadio_UI *m_UI;
		CIniParser *m_Config;
		CPSPSound *m_Sound;
		int m_iNetworkProfile;
		bool m_NetworkStarted;
		CPlayList *m_CurrentPlayList;
		CDirList  *m_CurrentPlayListDir;
		CPlayList::songmetadata *m_CurrentMetaData;

	};
#endif
