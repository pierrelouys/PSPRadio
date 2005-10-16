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
#include <PSPSound.h>
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

	Log(LOG_VERYLOW, "Initialize: Theme Initializing");
	if(FALSE == InitializeTheme(szThemeFile, szThemePath))
	{
		Log(LOG_ERROR, "Initialize: error initializing Theme");
		return -1;
	}		
	Log(LOG_VERYLOW, "Initialize: Theme Initialized");

	
	Log(LOG_VERYLOW, "Initialize: SDL Initializing");		
	if(FALSE == InitializeSDL())
	{
		Log(LOG_ERROR, "Initialize: error initializing SDL");
		return -1;
	}	
	Log(LOG_VERYLOW, "Initialize: SDL Initialized");	
	
	Log(LOG_VERYLOW, "Initialize: Images Initializing");
	if(FALSE == InitializeImages())
	{
		Log(LOG_ERROR, "Initialize: error initializing images");
		return -1;
	}	
	Log(LOG_VERYLOW, "Initialize: Images Initialied");
	
	SetBaseImage();
		
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
	DisplayWord(&m_posItemNetworkString, "Enabling Network", true);
	return 0;
}

int CGraphicsUI::DisplayMessage_NetworkSelection(int iProfileID, char *strProfileName)
{
	char szTmp[256];
	sprintf(szTmp, "Press TRIANGLE for Network Profile: %d '%s'", iProfileID, strProfileName);
	DisplayWord(&m_posItemNetworkString, szTmp, true);
	return 0;
}

int CGraphicsUI::DisplayMessage_DisablingNetwork()
{
	DisplayWord(&m_posItemNetworkString, "Disabling Network", true);
	return 0;
}

int CGraphicsUI::DisplayMessage_NetworkReady(char *strIP)
{
	DisplayWord(&m_posItemNetworkString, strIP, true);
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
	DisplayWord(&m_posItemErrorString, strMsg, true);
	return 0;
}

int CGraphicsUI::DisplayBufferPercentage(int iPercentage)
{
	char szTmp[50];
	sprintf(szTmp, "Buffer: %03d%%", iPercentage);
	DisplayWord(&m_posItemBufferString, szTmp, true);
	return 0;
}

int CGraphicsUI::OnNewStreamStarted()
{
	DisplayWord(&m_posItemStreamString, "Stream Starting", true);
	return 0;
}

int CGraphicsUI::OnStreamOpening()
{
	DisplayWord(&m_posItemStreamString, "Stream Opening", true);
	return 0;
}

int CGraphicsUI::OnStreamOpeningError()
{
	DisplayWord(&m_posItemStreamString, "Stream Error", true);
	return 0;
}

int CGraphicsUI::OnStreamOpeningSuccess()
{
	DisplayWord(&m_posItemStreamString, "Stream Opened", true);
	SetButton(m_themeItemLoad, UIBUTTONSTATE_OFF);
	return 0;
}

int CGraphicsUI::OnVBlank()
{
	return 0;
}

int CGraphicsUI::OnNewSongData(CPlayList::songmetadata *pData)
{
	char szTmp[50];
	
	DisplayWord(&m_posItemFileNameString, pData->strFileName, true);	
	DisplayWord(&m_posItemFileTitleString, pData->strFileTitle, true);	
	DisplayWord(&m_posItemURLString, pData->strURL, true);	
	DisplayWord(&m_posItemSongTitleString, pData->songTitle, true);
	DisplayWord(&m_posItemSongAuthorString, pData->songAuthor, true);
	
	sprintf(szTmp, "Length: %d", pData->iLength);
	DisplayWord(&m_posItemLengthString, szTmp, true);
	
	sprintf(szTmp, "Sample Rate: %d", pData->SampleRate);
	DisplayWord(&m_posItemSampleRateString, szTmp, true);
	
	sprintf(szTmp, "Bit Rate: %d", pData->BitRate);
	DisplayWord(&m_posItemBitRateString, szTmp, true);
	
	DisplayWord(&m_posItemMPEGLayerString, pData->strMPEGLayer, true);
	
	return 0;
}

