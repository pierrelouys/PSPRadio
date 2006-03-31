#ifndef __OPTIONS_PLUGINMENU_SCREEN__
	#define __OPTIONS_PLUGINMENU_SCREEN__

	#include "OptionsScreen.h"

	class OptionsPluginMenuScreen : public OptionsScreen
	{
		public:
			OptionsPluginMenuScreen(int Id, CScreenHandler *ScreenHandler);
			~OptionsPluginMenuScreen();			
			void UpdateOptionsData();

			void Activate(IPSPRadio_UI *UI);

			void LoadFromConfig();
			void SaveToConfigFile();

		protected:
			void OnOptionActivation();
			int  RetrievePlugins(Options &Option, char *strPrefix, char *strActive, bool bInsertOff = false);
	};

#endif
