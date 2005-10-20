#ifndef __SCREEN_HANDLER__
	#define __SCREEN_HANDLER__
	
	#include <PSPSound.h>
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
		
		enum request_on_play_stop
		{
			NOTHING,
			PLAY,
			STOP
		};
		
		#define PSPRADIO_SCREEN_LIST_BEGIN  PSPRADIO_SCREEN_PLAYLIST
		#define PSPRADIO_SCREEN_LIST_END	(PSPRADIO_SCREEN_OPTIONS+1)
		
		
		#define MAX_OPTION_LENGTH 60
		#define MAX_NUM_OF_OPTIONS 20
		
		/** Options screen */
		struct Options
		{
			int	 Id;
			char strName[MAX_OPTION_LENGTH];
			char *strStates[MAX_NUM_OF_OPTIONS];
			int  iActiveState;		/** indicates the currently active state -- user pressed X on this state */
			int  iSelectedState;	/** selection 'box' around option; not active until user presses X */
			int  iNumberOfStates;
		};
		list<Options> m_OptionsList;
		list<Options>::iterator m_CurrentOptionIterator;
		void OptionsScreenInputHandler(int iButtonMask);
		void OnOptionActivation();
		void PopulateOptionsData();
		/** Options screen */
		
		CScreenHandler(IPSPRadio_UI *UI, CIniParser *Config, CPSPSound *Sound);
		
		void SetUp(IPSPRadio_UI *UI, CIniParser *Config, CPSPSound *Sound, 
					CPlayList *CurrentPlayList, CDirList  *CurrentPlayListDir);
		void StartScreen(Screen screen);

		int  Start_Network(int iNewProfile = -1);
		int  Stop_Network();
		void GetNetworkProfileName(int iProfile, char *buf, size_t size);
		void PlayListScreenInputHandler(int iButtonMask);
		
		Screen GetCurrentScreen(){return m_CurrentScreen;}
		int 	GetCurrentNetworkProfile() { return m_iNetworkProfile; }
		
		
		request_on_play_stop m_RequestOnPlayOrStop;
		
	private:
		Screen m_CurrentScreen;
		IPSPRadio_UI *m_UI;
		CIniParser *m_Config;
		CPSPSound *m_Sound;
		int m_iNetworkProfile;
		bool m_NetworkStarted;
		CPlayList *m_CurrentPlayList;
		CDirList  *m_CurrentPlayListDir;
	};
#endif