int CGraphicsUI::DisplayPLList(CDirList *plList)
{
	if((false == m_posItemPlayListArea.m_bEnabled) ||
		(false == m_posItemPlayListAreaSel.m_bEnabled))
	{
		return 0;
	}
	
	ClearLine(&m_posItemPlayListArea);
	
	list<CDirList::directorydata> dataList = *(plList->GetList());
	list<CDirList::directorydata>::iterator dataIter = *(plList->GetCurrentElementIterator());
	
	int nItemCount = m_posItemPlayListArea.m_pointSize.y / m_posItemPlayListAreaSel.m_pointSize.y;
	int nItemMid = (nItemCount) / 2;

	CGraphicsUIPosItem posTemp;	
	
	posTemp.m_pointDst.x = m_posItemPlayListArea.m_pointDst.x;
	posTemp.m_pointDst.y = m_posItemPlayListArea.m_pointDst.y + (nItemMid * m_posItemPlayListAreaSel.m_pointSize.y);
	posTemp.m_pointSize.x = m_posItemPlayListAreaSel.m_pointSize.x;
	posTemp.m_pointSize.y = m_posItemPlayListAreaSel.m_pointSize.y;
	
	DisplayWord(&posTemp, dataIter->strURI, true);
	SetButton(m_posItemPlayListAreaSel, posTemp);
	
	return 0;
}

int CGraphicsUI::DisplayPLEntries(CPlayList *PlayList)
{
	if((false == m_posItemPlayListItemArea.m_bEnabled) ||
		(false == m_posItemPlayListItemAreaSel.m_bEnabled))
	{
		return 0;
	}

	ClearLine(&m_posItemPlayListItemArea);
		
	list<CPlayList::songmetadata> dataList = *(PlayList->GetList());
	list<CPlayList::songmetadata>::iterator dataIter = *(PlayList->GetCurrentElementIterator());
	
	int nItemCount = m_posItemPlayListItemArea.m_pointSize.y / m_posItemPlayListItemAreaSel.m_pointSize.y;
	int nItemMid = (nItemCount) / 2;

	CGraphicsUIPosItem posTemp;	
	
	posTemp.m_pointDst.x = m_posItemPlayListItemArea.m_pointDst.x;
	posTemp.m_pointDst.y = m_posItemPlayListItemArea.m_pointDst.y + (nItemMid * m_posItemPlayListItemAreaSel.m_pointSize.y);
	posTemp.m_pointSize.x = m_posItemPlayListItemAreaSel.m_pointSize.x;
	posTemp.m_pointSize.y = m_posItemPlayListItemAreaSel.m_pointSize.y;
		
	if(strlen(dataIter->strFileTitle))
	{
		DisplayWord(&posTemp, dataIter->strFileTitle, true);
	}
	else
	{
		DisplayWord(&posTemp, dataIter->strFileName, true);
	}	
	
	SetButton(m_posItemPlayListItemAreaSel, posTemp);
	
	return 0;
}

int CGraphicsUI::OnConnectionProgress()
{
	SetButton(m_themeItemLoad, UIBUTTONSTATE_ON);
	return 0;
}

void CGraphicsUI::SetBaseImage(void)
{
	SetButton(m_themeItemBackground, UIBUTTONSTATE_ON);
}

void CGraphicsUI::SetPlayButton(uibuttonstate_enum state)
{
	SetButton(m_themeItemPlay, state);
}

void CGraphicsUI::SetPauseButton(uibuttonstate_enum state)
{
	SetButton(m_themeItemPause, state);
}

void CGraphicsUI::SetStopButton(uibuttonstate_enum state)
{
	SetButton(m_themeItemStop, state);
}

void CGraphicsUI::SetSoundButton(uibuttonstate_enum state)
{
	SetButton(m_themeItemSound, state);
}

void CGraphicsUI::SetButton(CGraphicsUIThemeItem themeItem, uibuttonstate_enum state)
{
	SDL_Rect src = 	{ 
						themeItem.GetSrc(state).x,
						themeItem.GetSrc(state).y,
						themeItem.m_pointSize.x,
						themeItem.m_pointSize.y
					};
					
	SDL_Rect dst = 	{ 
						themeItem.m_pointDst.x,
						themeItem.m_pointDst.y,
					};
		
	SDL_BlitSurface(m_pImageBase, &src, m_pScreen, &dst);
}

