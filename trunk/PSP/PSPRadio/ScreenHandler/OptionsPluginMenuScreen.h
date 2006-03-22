#ifndef __OPTIONS_PLUGINMENU_SCREEN__
	#define __OPTIONS_PLUGINMENU_SCREEN__

	#include "OptionsScreen.h"

	enum plugin_type
	{
		PLUGIN_UI,
		PLUGIN_FSS,
		PLUGIN_APP,
		/** This has to be the last */
		NUMBER_OF_PLUGINS,
	};

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
		
		private:
			int LoadPlugin(char *strPlugin, plugin_type type);
			//int LoadFSSPlugin(char *strPlugin);
			//int LoadAPPPlugin(char *strPlugin);
			
		private:
			CPRXLoader *m_ModuleLoader[NUMBER_OF_PLUGINS];

	};

#endif
