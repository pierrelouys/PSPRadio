#ifndef __OPTIONS_SCREEN__
	#define __OPTIONS_SCREEN__

	#include "ScreenHandler.h"

	class OptionsScreen : public IScreen
	{
		public:
			OptionsScreen(int Id, CScreenHandler *ScreenHandler);
			virtual ~OptionsScreen(){};
			
			virtual void UpdateOptionsData();

			virtual void Activate(IPSPRadio_UI *UI);

			virtual void InputHandler(int iButtonMask);
			
			virtual int  Start_Network(int iNewProfile = -1);

			virtual void LoadFromConfig();
			virtual void SaveToConfigFile();

			
			#define MAX_OPTION_LENGTH 60
			#define MAX_NUM_OF_OPTIONS 20
			
			/** Options screen */
			struct Options
			{
				int	 Id;
				char strName[MAX_OPTION_LENGTH];
				char *strStates[MAX_NUM_OF_OPTIONS];
				int  iActiveState;		/** indicates the currently active state -- user pressed X on this state */
				int  iSelectedState;	/** selection 'box' around option; not active until user presses X */
				int  iNumberOfStates;
			};

		protected:
			virtual void OnOptionActivation();
			//virtual void PopulateOptionsData();
		
		private:
			int  Stop_Network();
			int  GetCurrentNetworkProfile() { return m_iNetworkProfile; }
		
		protected:
			Options *OptionsData;
			list<Options> m_OptionsList;
			list<Options>::iterator m_CurrentOptionIterator;
			int m_iNetworkProfile;
			bool m_WifiAutoStart, m_USBAutoStart;

	};

#endif
