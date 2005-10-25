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
			PSPRADIO_SCREEN_SHOUTCAST_BROWSER,
			/** Add more elements here **/

			PSPRADIO_SCREEN_OPTIONS /* Leave this as the last one */
		};
		/** These point to the first and one-beyond the last elements of the
			list; just like in a sdl list. This to allow for easy iteration
			though the list with a for loop/etc. 
		*/
		#define PSPRADIO_SCREEN_LIST_BEGIN  PSPRADIO_SCREEN_PLAYLIST
		#define PSPRADIO_SCREEN_LIST_END	(PSPRADIO_SCREEN_OPTIONS+1)
		
		enum UIs
		{
			UI_TEXT = 0,
			UI_GRAPHICS = 1,
			UI_3D = 2,
		};
		
		enum request_on_play_stop
		{
			NOTHING,
			PLAY,
			STOP
		};
		
		enum PlayListSide
		{
			PLAYLIST_LIST,
			PLAYLIST_ENTRIES
		};
		
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
		
		CScreenHandler(CIniParser *Config, CPSPSound *Sound);
		~CScreenHandler();
		IPSPRadio_UI *StartUI(UIs UI);
		
		void SetUp(CIniParser *Config, CPSPSound *Sound, 
					CPlayList *CurrentPlayList, CDirList  *CurrentPlayListDir);
		void StartScreen(Screen screen);
		void CommonInputHandler(int iButtonMask);

		int  Start_Network(int iNewProfile = -1);
		int  Stop_Network();
		void GetNetworkProfileName(int iProfile, char *buf, size_t size);
		void PlayListScreenInputHandler(int iButtonMask);
		
		void DownloadSHOUTcastDB();


		Screen GetCurrentScreen(){return m_CurrentScreen;}
		int 	GetCurrentNetworkProfile() { return m_iNetworkProfile; }
		
		void OnVBlank();
		
		request_on_play_stop m_RequestOnPlayOrStop;
		
	private:
		Screen m_CurrentScreen;
		Screen m_PreviousScreen;
		UIs m_CurrentUI;
		IPSPRadio_UI *m_UI;
		CIniParser *m_Config;
		CPSPSound *m_Sound;
		int m_iNetworkProfile;
		bool m_NetworkStarted;
		CPlayList *m_CurrentPlayList;
		CDirList  *m_CurrentPlayListDir;
		PlayListSide m_CurrentPlayListSideSelection;
	};
#endif
