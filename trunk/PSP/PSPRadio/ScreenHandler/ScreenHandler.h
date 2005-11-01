#ifndef __SCREEN_HANDLER__
	#define __SCREEN_HANDLER__
	
	#include <PSPSound.h>
	#include <iniparser.h>
	#include "PlayList.h"
	#include "DirList.h"
	#include <list>
	using namespace std;


	/** Messages from PSPSound to decode thread */
	enum EventIDsFromScreenHandlerToPSPRadio
	{
		EID_NEW_UI_POINTER,
	};

	/** Sender IDs */
	#define SID_SCREENHANDLER		0x00000001

	class IPSPRadio_UI;

	class CScreenHandler;
	
	class IScreen
	{
		public:
			IScreen(int Id, CScreenHandler *ScreenHandler)
				{m_ScreenHandler = ScreenHandler, m_Id = Id;}
			virtual ~IScreen(){}
			
			int GetId(){ return m_Id; }

			virtual void Activate(IPSPRadio_UI *UI);

			virtual void InputHandler(int iButtonMask){};
		
		protected:
			int m_Id;
			IPSPRadio_UI *m_UI;
			CScreenHandler *m_ScreenHandler;
	};

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
		
		
		CScreenHandler(char *strCWD, CIniParser *Config, CPSPSound *Sound);
		~CScreenHandler();
		IPSPRadio_UI *StartUI(UIs UI);
		
		void CommonInputHandler(int iButtonMask);
		void OnHPRMReleased(u32 iHPRMMask);

		bool DownloadSHOUTcastDB();


		IScreen *GetCurrentScreen(){return m_CurrentScreen;}
		UIs GetCurrentUI(){return m_CurrentUI;}
		char *GetCWD(){return m_strCWD;}
		CPSPSound *GetSound(){return m_Sound;}
		IScreen *GetScreen(int Id){return Screens[Id];}

		void OnVBlank();
		
		request_on_play_stop m_RequestOnPlayOrStop;

		/** Screenshot functionality */

		#define SCREEN_WIDTH 480
		#define SCREEN_HEIGHT 272

		enum ScreenShotState
		{
			PSPRADIO_SCREENSHOT_ACTIVE,
			PSPRADIO_SCREENSHOT_NOT_ACTIVE
		};

		void Screenshot();
		char *ScreenshotName(char *path);
		void ScreenshotStore(char *filename);
		
	protected:
		friend class IScreen;
		void SetCurrentScreen(IScreen *pScreen){m_CurrentScreen = pScreen;}

	private:
		IScreen *m_CurrentScreen;
		IScreen *m_PreviousScreen;
		UIs m_CurrentUI;
		IPSPRadio_UI *m_UI;
		CIniParser *m_Config;
		CPSPSound *m_Sound;
		char *m_strCWD;
		IScreen *Screens[PSPRADIO_SCREEN_LIST_END];
	};
#endif
