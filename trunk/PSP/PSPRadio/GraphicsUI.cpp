/* 
	PSPRadio / Music streaming client for the PSP. (Initial Release: Sept. 2005)
	Copyright (C) 2005  Rafael Cabezas a.k.a. Raf
	
	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include <list>
#include <PSPApp.h>
#include <PSPSound_MP3.h>
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <malloc.h>
#include <iniparser.h>
#include <Tools.h>
#include <stdarg.h>
#include <Logging.h>

#include "GraphicsUI.h"

#define PSP_RES_WIDTH	480
#define PSP_RES_HEIGHT	272

#define THEME_FILE 		"PSPRadio_AllStates.theme"

CGraphicsUI::CGraphicsUI()
{
	m_pImageBase = NULL;
	m_pScreen = NULL;
	m_pVideoInfo = NULL;
	m_nDepth = -1;
	m_nFlags = SDL_FULLSCREEN | /*SDL_DOUBLEBUF |*/ SDL_HWSURFACE;
}

CGraphicsUI::~CGraphicsUI()
{
}

int CGraphicsUI::Initialize(char *strCWD)
{	
	char szThemeFile[256];
	char szThemePath[256];

	sprintf(szThemePath, "%s/THEME/", strCWD);
	sprintf(szThemeFile, "%s%s", szThemePath, THEME_FILE);

	Log(LOG_LOWLEVEL, "Initialize: Theme Initializing");
	if(FALSE == InitializeTheme(szThemeFile, szThemePath))
	{
		Log(LOG_ERROR, "Initialize: error initializing Theme");
		return -1;
	}		
	Log(LOG_LOWLEVEL, "Initialize: Theme Initialized");

	
	Log(LOG_LOWLEVEL, "Initialize: SDL Initializing");		
	if(FALSE == InitializeSDL())
	{
		Log(LOG_ERROR, "Initialize: error initializing SDL");
		return -1;
	}	
	Log(LOG_LOWLEVEL, "Initialize: SDL Initialized");	
	
	Log(LOG_LOWLEVEL, "Initialize: Images Initializing");
	if(FALSE == InitializeImages())
	{
		Log(LOG_ERROR, "Initialize: error initializing images");
		return -1;
	}	
	Log(LOG_LOWLEVEL, "Initialize: Images Initialied");
	
	SetBaseImage();
	
	DisplayWord("This is a test", 10, 10);
	
	return 0;
}

SDL_Surface *CGraphicsUI::LoadImage(char *szImageName)
{
	SDL_Surface *pImage = NULL;
	pImage = IMG_Load(szImageName);
	
	if(NULL == pImage)
	{
		Log(LOG_ERROR, "LoadImage: error loading image %s : %s",
			szImageName,
			SDL_GetError());		
	}
	else
	{
		Log(LOG_INFO, "LoadImage: %s loaded", szImageName);
	}
	
	return pImage;
}

void CGraphicsUI::UnLoadImage(SDL_Surface **ppImage)
{
	if(NULL != *ppImage)
	{
		SDL_FreeSurface(*ppImage);
		*ppImage = NULL;
	}
}

void CGraphicsUI::Terminate()
{
	Log(LOG_INFO, "Terminate: unloading images");
	UnLoadImage(&m_pImageBase);
	Log(LOG_INFO, "Terminate: images all unloaded");
	
	/** If we are initialized do some cleaning up **/
	if(0 != SDL_WasInit(SDL_INIT_VIDEO))
	{
		Log(LOG_INFO, "Terminate: cleaning up SDL");
		/** Shut down SDL **/
		SDL_Quit();
	}
	
	Log(LOG_INFO, "Terminate: completed");
}

int CGraphicsUI::SetTitle(char *strTitle)
{
	return 0;
}

int CGraphicsUI::DisplayMessage_EnablingNetwork()
{
	return 0;
}

int CGraphicsUI::DisplayMessage_NetworkSelection(int iProfileID, char *strProfileName)
{
	return 0;
}

int CGraphicsUI::DisplayMessage_DisablingNetwork()
{
	return 0;
}

int CGraphicsUI::DisplayMessage_NetworkReady(char *strIP)
{
	return 0;
}

int CGraphicsUI::DisplayMainCommands()
{
	return 0;
}

int CGraphicsUI::DisplayActiveCommand(CPSPSound::pspsound_state playingstate)
{
	switch(playingstate)
	{
		case CPSPSound::STOP:
			SetPlayButton(UIBUTTONSTATE_OFF);
			SetPauseButton(UIBUTTONSTATE_OFF);
			SetStopButton(UIBUTTONSTATE_ON);			
			break;
		
		case CPSPSound::PLAY:
			SetPlayButton(UIBUTTONSTATE_ON);
			SetPauseButton(UIBUTTONSTATE_OFF);
			SetStopButton(UIBUTTONSTATE_OFF);			
			break;
		
		case CPSPSound::PAUSE:
			SetPlayButton(UIBUTTONSTATE_OFF);
			SetPauseButton(UIBUTTONSTATE_ON);
			SetStopButton(UIBUTTONSTATE_OFF);			
			break;
	}
	
	return 0;
}

