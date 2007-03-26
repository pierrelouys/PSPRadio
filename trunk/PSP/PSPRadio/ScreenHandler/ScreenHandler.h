#ifndef __SCREEN_HANDLER__
	#define __SCREEN_HANDLER__

	#include <PSPSound.h>
	#include <iniparser.h>
	#include <PRXLoader.h>

#ifdef __cplusplus
	#include <list>
	using namespace std;
#endif

	//#define IS_BUTTON_PRESSED(i,b) ((i & b) == b)
	#define IS_BUTTON_PRESSED(i,b) ((i & 0xFFFF) == b)
	
	#define DEFAULT_UI_MODULE "UI_Text.prx"
	#define DEFAULT_SKIN	  "Default"

	/** Messages from PSPSound to decode thread */
	typedef enum 
	{
		EID_NEW_UI_POINTER,
		EID_EXIT_SELECTED
	}EventIDsFromScreenHandlerToPSPRadio;

	typedef enum 
	{
		PLAYSTATE_PLAY,
		PLAYSTATE_STOP,
		PLAYSTATE_PAUSE,
	}playstates;

#ifdef __cplusplus
	class IPSPRadio_UI;

	class CScreenHandler;

	class IScreen
	{
		public:

			IScreen(int Id, CScreenHandler *ScreenHandler)
				{m_ScreenHandler = ScreenHandler, m_Id = Id; m_strConfigFilename = NULL;}
			virtual ~IScreen()
				{if (m_strConfigFilename)free(m_strConfigFilename), m_strConfigFilename= NULL;}

			int GetId(){ return m_Id; }

			virtual void Activate();

			virtual void InputHandler(int iButtonMask){};

			virtual void OnPlayStateChange(playstates NewPlayState){};
			virtual void EOSHandler(){};

			void  SetConfigFilename(char *strConfigFilename){m_strConfigFilename = strdup(strConfigFilename);}
			char *GetConfigFilename(){return m_strConfigFilename;}

		protected:
			int m_Id;
			CScreenHandler *m_ScreenHandler;
			char *m_strConfigFilename;
	};
#endif

	typedef enum playmodes
	{
		PLAYMODE_NORMAL,
		PLAYMODE_SINGLE,
		PLAYMODE_REPEAT,
		PLAYMODE_GLOBAL_NEXT,
	} playmodes;

#ifdef __cplusplus
		class CScreenHandler
		{
		public:
			enum Screen
			{
				PSPRADIO_SCREEN_LOCALFILES,
				PSPRADIO_SCREEN_PLAYLIST,
				PSPRADIO_SCREEN_SHOUTCAST_BROWSER,
				/** Add more elements here **/
	
				PSPRADIO_SCREEN_OPTIONS /* Leave this as the last one */
				,PSPRADIO_SCREEN_OPTIONS_PLUGIN_MENU /** Or this one for dynamic builds */
			};
			/** These point to the first and one-beyond the last elements of the
				list; just like in a sdl list. This to allow for easy iteration
				though the list with a for loop/etc.
			*/
			#define PSPRADIO_SCREEN_LIST_BEGIN  PSPRADIO_SCREEN_LOCALFILES
			#define PSPRADIO_SCREEN_LIST_END	(PSPRADIO_SCREEN_OPTIONS_PLUGIN_MENU+1)
	
			enum request_on_play_stop
			{
				NOTHING,
				PLAY_REQUEST,
				STOP_REQUEST,		/** User selected to stop */
			};
	
	
			CScreenHandler(char *strCWD, CIniParser *Config, CPSPSound *Sound, Screen InitialScreen = PSPRADIO_SCREEN_PLAYLIST);
			~CScreenHandler();
			IPSPRadio_UI *StartUI(const char *strUIModule, const char *strSkinName, bool bJustChangeSkin = false);
			void PrepareShutdown();
	
			void CommonInputHandler(int iButtonMask, u32 iEventType); /** Event Type is MID_ONBUTTON_RELEASED or MID_ONBUTTON_REPEAT */
			void OnHPRMReleased(u32 iHPRMMask);
	
			bool DownloadSHOUTcastDB();
			bool DownloadNewSHOUTcastDB();
	
			IScreen *GetCurrentScreen(){return m_CurrentScreen;}
			char *GetCurrentUIName(){return m_strCurrentUIName;}
			char *GetCurrentSkin(){return m_strCurrentSkin;}
			bool  SetCurrentUIName(const char *strNewName);
			bool  SetCurrentSkinName(const char *strNewName);
			//IPSPRadio_UI *GetCurrentUIPtr(){ return m_UI; }
			char *GetCWD(){return m_strCWD;}
			CPSPSound *GetSound(){return m_Sound;}
			IScreen *GetScreen(int Id){return Screens[Id];}
			CIniParser *GetConfig() { return m_Config;}
	
			Screen GetInitialScreen(){ return m_InitialScreen; }
			void SetInitialScreen(Screen screen){m_InitialScreen = screen;}
	
			IScreen *GetStreamOwnerScreen() { return m_StreamOwnerScreen; }
			void SetStreamOwnerScreen(IScreen *screen) { m_StreamOwnerScreen = screen; }
	
			playmodes GetPlayMode() { return m_PlayMode; }
			void SetPlayMode(playmodes mode) { m_PlayMode = mode; }
	
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
			friend class OptionsScreen;
			void SetCurrentScreen(IScreen *pScreen){m_CurrentScreen = pScreen;}
	
		private:
			IScreen *m_CurrentScreen;
			IScreen *m_PreviousScreen;
			IScreen *m_StreamOwnerScreen;
			Screen   m_InitialScreen;
			//CPRXLoader *m_UIModuleLoader;
			char *m_strCurrentUIName;
			char *m_strCurrentSkin;
			//IPSPRadio_UI *m_UI;
			CIniParser *m_Config;
			CPSPSound *m_Sound;
			char *m_strCWD;
			IScreen *Screens[PSPRADIO_SCREEN_LIST_END];
			playmodes m_PlayMode;
		};
	#endif
#endif // __cplusplus