void CGraphicsUI::SetButton(CGraphicsUIPosItem posSrc, CGraphicsUIPosItem posDst)
{
	SDL_Rect src = 	{ 
						posSrc.m_pointDst.x,
						posSrc.m_pointDst.y,
						posSrc.m_pointSize.x,
						posSrc.m_pointSize.y
					};
					
	SDL_Rect dst = 	{ 
						posDst.m_pointDst.x,
						posDst.m_pointDst.y,
						posDst.m_pointSize.x,
						posDst.m_pointSize.y
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
	Log(LOG_VERYLOW, "InitializeTheme: getting image path");
	if(0 != m_theme.GetImagePath(szBaseImage, sizeof(szBaseImage)))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme image path");
		return FALSE;
	}	
	
	sprintf(m_szThemeImagePath, "%s%s", szThemePath, szBaseImage);
	Log(LOG_VERYLOW, "InitializeTheme: base image = %s", m_szThemeImagePath);
	
	/** Get the theme items */
	Log(LOG_VERYLOW, "InitializeTheme: getting background");
	if(0 != m_theme.GetItem("background", &m_themeItemBackground))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme background");
		return FALSE;
	}
	
	Log(LOG_VERYLOW, "InitializeTheme: getting play");
	if(0 != m_theme.GetItem("play", &m_themeItemPlay))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme play");
		return FALSE;
	}
	
	Log(LOG_VERYLOW, "InitializeTheme: getting pause");
	if(0 != m_theme.GetItem("pause", &m_themeItemPause))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme pause");
		return FALSE;
	}
	
	Log(LOG_VERYLOW, "InitializeTheme: getting stop");
	if(0 != m_theme.GetItem("stop", &m_themeItemStop))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme stop");
		return FALSE;
	}
	
	Log(LOG_VERYLOW, "InitializeTheme: getting load");
	if(0 != m_theme.GetItem("load", &m_themeItemLoad))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme load");
		return FALSE;
	}
	
	Log(LOG_VERYLOW, "InitializeTheme: getting sound");
	if(0 != m_theme.GetItem("sound", &m_themeItemSound))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme sound");
		return FALSE;
	}
	
	Log(LOG_VERYLOW, "InitializeTheme: getting volume");
	if(0 != m_theme.GetItem("volume", &m_themeItemVolume))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme volume");
		return FALSE;
	}
	
	Log(LOG_VERYLOW, "InitializeTheme: getting letters adn numbers");
	if(0 != m_theme.GetLettersAndNumbers("letters", "numbers", &m_themeItemABC123))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme letters and numbers");
		return FALSE;
	}
	
	/** Get the string positions from ini file. If the value is not found we */
	/** will just disable that string item. */
	Log(LOG_VERYLOW, "InitializeTheme: getting string pos stringpos:filename");
	if(0 != m_theme.GetPosItem("stringpos:filename", &m_posItemFileNameString))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos filename disabling");
		m_posItemFileNameString.m_bEnabled = false;
	}
	
	Log(LOG_VERYLOW, "InitializeTheme: getting string pos stringpos:filetitle");
	if(0 != m_theme.GetPosItem("stringpos:filetitle", &m_posItemFileTitleString))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos filetitle disabling");
		m_posItemFileTitleString.m_bEnabled = false;
	}
	
	Log(LOG_VERYLOW, "InitializeTheme: getting string pos stringpos:uri");
	if(0 != m_theme.GetPosItem("stringpos:uri", &m_posItemURLString))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos url disabling");
		m_posItemURLString.m_bEnabled = false;
	}
	
	Log(LOG_VERYLOW, "InitializeTheme: getting string pos stringpos:songtitle");
	if(0 != m_theme.GetPosItem("stringpos:songtitle", &m_posItemSongTitleString))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos songtitle disabling");
		m_posItemSongTitleString.m_bEnabled = false;
	}
	
	Log(LOG_VERYLOW, "InitializeTheme: getting string pos stringpos:songauthor");
	if(0 != m_theme.GetPosItem("stringpos:songauthor", &m_posItemSongAuthorString))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos songauthor disabling");
		m_posItemSongAuthorString.m_bEnabled = false;
	}
	
	Log(LOG_VERYLOW, "InitializeTheme: getting string pos stringpos:length");
	if(0 != m_theme.GetPosItem("stringpos:length", &m_posItemLengthString))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos length disabling");
		m_posItemLengthString.m_bEnabled = false;
	}
	
	Log(LOG_VERYLOW, "InitializeTheme: getting string pos stringpos:samplerate");
	if(0 != m_theme.GetPosItem("stringpos:samplerate", &m_posItemSampleRateString))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos samplerate disabling");
		m_posItemSampleRateString.m_bEnabled = false;
	}

	Log(LOG_VERYLOW, "InitializeTheme: getting string pos stringpos:bitrate");
	if(0 != m_theme.GetPosItem("stringpos:bitrate", &m_posItemBitRateString))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos bitrate disabling");
		m_posItemBitRateString.m_bEnabled = false;
	}
	
	Log(LOG_VERYLOW, "InitializeTheme: getting string pos stringpos:mpeglayer");
	if(0 != m_theme.GetPosItem("stringpos:mpeglayer", &m_posItemMPEGLayerString))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos mpeglayer disabling");
		m_posItemMPEGLayerString.m_bEnabled = false;
	}
	
	Log(LOG_VERYLOW, "InitializeTheme: getting string pos stringpos:error");
	if(0 != m_theme.GetPosItem("stringpos:error", &m_posItemErrorString))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos error disabling");
		m_posItemErrorString.m_bEnabled = false;
	}

	Log(LOG_VERYLOW, "InitializeTheme: getting string pos stringpos:stream");
	if(0 != m_theme.GetPosItem("stringpos:stream", &m_posItemStreamString))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos stream disabling");
		m_posItemStreamString.m_bEnabled = false;
	}

	Log(LOG_VERYLOW, "InitializeTheme: getting string pos stringpos:network");
	if(0 != m_theme.GetPosItem("stringpos:network", &m_posItemNetworkString))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos network disabling");
		m_posItemNetworkString.m_bEnabled = false;
	}
	
	Log(LOG_VERYLOW, "InitializeTheme: getting string pos stringpos:buffer");
	if(0 != m_theme.GetPosItem("stringpos:buffer", &m_posItemBufferString))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos buffer disabling");
		m_posItemBufferString.m_bEnabled = false;
	}
	
	Log(LOG_VERYLOW, "InitializeTheme: getting string pos itempos:playlistarea");
	if(0 != m_theme.GetPosItem("itempos:playlistarea", &m_posItemPlayListArea))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos playlistarea disabling");
		m_posItemPlayListArea.m_bEnabled = false;
	}
	
	Log(LOG_VERYLOW, "InitializeTheme: getting string pos itempos:playlistareasel");
	if(0 != m_theme.GetPosItem("itempos:playlistareasel", &m_posItemPlayListAreaSel))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos playlistareasel disabling");
		m_posItemPlayListAreaSel.m_bEnabled = false;
	}
	
	Log(LOG_VERYLOW, "InitializeTheme: getting string pos itempos:playlistitemarea");
	if(0 != m_theme.GetPosItem("itempos:playlistitemarea", &m_posItemPlayListItemArea))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos playlistitemarea disabling");
		m_posItemPlayListItemArea.m_bEnabled = false;
	}
	
	Log(LOG_VERYLOW, "InitializeTheme: getting string pos itempos:playlistitemareasel");
	if(0 != m_theme.GetPosItem("itempos:playlistitemareasel", &m_posItemPlayListItemAreaSel))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos playlistitemareasel disabling");
		m_posItemPlayListItemAreaSel.m_bEnabled = false;
	}

	return TRUE;
}