int CGraphicsUI::DisplayErrorMessage(char *strMsg)
{
	return 0;
}

int CGraphicsUI::DisplayBufferPercentage(int iPercentage)
{
	return 0;
}

int CGraphicsUI::OnNewStreamStarted()
{
	return 0;
}

int CGraphicsUI::OnStreamOpening()
{
	return 0;
}

int CGraphicsUI::OnStreamOpeningError()
{
	return 0;
}

int CGraphicsUI::OnStreamOpeningSuccess()
{
	SDL_Rect src = 	{ 
						m_themeItemLoad.GetSrc(1).x,
						m_themeItemLoad.GetSrc(1).y,
						m_themeItemLoad.GetSize(1).x,
						m_themeItemLoad.GetSize(1).y
					};
					
	SDL_Rect dst = 	{ 
						m_themeItemLoad.m_pointDst.x,
						m_themeItemLoad.m_pointDst.y,
					};
		
	SDL_BlitSurface(m_pImageBase, &src, m_pScreen, &dst);

	return 0;
}

int CGraphicsUI::OnVBlank()
{
	return 0;
}

int CGraphicsUI::OnNewSongData(CPlayList::songmetadata *pData)
{
	return 0;
}

int CGraphicsUI::DisplaySampleRateAndKBPS(int samplerate, int bitrate)
{
	return 0;
}

int CGraphicsUI::DisplayMPEGLayerType(char *strType)
{
	return 0;
}

int CGraphicsUI::DisplayPLList(CDirList *plList)
{
	return 0;
}

int CGraphicsUI::OnConnectionProgress()
{
	SDL_Rect src = 	{ 
						m_themeItemLoad.GetSrc(0).x,
						m_themeItemLoad.GetSrc(0).y,
						m_themeItemLoad.GetSize(0).x,
						m_themeItemLoad.GetSize(0).y
					};
					
	SDL_Rect dst = 	{ 
						m_themeItemLoad.m_pointDst.x,
						m_themeItemLoad.m_pointDst.y,
					};
		
	SDL_BlitSurface(m_pImageBase, &src, m_pScreen, &dst);

	return 0;
}

void CGraphicsUI::SetBaseImage(void)
{
	SDL_Rect src = 	{ 
						m_themeItemBackground.GetSrc(0).x,
						m_themeItemBackground.GetSrc(0).y,
						m_themeItemBackground.GetSize(0).x,
						m_themeItemBackground.GetSize(0).y
					};
					
	SDL_Rect dst = 	{ 
						m_themeItemBackground.m_pointDst.x,
						m_themeItemBackground.m_pointDst.y,
					};
		
	SDL_BlitSurface(m_pImageBase, &src, m_pScreen, &dst);
}

void CGraphicsUI::SetPlayButton(uibuttonstate_enum state)
{
	SDL_Rect src = 	{ 
						m_themeItemPlay.GetSrc(state).x,
						m_themeItemPlay.GetSrc(state).y,
						m_themeItemPlay.GetSize(state).x,
						m_themeItemPlay.GetSize(state).y
					};
					
	SDL_Rect dst = 	{ 
						m_themeItemPlay.m_pointDst.x,
						m_themeItemPlay.m_pointDst.y,
					};
		
	SDL_BlitSurface(m_pImageBase, &src, m_pScreen, &dst);
}

void CGraphicsUI::SetPauseButton(uibuttonstate_enum state)
{
	SDL_Rect src = 	{ 
						m_themeItemPause.GetSrc(state).x,
						m_themeItemPause.GetSrc(state).y,
						m_themeItemPause.GetSize(state).x,
						m_themeItemPause.GetSize(state).y
					};
					
	SDL_Rect dst = 	{ 
						m_themeItemPause.m_pointDst.x,
						m_themeItemPause.m_pointDst.y,
					};
		
	SDL_BlitSurface(m_pImageBase, &src, m_pScreen, &dst);
}

void CGraphicsUI::SetStopButton(uibuttonstate_enum state)
{
	SDL_Rect src = 	{ 
						m_themeItemStop.GetSrc(state).x,
						m_themeItemStop.GetSrc(state).y,
						m_themeItemStop.GetSize(state).x,
						m_themeItemStop.GetSize(state).y
					};
					
	SDL_Rect dst = 	{ 
						m_themeItemStop.m_pointDst.x,
						m_themeItemStop.m_pointDst.y,
					};
		
	SDL_BlitSurface(m_pImageBase, &src, m_pScreen, &dst);
}

