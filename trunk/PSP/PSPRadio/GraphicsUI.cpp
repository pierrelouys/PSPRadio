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

#define BASE_IMAGE 		"ms0:/PSP/GAME/PSPRadio/THEME/PSPRadio_sansAll.jpg"
#define STOP_IMAGE_ON 	"ms0:/PSP/GAME/PSPRadio/THEME/ON_OFF_States/Stop_ON.jpg"
#define STOP_IMAGE_OFF	"ms0:/PSP/GAME/PSPRadio/THEME/ON_OFF_States/Stop_OFF.jpg"
#define PLAY_IMAGE_ON 	"ms0:/PSP/GAME/PSPRadio/THEME/ON_OFF_States/Play_ON.jpg"
#define PLAY_IMAGE_OFF	"ms0:/PSP/GAME/PSPRadio/THEME/ON_OFF_States/Play_OFF.jpg"
#define PAUSE_IMAGE_ON 	"ms0:/PSP/GAME/PSPRadio/THEME/ON_OFF_States/Pause_ON.jpg"
#define PAUSE_IMAGE_OFF	"ms0:/PSP/GAME/PSPRadio/THEME/ON_OFF_States/Pause_OFF.jpg"
#define LOAD_IMAGE_ON 	"ms0:/PSP/GAME/PSPRadio/THEME/ON_OFF_States/Load_ON.jpg"
#define LOAD_IMAGE_OFF	"ms0:/PSP/GAME/PSPRadio/THEME/ON_OFF_States/Load_OFF.jpg"
#define SOUND_IMAGE_ON 	"ms0:/PSP/GAME/PSPRadio/THEME/ON_OFF_States/Sound_ON.jpg"
#define SOUND_IMAGE_OFF	"ms0:/PSP/GAME/PSPRadio/THEME/ON_OFF_States/Sound_OFF.jpg"

CGraphicsUI::CGraphicsUI()
{
	m_bSDLInitialized = false;
	
	m_pImageBase = NULL;
	m_pImageLoad[0] = NULL;
	m_pImageLoad[1] = NULL;
	m_pImagePlay[0] = NULL;
	m_pImagePlay[1] = NULL;
	m_pImagePause[0] = NULL;
	m_pImagePause[1] = NULL;
	m_pImageStop[0] = NULL;
	m_pImageStop[1] = NULL;
	m_pImageSound[0] = NULL;
	m_pImageSound[1] = NULL;
	
	m_pScreen = NULL;
	m_nDepth = -1;
	m_nFlags = SDL_SWSURFACE | SDL_FULLSCREEN;
}

CGraphicsUI::~CGraphicsUI()
{
}

