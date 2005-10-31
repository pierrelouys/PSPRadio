#ifndef __SHOUTCAST_SCREEN__
	#define __SHOUTCAST_SCREEN__

	#include "ScreenHandler.h"
	#include "PlayListScreen.h"

	class SHOUTcastScreen : public PlayListScreen
	{
		public:
			SHOUTcastScreen(int Id, CScreenHandler *ScreenHandler);
			~SHOUTcastScreen(){};

			void LoadLists();
			
			void Activate(IPSPRadio_UI *UI);

			void InputHandler(int iButtonMask);
		
		private:
	};

#endif
