#ifndef __SCREEN_HANDLER__
	#define __SCREEN_HANDLER__
	
	#include <PSPSound.h>
	#include "IPSPRadio_UI.h"
	#include <iniparser.h>
	#include "PlayList.h"
	#include "DirList.h"
	#include <list>
	using namespace std;

	class IPSPRadio_UI;
	
	class CScreenHandler
	{
	public:
		enum Screen
		{
			PSPRADIO_SCREEN_PLAYLIST,
			PSPRADIO_SCREEN_OPTIONS
		};
		
		#define PSPRADIO_SCREEN_LIST_BEGIN  PSPRADIO_SCREEN_PLAYLIST
		#define PSPRADIO_SCREEN_LIST_END	(PSPRADIO_SCREEN_OPTIONS+1)
		
		/** Options screen */
		struct Options
		{
			int	 Id;
			char strName[60];
			char strStates[256];
			int  iSelectedState;
			int  iNumberOfStates;
		};
		list<Options> m_OptionsList;
		list<Options>::iterator m_CurrentOptionIterator;
		void OptionsScreenInputHandler(int iButtonMask);
		void OnOptionChange();
		/** Options screen */
		
		CScreenHandler(IPSPRadio_UI *UI, CIniParser *Config, CPSPSound *Sound);
		
		void SetUp(IPSPRadio_UI *UI, CIniParser *Config, CPSPSound *Sound, 
					CPlayList *CurrentPlayList, CDirList  *CurrentPlayListDir, CPlayList::songmetadata *CurrentMetaData);
		void StartScreen(Screen screen);

		int  Start_Network(int iNewProfile = -1);
		int  Stop_Network();
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