int CGraphicsUI::Initialize()
{
	/** If we have already initialized just return success **/
	if(true == m_bSDLInitialized)
	{
		Log(LOG_ERROR, "Initialize: SDL Already initialized!");
		return -1;
	}
	
	Log(LOG_LOWLEVEL, "Initialize: SDL Initializing");
	
	/** Initialize SDL **/
	if(SDL_Init(SDL_INIT_VIDEO) < 0) 
	{	
		Log(LOG_ERROR, "Initialize: SDL_Init error : %s", SDL_GetError());
		return -1;
	}
	
	Log(LOG_LOWLEVEL, "Initialize: SDL Initialized");
	
 	m_bSDLInitialized = true;
 	
	Log(LOG_LOWLEVEL, "Initialize: Loading images"); 	
		
	/** Initialize Base Image **/
	m_pImageBase 		= LoadImage(BASE_IMAGE);
	m_pImageLoad[0] 	= LoadImage(LOAD_IMAGE_OFF);
	m_pImageLoad[1] 	= LoadImage(LOAD_IMAGE_ON);
	m_pImagePlay[0] 	= LoadImage(PLAY_IMAGE_OFF);
	m_pImagePlay[1] 	= LoadImage(PLAY_IMAGE_ON);
	m_pImagePause[0] 	= LoadImage(PAUSE_IMAGE_OFF);
	m_pImagePause[1] 	= LoadImage(PAUSE_IMAGE_ON);
	m_pImageStop[0] 	= LoadImage(STOP_IMAGE_OFF);
	m_pImageStop[1] 	= LoadImage(STOP_IMAGE_ON);
	m_pImageSound[0] 	= LoadImage(SOUND_IMAGE_OFF);
	m_pImageSound[1] 	= LoadImage(SOUND_IMAGE_ON);
	
	Log(LOG_LOWLEVEL, "Initialize: Images loaded"); 	
	
	/** Make sure image was sucessfully loaded **/
	if((NULL == m_pImageBase) 			||
		(NULL == m_pImageLoad[0]) 		||
		(NULL == m_pImageLoad[1]) 		||
		(NULL == m_pImagePlay[0]) 		||
		(NULL == m_pImagePlay[1]) 		||
		(NULL == m_pImagePause[0]) 		||
		(NULL == m_pImagePause[1]) 		||
		(NULL == m_pImageStop[0]) 		||
		(NULL == m_pImageStop[1]) 		||
		(NULL == m_pImageSound[0]) 		||
		(NULL == m_pImageSound[1]))
	{
		Log(LOG_ERROR, "Initialize: error not all images loaded");
		return -1;
	}
	
	Log(LOG_LOWLEVEL, "Initialize: Setting video mode"); 	
	
	/** Create a display for the image **/
	m_nDepth = SDL_VideoModeOK(m_pImageBase->w, m_pImageBase->h, 32, m_nFlags);
	
	Log(LOG_LOWLEVEL, "Initialize: Setting video mode completed depth %d", m_nDepth); 	
	
	/** Use the deepest native mode, except that we emulate 32bpp 
		for viewing non-indexed images on 8bpp screens **/
	if(m_nDepth == 0) 
	{
		if(m_pImageBase->format->BytesPerPixel > 1)
		{
			m_nDepth = 32;
		} 
		else 
		{
			m_nDepth = 8;
		}
	} 
	else if((m_pImageBase->format->BytesPerPixel > 1) && (m_nDepth == 8))
	{
		m_nDepth = 32;
	}
	
	Log(LOG_LOWLEVEL, "Initialize: New Depth is %d", m_nDepth); 		
		
	if(m_nDepth == 8)
	{
		m_nFlags |= SDL_HWPALETTE;
	}
		
	Log(LOG_LOWLEVEL, "Initialize: Setting video mode");
 	
 	m_pScreen = SDL_SetVideoMode(m_pImageBase->w, m_pImageBase->h, m_nDepth, m_nFlags);
	
	Log(LOG_LOWLEVEL, "Initialize: Setting video mode completed");
		
	if(m_pScreen == NULL) 
	{
		Log(LOG_ERROR, "Initialize: SDL_SetVideoMode error %dx%dx%d video mode: %s\n",
			m_pImageBase->w, m_pImageBase->h, m_nDepth, SDL_GetError());
	}

	/** Set the palette, if one exists **/
	if(m_pImageBase->format->palette) 
	{
		Log(LOG_LOWLEVEL, "Initialize: Setting palette");
		SDL_SetColors(m_pScreen, m_pImageBase->format->palette->colors, 0, m_pImageBase->format->palette->ncolors);
		Log(LOG_LOWLEVEL, "Initialize: Setting palette completed");
	}
	
	/** Display the image **/
	SDL_BlitSurface(m_pImageBase, NULL, m_pScreen, NULL);
	SDL_UpdateRect(m_pScreen, 0, 0, 0, 0);
		
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
	Log(LOG_INFO, "UnloadImage: unloading 0x08X", *ppImage);
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
	UnLoadImage(&m_pImageLoad[0]);
	UnLoadImage(&m_pImageLoad[1]);
	UnLoadImage(&m_pImagePlay[0]);
	UnLoadImage(&m_pImagePlay[1]);
	UnLoadImage(&m_pImagePause[0]);
	UnLoadImage(&m_pImagePause[1]);
	UnLoadImage(&m_pImageStop[0]);
	UnLoadImage(&m_pImageStop[1]);
	UnLoadImage(&m_pImageSound[0]);
	UnLoadImage(&m_pImageSound[1]);
	Log(LOG_INFO, "Terminate: images all unloaded");

	/** If we are initialized do some cleaning up **/
	if(true == m_bSDLInitialized)
	{
		Log(LOG_INFO, "Terminate: cleaning up SDL");
		/** Shut down SDL **/
		SDL_Quit();
		
		/** Reset our initialized flag **/
		m_bSDLInitialized = false;
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

void CGraphicsUI::SetPlayButton(uibuttonstate_enum state)
{
	SDL_Rect destRect;
	destRect.x = 16;
	destRect.y = 252;
			
	SDL_BlitSurface(m_pImagePlay[state], NULL, m_pScreen, &destRect);
	SDL_UpdateRect(m_pScreen, 0, 0, 0, 0);			
}

void CGraphicsUI::SetPauseButton(uibuttonstate_enum state)
{
	SDL_Rect destRect;
	destRect.x = 34;
	destRect.y = 252;
			
	SDL_BlitSurface(m_pImagePause[state], NULL, m_pScreen, &destRect);
	SDL_UpdateRect(m_pScreen, 0, 0, 0, 0);			
}

void CGraphicsUI::SetStopButton(uibuttonstate_enum state)
{
	SDL_Rect destRect;
	destRect.x = 52;
	destRect.y = 252;
			
	SDL_BlitSurface(m_pImageStop[state], NULL, m_pScreen, &destRect);
	SDL_UpdateRect(m_pScreen, 0, 0, 0, 0);			
}

int CGraphicsUI::DisplayErrorMessage(char *strMsg)
{
	return 0;
}

int CGraphicsUI::DisplayPlayBuffer(int a, int b)
{
	return 0;
}

int CGraphicsUI::DisplayDecodeBuffer(int a, int b)
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

int CGraphicsUI::OnConnectionProgress()
{
	return 0;
}
