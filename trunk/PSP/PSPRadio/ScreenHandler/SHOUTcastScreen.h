#ifndef __SHOUTCAST_SCREEN__
	#define __SHOUTCAST_SCREEN__

	#include "ScreenHandler.h"
	#include "PlayListScreen.h"
	#include "MetaDataContainer.h"
	
	
	class SHOUTcastScreen : public PlayListScreen
	{
		public:
			SHOUTcastScreen(int Id, CScreenHandler *ScreenHandler);
			~SHOUTcastScreen();

			void LoadLists();
		
		private:
	};

#endif
