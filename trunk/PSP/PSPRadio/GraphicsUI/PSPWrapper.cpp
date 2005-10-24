#include "PSPWrapper.h"

CPSPWrapper::CPSPWrapper(void)
{
}

CPSPWrapper::~CPSPWrapper(void)
{
	Terminate();
}

bool CPSPWrapper::Initialize(char *szThemeFileName)
{
	if(0 != m_GUI.Initialize(szThemeFileName))
	{
		return false;
	}

	return true;
}

bool CPSPWrapper::Terminate(void)
{
	m_GUI.Terminate();
	return true;
}

bool CPSPWrapper::Run(void)
{
	SDL_Event event;

	static int count = 1;
	
	/* Ignore key events */
//	SDL_EventState(SDL_KEYDOWN, SDL_IGNORE);
//	SDL_EventState(SDL_KEYUP, SDL_IGNORE);

	/* Loop waiting for ESC+Mouse_Button */
	while(SDL_WaitEvent(&event) >= 0) 
	{
		switch (event.type) 
		{
			case SDL_ACTIVEEVENT: 
			{
			} break;

			case SDL_KEYDOWN:
			{
				switch(event.key.keysym.sym)
				{
					case SDLK_F1: 
					{
						m_GUI.DisplayMainScreen();
					} break;

					case SDLK_F2:
					{
						m_GUI.DisplayShoutcastScreen();
					} break;

					case SDLK_F3:
					{
						m_GUI.DisplaySettingScreen();
					} break;

					case SDLK_1:
					{
						m_GUI.DisplayButton(BP_PLAY);
					} break;

					case SDLK_2:
					{
						m_GUI.DisplayButton(BP_PAUSE);
					} break;

					case SDLK_3:
					{
						m_GUI.DisplayButton(BP_STOP);
					} break;

					case SDLK_4:
					{
						m_GUI.DisplayButton(BP_LOAD);
					} break;

					case SDLK_5:
					{
						m_GUI.DisplayButton(BP_SOUND);
					} break;

					case SDLK_6:
					{
						m_GUI.DisplayButton(BP_VOLUME);
					} break;

					case SDLK_7:
					{
						m_GUI.DisplayString("TEST 1", OA_PLAYLIST, count);
						m_GUI.DisplayString("TEST 1", OA_PLAYLISTITEM, count);
						count++;
					} break;

					case SDLK_8:
					{
						m_GUI.DisplayString("TEST 1", OA_SETTINGS, count);
						count++;
					} break;

					case SDLK_9:
					{
					} break;

					case SDLK_0:
					{
					} break;


					case SDLK_a:
					{
						m_GUI.DisplayString("SP_FILENAME", SP_FILENAME);
					} break;

					case SDLK_b:
					{
						m_GUI.DisplayString("SP_FILETITLE", SP_FILETITLE);
					} break;

					case SDLK_c:
					{
						m_GUI.DisplayString("SP_URI", SP_URI);
					} break;

					case SDLK_d:
					{
						m_GUI.DisplayString("SP_BUFFER", SP_BUFFER);
					} break;

					case SDLK_e:
					{
						m_GUI.DisplayString("SP_SAMPLERATE", SP_SAMPLERATE);
					} break;

					case SDLK_f:
					{
						m_GUI.DisplayString("SP_MPEGLAYER", SP_MPEGLAYER);
					} break;

					case SDLK_g:
					{
						m_GUI.DisplayString("SP_STREAM", SP_STREAM);
					} break;

					case SDLK_h:
					{
						m_GUI.DisplayString("SP_ERROR", SP_ERROR);
					} break;

					case SDLK_i:
					{
						m_GUI.DisplayString("SP_NETWORK", SP_NETWORK);
					} break;

					case SDLK_j:
					{
						m_GUI.DisplayString("SP_SONGTITLE", SP_SONGTITLE);
					} break;

					case SDLK_k:
					{
						m_GUI.DisplayString("SP_SONGAUTHOR", SP_SONGAUTHOR);
					} break;

					case SDLK_l:
					{
						
						m_GUI.DisplayString("SP_LENGTH", SP_LENGTH);
					} break;

					case SDLK_m:
					{
						m_GUI.DisplayString("SP_BITRATE", SP_BITRATE);
					} break;

					case SDLK_n:
					{
					} break;

					case SDLK_o:
					{
					} break;

					case SDLK_p:
					{
					} break;

					case SDLK_q:
					{
					} break;

					case SDLK_r:
					{
					} break;

					case SDLK_s:
					{
					} break;

					case SDLK_t:
					{
					} break;

					case SDLK_u:
					{
					} break;

					case SDLK_v:
					{
					} break;

					case SDLK_w:
					{
					} break;

					case SDLK_x:
					{
					} break;

					case SDLK_y:
					{
					} break;

					case SDLK_z:
					{
					} break;
				}

			} break;

			case SDL_KEYUP:
			{
			} break;

			case SDL_MOUSEMOTION:
			{
			} break;
				
			case SDL_MOUSEBUTTONUP:
			{
			} break;

			case SDL_JOYAXISMOTION:
			{
			} break;

			case SDL_JOYBALLMOTION:
			{
			} break;

			case SDL_JOYHATMOTION:
			{
			} break;

			case SDL_JOYBUTTONDOWN:
			{
			} break;

			case SDL_JOYBUTTONUP:
			{
			} break;

			case SDL_MOUSEBUTTONDOWN: 
			{
			} break;

			case SDL_SYSWMEVENT:
			{
			} break;

			case SDL_VIDEORESIZE:
			{
			} break;

			case SDL_VIDEOEXPOSE:
			{
			} break;

			case SDL_USEREVENT:
			{
			} break;

			case SDL_QUIT: 
			{
				return true;
			} break;
		}
	}

	return false;
}

int main(int argc, char *argv[])
{
	CPSPWrapper psp;

	if(false == psp.Initialize("PSPRadio_AllStates.theme"))
	{
		printf("Error initializing PSPWrapper\n");
	}

	psp.Run();

	return 1;
}