#ifndef __OPTIONS_PLUGINMENU_SCREEN__
	#define __OPTIONS_PLUGINMENU_SCREEN__

	#include "OptionsScreen.h"

	class OptionsPluginMenuScreen : public OptionsScreen
	{
		public:
			OptionsPluginMenuScreen(int Id, CScreenHandler *ScreenHandler);
			
			void UpdateOptionsData();

			void Activate(IPSPRadio_UI *UI);

			void LoadFromConfig();
			void SaveToConfigFile();

			char *GetCurrentFSS();

			
		protected:
			void OnOptionActivation();
			int  RetrievePlugins(Options &Option, char *strPrefix, char *strActive, bool bInsertOff = false);
		//	void PopulateOptionsData();
		
		private:
			int LoadFSSPlugin(char *strPlugin);
			
		private:
			CPRXLoader *m_FSSModuleLoader;
			
	};

#endif
