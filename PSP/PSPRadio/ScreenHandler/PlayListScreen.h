#ifndef __PLAYLIST_SCREEN__
	#define __PLAYLIST_SCREEN__

	#include "MetaDataContainer.h"
	#include "ScreenHandler.h"

	class PlayListScreen : public IScreen
	{
		public:
			PlayListScreen(int Id, CScreenHandler *ScreenHandler);
			virtual ~PlayListScreen();
			
			virtual void LoadLists();

			virtual void Activate(IPSPRadio_UI *UI);

			virtual void InputHandler(int iButtonMask);

			virtual void OnHPRMReleased(u32 iHPRMMask);
			
			virtual void OnPlayStateChange(CPSPSound::pspsound_state NewPlayState);

		protected:
			CMetaDataContainer *m_Lists;
	};

#endif
