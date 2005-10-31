#ifndef __PLAYLIST_SCREEN__
	#define __PLAYLIST_SCREEN__

	#include "ScreenHandler.h"

	class PlayListScreen : public IScreen
	{
		public:
			enum PlayListSide
			{
				PLAYLIST_LIST,
				PLAYLIST_ENTRIES
			};

			PlayListScreen(int Id, CScreenHandler *ScreenHandler, CPlayList *CurrentPlayList, CDirList  *CurrentPlayListDir);
			virtual ~PlayListScreen();
			
			virtual void LoadLists();

			virtual void Activate(IPSPRadio_UI *UI);

			virtual void InputHandler(int iButtonMask);

			virtual void OnHPRMReleased(u32 iHPRMMask);

		protected:
			CPlayList *m_CurrentPlayList;
			CDirList  *m_CurrentPlayListDir;
			PlayListSide m_CurrentPlayListSideSelection;
	};

#endif