bool CGraphicsUI::InitializeSDL()
{
	Log(LOG_VERYLOW, "InitializeSDL: SDL Initializing with SDL_INIT_VIDEO");
	if(SDL_Init(SDL_INIT_VIDEO) < 0) 
	{	
		Log(LOG_ERROR, "InitializeSDL: SDL_Init error : %s", SDL_GetError());
		return FALSE;
	}	
	Log(LOG_VERYLOW, "InitializeSDL: SDL Initialized with SDL_INIT_VIDEO");	
	
	Log(LOG_VERYLOW, "InitializeSDL: disabling cursor"); 	
	SDL_ShowCursor(SDL_DISABLE);
	Log(LOG_VERYLOW, "InitializeSDL: disabled cursor"); 	
	
	Log(LOG_VERYLOW, "InitializeSDL: Checking video mode"); 	
	m_nDepth = SDL_VideoModeOK(PSP_RES_WIDTH, PSP_RES_HEIGHT, 32, m_nFlags);
	Log(LOG_VERYLOW, "InitializeSDL: Checking video mode completed depth %d", m_nDepth); 	
		
	Log(LOG_VERYLOW, "InitializeSDL: Setting video mode"); 	
 	if(NULL == (m_pScreen = SDL_SetVideoMode(PSP_RES_WIDTH, 
 												PSP_RES_HEIGHT, 
 												m_nDepth, 
 												m_nFlags)))
 	{
		Log(LOG_ERROR, "InitializeSDL: SDL_SetVideoMode error %dx%dx%d video mode: %s\n",
			PSP_RES_WIDTH, PSP_RES_HEIGHT, m_nDepth, SDL_GetError());
		return FALSE;
 	}
	Log(LOG_VERYLOW, "InitializeSDL: Setting video mode completed");	
			
	return TRUE;
}

