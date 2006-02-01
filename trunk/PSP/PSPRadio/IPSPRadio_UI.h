#ifndef _UI_I_
	#define _UI_I_

	#include "ScreenHandler.h"
	#include "PlayListScreen.h"
	#include "OptionsScreen.h"
	#include "PSPSound.h"
	#include "MetaDataContainer.h"
	#include <psprtc.h>

	/** UI class interface */
	class IPSPRadio_UI
	{
	public:
		virtual ~IPSPRadio_UI();

		virtual int Initialize(char *strCWD){return 0;};
		virtual void PrepareShutdown(){};
		virtual void Terminate(){};

		virtual int SetTitle(char *strTitle){return 0;};
		virtual int DisplayMessage_EnablingNetwork(){return 0;};
		virtual int DisplayMessage_DisablingNetwork(){return 0;};
		virtual int DisplayMessage_NetworkReady(char *strIP){return 0;};
		virtual int DisplayMessage_NetworkSelection(int iProfileID, char *strProfileName){return 0;};
		virtual int DisplayMainCommands(){return 0;};
		virtual int DisplayActiveCommand(CPSPSound::pspsound_state playingstate){return 0;};
		virtual int DisplayMessage(char *strMsg){return 0;};
		virtual int DisplayErrorMessage(char *strMsg){return 0;};
		virtual int DisplayBufferPercentage(int a){return 0;};

		/** these are listed in sequential order */
		virtual int OnNewStreamStarted(){return 0;};
		virtual int OnStreamOpening(){return 0;};
		virtual int OnConnectionProgress(){return 0;};
		virtual int OnStreamOpeningError(){return 0;};
		virtual int OnStreamOpeningSuccess(){return 0;};
		virtual int OnVBlank(){return 0;};
		virtual int OnNewSongData(MetaData *pData){return 0;};
		virtual int OnTimeUpdate(MetaData *pData){return 0;};

		/** Screen Handling */
		virtual void Initialize_Screen(IScreen *Screen){};
		virtual void UpdateOptionsScreen(list<OptionsScreen::Options> &OptionsList,
										 list<OptionsScreen::Options>::iterator &CurrentOptionIterator){};
		virtual void OnScreenshot(CScreenHandler::ScreenShotState state){};
		virtual void OnBatteryChange(int Percentage){};
		virtual void OnTimeChange(pspTime *LocalTime){};
		virtual void OnUSBEnable(){};
		virtual void OnUSBDisable(){};

		virtual void DisplayContainers(CMetaDataContainer *Container){}
		virtual void DisplayElements(CMetaDataContainer *Container){}
		virtual void OnCurrentContainerSideChange(CMetaDataContainer *Container){}
		};

#endif

