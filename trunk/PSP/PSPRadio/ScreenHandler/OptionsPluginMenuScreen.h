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

			
		protected:
			void OnOptionActivation();
		//	void PopulateOptionsData();
		
		private:
			int LoadFSSPlugin(int iFSSIndex);
			
		private:
			CPRXLoader *m_FSSModuleLoader;
			
	};

#endif
