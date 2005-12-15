#ifndef __LOCALFILES_SCREEN__
	#define __LOCALFILES_SCREEN__

	#include "MetaDataContainer.h"
	#include "ScreenHandler.h"
	#include "PlayListScreen.h"

	class LocalFilesScreen : public PlayListScreen
	{
		public:
			LocalFilesScreen(int Id, CScreenHandler *ScreenHandler);
			~LocalFilesScreen();
			
			void LoadLists();

		protected:
			char *m_strPath;
	};

#endif