bool CGraphicsUI::InitializeImages()
{
	Log(LOG_VERYLOW, "InitializeImages: Loading base image"); 		
	if(NULL == (m_pImageBase = LoadImage(m_szThemeImagePath)))
	{
		Log(LOG_ERROR, "InitializeImages: error loading base image");
		return FALSE;
	}	
	Log(LOG_VERYLOW, "InitializeImages: Loaded base image"); 		
	
	Log(LOG_VERYLOW, "InitializeSDL: Setting transparency");
	SDL_SetColorKey(m_pImageBase, SDL_SRCCOLORKEY, SDL_MapRGB(m_pImageBase->format, 255, 0, 255)); 
	Log(LOG_VERYLOW, "InitializeSDL: Setting completed");

	return TRUE;
}

void CGraphicsUI::DisplayWord(CGraphicsUIPosItem *pPosItem, char *szWord, bool bCenter)
{
	if(NULL == pPosItem)
	{
		Log(LOG_ERROR, "DisplayWord: error pPosItem is NULL");
		return;
	}
	
	if(false == pPosItem->m_bEnabled)
	{
		return;
	}
	
	int nStringLen = strlen(szWord);
	int nFontWidth = m_themeItemABC123.m_pointSize.x;
	int nFontHeight = m_themeItemABC123.m_pointSize.y;
	
	int nCurrentXPos = pPosItem->m_pointDst.x; 
	int nCurrentYPos = pPosItem->m_pointDst.y + 
						(pPosItem->m_pointSize.y / 2) -
						(nFontHeight / 2);
						
	/** If our word is longer than the position lets truncate it for now */
	/** to prevent us from somping on other strings */
	/** Eventually I want to implement a scrolling text option */	
	if((nStringLen * nFontWidth) > pPosItem->m_pointSize.x)
	{
		nStringLen = pPosItem->m_pointSize.x / nFontWidth;
	}
		
	ClearLine(pPosItem);
		
	if(true == bCenter)
	{
		int nStringWidth = nStringLen * nFontWidth;
		
		nCurrentXPos = pPosItem->m_pointDst.x + 
						(pPosItem->m_pointSize.x / 2) - 
						(nStringWidth / 2);
	}	
	
	for(int x = 0; x != nStringLen; x++)
	{
		int index = m_themeItemABC123.GetIndexFromKey(toupper(szWord[x]));
		
		SDL_Rect src = 	{ 
							m_themeItemABC123.GetSrc(index).x,
							m_themeItemABC123.GetSrc(index).y,
							m_themeItemABC123.m_pointSize.x,
							m_themeItemABC123.m_pointSize.y
						};
						
		SDL_Rect dst = 	{ 
							nCurrentXPos,
							nCurrentYPos,
						};
			
		SDL_BlitSurface(m_pImageBase, &src, m_pScreen, &dst);
		
		nCurrentXPos += nFontWidth;		
	}
}

void CGraphicsUI::ClearLine(CGraphicsUIPosItem *pPosItem)
{
	if(NULL == pPosItem)
	{
		Log(LOG_ERROR, "ClearLine: error pPosItem is NULL");
		return;
	}
	
	if(false == pPosItem->m_bEnabled)
	{
		return;
	}	
	
	SDL_Rect src = 	{ 
						pPosItem->m_pointDst.x,
						pPosItem->m_pointDst.y,
						pPosItem->m_pointSize.x,
						pPosItem->m_pointSize.y
					};
						
	SDL_Rect dst = 	{ 
						pPosItem->m_pointDst.x,
						pPosItem->m_pointDst.y,
						pPosItem->m_pointSize.x,
						pPosItem->m_pointSize.y
					};
			
	SDL_BlitSurface(m_pImageBase, &src, m_pScreen, &dst);		
}