bool CGraphicsUI::InitializeTheme(char *szFilename, char *szThemePath)
{
	char szBaseImage[256];
	if(0 != m_theme.Initialize(szFilename))
	{
		Log(LOG_ERROR, "InitializeTheme: error initializing theme (%s)", szFilename);
		return FALSE;
	}	
	
	/** Get theme image */
	Log(LOG_LOWLEVEL, "InitializeTheme: getting image path");
	//if(0 != m_theme.GetImagePath(m_szThemeImagePath, sizeof(m_szThemeImagePath)))
	if(0 != m_theme.GetImagePath(szBaseImage, sizeof(szBaseImage)))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme image path");
		return FALSE;
	}	
	
	sprintf(m_szThemeImagePath, "%s%s", szThemePath, szBaseImage);
	Log(LOG_LOWLEVEL, "InitializeTheme: base image = %s", m_szThemeImagePath);
	
	/** Get the theme items */
	Log(LOG_LOWLEVEL, "InitializeTheme: getting background");
	if(0 != m_theme.GetItem("background", &m_themeItemBackground))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme background");
		return FALSE;
	}
	
	Log(LOG_LOWLEVEL, "InitializeTheme: getting play");
	if(0 != m_theme.GetItem("play", &m_themeItemPlay))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme play");
		return FALSE;
	}
	
	Log(LOG_LOWLEVEL, "InitializeTheme: getting pause");
	if(0 != m_theme.GetItem("pause", &m_themeItemPause))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme pause");
		return FALSE;
	}
	
	Log(LOG_LOWLEVEL, "InitializeTheme: getting stop");
	if(0 != m_theme.GetItem("stop", &m_themeItemStop))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme stop");
		return FALSE;
	}
	
	Log(LOG_LOWLEVEL, "InitializeTheme: getting load");
	if(0 != m_theme.GetItem("load", &m_themeItemLoad))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme load");
		return FALSE;
	}
	
	Log(LOG_LOWLEVEL, "InitializeTheme: getting sound");
	if(0 != m_theme.GetItem("sound", &m_themeItemSound))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme sound");
		return FALSE;
	}
	
	Log(LOG_LOWLEVEL, "InitializeTheme: getting volume");
	if(0 != m_theme.GetItem("volume", &m_themeItemVolume))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme volume");
		return FALSE;
	}
	
	Log(LOG_LOWLEVEL, "InitializeTheme: getting letters");
	if(0 != m_theme.GetLetters("letters", &m_themeItemLetters))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme letters");
		return FALSE;
	}
	
	Log(LOG_LOWLEVEL, "InitializeTheme: getting numbers");
	if(0 != m_theme.GetNumbers("numbers", &m_themeItemNumbers))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme numbers");
		return FALSE;
	}
	
	return TRUE;
}

bool CGraphicsUI::InitializeSDL()
{
	Log(LOG_LOWLEVEL, "InitializeSDL: SDL Initializing with SDL_INIT_VIDEO");
	if(SDL_Init(SDL_INIT_VIDEO) < 0) 
	{	
		Log(LOG_ERROR, "InitializeSDL: SDL_Init error : %s", SDL_GetError());
		return FALSE;
	}	
	Log(LOG_LOWLEVEL, "InitializeSDL: SDL Initialized with SDL_INIT_VIDEO");	
	
	Log(LOG_LOWLEVEL, "InitializeSDL: Checking video mode"); 	
	m_nDepth = SDL_VideoModeOK(PSP_RES_WIDTH, PSP_RES_HEIGHT, 32, m_nFlags);
	Log(LOG_LOWLEVEL, "InitializeSDL: Checking video mode completed depth %d", m_nDepth); 	
	
		
	Log(LOG_LOWLEVEL, "InitializeSDL: Setting video mode"); 	
 	if(NULL == (m_pScreen = SDL_SetVideoMode(PSP_RES_WIDTH, 
 												PSP_RES_HEIGHT, 
 												m_nDepth, 
 												m_nFlags)))
 	{
		Log(LOG_ERROR, "InitializeSDL: SDL_SetVideoMode error %dx%dx%d video mode: %s\n",
			PSP_RES_WIDTH, PSP_RES_HEIGHT, m_nDepth, SDL_GetError());
		return FALSE;
 	}
	Log(LOG_LOWLEVEL, "InitializeSDL: Setting video mode completed");
			
	return TRUE;
}

void CGraphicsUI::DisplayWord(char *szWord, int dstX, int dxtY)
{
	for(int x = 0; x <= 29; x++)
	{
		SDL_Rect src = 	{ 
							m_themeItemLetters.GetSrc(x).x,
							m_themeItemLetters.GetSrc(x).y,
							m_themeItemLetters.GetSize(x).x,
							m_themeItemLetters.GetSize(x).y
						};
						
		SDL_Rect dst = 	{ 
							dstX += src.w,
							dxtY,
						};
			
		SDL_BlitSurface(m_pImageBase, &src, m_pScreen, &dst);
	}
}

bool CGraphicsUI::InitializeImages()
{
	Log(LOG_LOWLEVEL, "InitializeImages: Loading base image"); 		
	if(NULL == (m_pImageBase = LoadImage(m_szThemeImagePath)))
	{
		Log(LOG_ERROR, "InitializeImages: error loading base image");
		return FALSE;
	}	
	Log(LOG_LOWLEVEL, "InitializeImages: Loaded base image"); 		
	
	Log(LOG_LOWLEVEL, "InitializeSDL: Setting transparency");
	SDL_SetColorKey(m_pImageBase, SDL_SRCCOLORKEY, SDL_MapRGB(m_pImageBase->format, 255, 0, 255)); 
	Log(LOG_LOWLEVEL, "InitializeSDL: Setting transparency completed");	

	return TRUE;
}