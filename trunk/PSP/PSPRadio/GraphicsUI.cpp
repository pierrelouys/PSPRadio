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

#define BASE_IMAGE "ms0:/PSP/GAME/PSPRadio/THEME/PSPRadio_sansAll.jpg"

CGraphicsUI::CGraphicsUI()
{
	m_bSDLInitialized = false;
	m_pImageBase = NULL;
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
 	
	Log(LOG_LOWLEVEL, "Initialize: Loading image %s", BASE_IMAGE); 	
		
	/** Initialize Base Image **/
	m_pImageBase = IMG_Load(BASE_IMAGE);
	
	Log(LOG_LOWLEVEL, "Initialize: Image loaded %s", BASE_IMAGE); 	
	
	/** Make sure image was sucessfully loaded **/
	if(NULL == m_pImageBase)
	{
		Log(LOG_ERROR, "Initialize: IMG_Load(%s) error : %s", BASE_IMAGE, SDL_GetError());
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
	
	/* Display the image */
	SDL_BlitSurface(m_pImageBase, NULL, m_pScreen, NULL);
	SDL_UpdateRect(m_pScreen, 0, 0, 0, 0);
	
	return 0;
}

void CGraphicsUI::Terminate()
{
	/** If we are initialized do some cleaning up **/
	if(true == m_bSDLInitialized)
	{
		/** Cleanup Images **/
		if(NULL != m_pImageBase)
		{
			SDL_FreeSurface(m_pImageBase);
			m_pImageBase = NULL;
		}
	
		/** Shut down SDL **/
		SDL_Quit();
		
		/** Reset our initialized flag **/
		m_bSDLInitialized = false;
	}
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
			break;
		
		case CPSPSound::PLAY:
			break;
		
		case CPSPSound::PAUSE:
			break;
	}
	
	return 0;
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

int CGraphicsUI::OnStreamOpening(char *StreamName)
{
	return 0;
}

int CGraphicsUI::OnStreamOpeningError(char *StreamName)
{
	return 0;
}

int CGraphicsUI::OnStreamOpeningSuccess(char *StreamName)
{
	return 0;
}

int CGraphicsUI::OnVBlank()
{
	return 0;
}

int CGraphicsUI::DisplayMetadata(char *strTitle, char *strURL)
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
