#include "ThemeTool.h"
#include "Logging.h"

CPSPThemeTool::CPSPThemeTool(void)
{
}

CPSPThemeTool::~CPSPThemeTool(void)
{
	Terminate();
}

bool CPSPThemeTool::Initialize(char *szThemeFileName)
{
	if(0 != m_GUI.Initialize(szThemeFileName, false))
	{
		return false;
	}

	return true;
}

bool CPSPThemeTool::Terminate(void)
{
	m_GUI.Terminate();
	return true;
}

bool CPSPThemeTool::Run(void)
{
	SDL_Event event;

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
						m_GUI.DisplayButton(BP_BUFFER);
					} break;

					case SDLK_8:
					{
						m_GUI.DisplayButton(BP_STREAM);
					} break;

					case SDLK_9:
					{
						m_GUI.DisplayButton(BP_NETWORK);
					} break;

					case SDLK_0:
					{
					} break;


					case SDLK_a:
					{
						m_GUI.DisplayString("File Title", SP_META_FILETITLE);
						m_GUI.DisplayString("Uri",SP_META_URI);
						m_GUI.DisplayString("Url",SP_META_URL);
						m_GUI.DisplayString("Sample Rate",SP_META_SAMPLERATE);
						m_GUI.DisplayString("MPEG Layer",SP_META_MPEGLAYER);
						m_GUI.DisplayString("Genre",SP_META_GENRE);
						m_GUI.DisplayString("Author",SP_META_SONGAUTHOR);
						m_GUI.DisplayString("Length",SP_META_LENGTH);
						m_GUI.DisplayString("Bitrate",SP_META_BITRATE);
						m_GUI.DisplayString("Channels",SP_META_CHANNELS);
						m_GUI.DisplayString("Error",SP_ERROR);
						
					} break;

					case SDLK_b:
					{
						m_GUI.DisplayString("OA_PLAYLIST", OA_PLAYLIST, 1);
						m_GUI.DisplayString("OA_PLAYLIST", OA_PLAYLIST, 2);
						m_GUI.DisplayString("OA_PLAYLIST", OA_PLAYLIST, 3);
						m_GUI.DisplayString("OA_PLAYLIST", OA_PLAYLIST, 4);
						m_GUI.DisplayString("OA_PLAYLIST", OA_PLAYLIST, 5);
						m_GUI.DisplayString("OA_PLAYLIST", OA_PLAYLIST, 6);
					} break;

					case SDLK_c:
					{
						m_GUI.DisplayString("OA_PLAYLISTITEM", OA_PLAYLISTITEM, 1);
						m_GUI.DisplayString("OA_PLAYLISTITEM", OA_PLAYLISTITEM, 2);
						m_GUI.DisplayString("OA_PLAYLISTITEM", OA_PLAYLISTITEM, 3);
						m_GUI.DisplayString("OA_PLAYLISTITEM", OA_PLAYLISTITEM, 4);
						m_GUI.DisplayString("OA_PLAYLISTITEM", OA_PLAYLISTITEM, 5);
						m_GUI.DisplayString("OA_PLAYLISTITEM", OA_PLAYLISTITEM, 6);
					} break;

					case SDLK_d:
					{
						m_GUI.DisplayString("OA_SETTINGS", OA_SETTINGS, 1);
						m_GUI.DisplayString("OA_SETTINGS", OA_SETTINGS, 2);
						m_GUI.DisplayString("OA_SETTINGS", OA_SETTINGS, 3);
						m_GUI.DisplayString("OA_SETTINGS", OA_SETTINGS, 4);
						m_GUI.DisplayString("OA_SETTINGS", OA_SETTINGS, 5);
						m_GUI.DisplayString("OA_SETTINGS", OA_SETTINGS, 6);
					} break;

					case SDLK_e:
					{
					} break;

					case SDLK_f:
					{
					} break;

					case SDLK_g:
					{
					} break;

					case SDLK_h:
					{
					} break;

					case SDLK_i:
					{
					} break;

					case SDLK_j:
					{
					} break;

					case SDLK_k:
					{
					} break;

					case SDLK_l:
					{
						
					} break;

					case SDLK_m:
					{
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
		
		m_GUI.OnVBlank();
	}

	return false;
}

int main(int argc, char *argv[])
{
	CPSPThemeTool psp;
	
	Logging.Set("ThemeTool.log", LOG_VERYLOW);

	if(false == psp.Initialize("THEME/PSPRadio_AllStates.theme"))
	{
		printf("Error initializing PSPWrapper\n");
	}

	psp.Run();

	return 1;
}
