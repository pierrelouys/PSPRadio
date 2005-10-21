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
	memset(m_pPlayListText, 0, sizeof(m_pPlayListText));
	m_nPlayListTextCount = -1;
	memset(m_pPlayListItemText, 0, sizeof(m_pPlayListItemText));
	m_nPlayListItemTextCount = -1;
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

	if(FALSE == InitializeTheme(szThemeFile, szThemePath))
	{
		Log(LOG_ERROR, "Initialize: error initializing Theme");
		return -1;
	}		
	
	if(FALSE == InitializeSDL())
	{
		Log(LOG_ERROR, "Initialize: error initializing SDL");
		return -1;
	}	
	
	if(FALSE == InitializeImages())
	{
		Log(LOG_ERROR, "Initialize: error initializing images");
		return -1;
	}	
	
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
	UnLoadImage(&m_pImageBase);
	UnLoadImage(&m_pScreen);
	
	for(int x = 0; x != 100; x++)
	{
		UnLoadImage(&m_pPlayListText[x]);
		UnLoadImage(&m_pPlayListItemText[x]);
	}	
		
	/** If we are initialized do some cleaning up **/
	if(0 != SDL_WasInit(SDL_INIT_VIDEO))
	{
		/** Shut down SDL **/
		SDL_Quit();
	}	
}

void CGraphicsUI::Initialize_Screen(CScreenHandler::Screen screen)
{
//	static SDL_Surface *pSurfaceSave = NULL;
	
	switch (screen)
	{
		case CScreenHandler::PSPRADIO_SCREEN_SHOUTCAST_BROWSER:
		case CScreenHandler::PSPRADIO_SCREEN_PLAYLIST:
			SetButton(m_themeItemBackground, UIBUTTONSTATE_ON);
			break;
			
		case CScreenHandler::PSPRADIO_SCREEN_OPTIONS:
			// TODO: Copy current screen and repaint when we go back
			// to playlist mode
			SetButton(m_themeItemSettings, UIBUTTONSTATE_ON);
			break;
	}
}

void CGraphicsUI::UpdateOptionsScreen(list<CScreenHandler::Options> &OptionsList, 
								list<CScreenHandler::Options>::iterator &CurrentOptionIterator)
{
	list<CScreenHandler::Options>::iterator OptionIterator;
	CScreenHandler::Options	Option;
	
	
	if (OptionsList.size() > 0)
	{
		int nLineNumber = 0;
		for (OptionIterator = OptionsList.begin() ; OptionIterator != OptionsList.end() ; OptionIterator++)
		{
			char szTemp[100];
			
			Option = (*OptionIterator);

			if (OptionIterator == CurrentOptionIterator)
			{
				sprintf(szTemp, "[%s]", Option.strName); 
			}
			else
			{
				sprintf(szTemp, " %s ", Option.strName); 
			}
									
			PrintOption(nLineNumber, szTemp, Option.strStates, Option.iNumberOfStates, Option.iSelectedState, 
						Option.iActiveState);
			
			nLineNumber++;
		}
	}
}

void CGraphicsUI::PrintOption(int nLineNumber, char *strName, char *strStates[], int iNumberOfStates, int iSelectedState,
						  int iActiveState)
{	
	if (iNumberOfStates > 0)
	{
		char szTemp[100];
		
		SDL_Surface *pSurface = DisplayWord(strName);
		
		CGraphicsUIPosItem posDst;	
			
		posDst.m_pointDst.x = m_posItemSettingsArea.m_pointDst.x;
		posDst.m_pointDst.y = m_posItemSettingsArea.m_pointDst.y + ((m_themeItemABC123.m_pointSize.y * 2) * nLineNumber);
		posDst.m_pointSize.x = m_posItemSettingsArea.m_pointSize.x;
		posDst.m_pointSize.y = m_themeItemABC123.m_pointSize.y * 2;
			
		ResetImageArea(&posDst, m_pImageBase, m_pScreen);		
		CopySurface(pSurface, m_pScreen, &posDst, false);
		SDL_FreeSurface(pSurface);					
		
		for (int iStates = 0; iStates < iNumberOfStates ; iStates++)
		{
			posDst.m_pointDst.x = m_posItemSettingsArea.m_pointDst.x + 200 + (100*iStates+1);
			ResetImageArea(&posDst, m_pImageBase, m_pScreen);		
			
			if ((iStates+1 == iActiveState) && (iStates+1 == iSelectedState))
			{
				sprintf(szTemp, "[*%s*]", strStates[iStates]);
				pSurface = DisplayWord(szTemp);
			}
			else if (iStates+1 == iActiveState)
			{
				sprintf(szTemp, " *%s* ", strStates[iStates]);
				pSurface = DisplayWord(szTemp);
			}
			else if (iStates+1 == iSelectedState) /** 1-based */
			{
				sprintf(szTemp, "[ %s ]", strStates[iStates]);
				pSurface = DisplayWord(szTemp);
			}
			else
			{
				sprintf(szTemp, "  %s  ", strStates[iStates]);
				pSurface = DisplayWord(szTemp);
			}
			
	
			CopySurface(pSurface, m_pScreen, &posDst, false);
			SDL_FreeSurface(pSurface);					
		}
	}	
}


int CGraphicsUI::SetTitle(char *strTitle)
{
	return 0;
}

int CGraphicsUI::DisplayMessage_EnablingNetwork()
{
	ResetImageArea(&m_posItemNetworkString, m_pImageBase, m_pScreen);
	DisplayWord(&m_posItemNetworkString, "Enabling Network", true);
	return 0;
}

int CGraphicsUI::DisplayMessage_NetworkSelection(int iProfileID, char *strProfileName)
{
	char szTmp[256];
	ResetImageArea(&m_posItemNetworkString, m_pImageBase, m_pScreen);
	sprintf(szTmp, "Press TRIANGLE for Network Profile: %d '%s'", iProfileID, strProfileName);
	DisplayWord(&m_posItemNetworkString, szTmp, true);
	return 0;
}

int CGraphicsUI::DisplayMessage_DisablingNetwork()
{
	ResetImageArea(&m_posItemNetworkString, m_pImageBase, m_pScreen);
	DisplayWord(&m_posItemNetworkString, "Disabling Network", true);
	return 0;
}

int CGraphicsUI::DisplayMessage_NetworkReady(char *strIP)
{
	ResetImageArea(&m_posItemNetworkString, m_pImageBase, m_pScreen);
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
	ResetImageArea(&m_posItemErrorString, m_pImageBase, m_pScreen);
	DisplayWord(&m_posItemErrorString, strMsg, true);
	return 0;
}

int CGraphicsUI::DisplayBufferPercentage(int iPercentage)
{
	char szTmp[50];
	sprintf(szTmp, "Buffer: %03d%%", iPercentage);
	ResetImageArea(&m_posItemBufferString, m_pImageBase, m_pScreen);
	DisplayWord(&m_posItemBufferString, szTmp, true);
	return 0;
}

int CGraphicsUI::OnNewStreamStarted()
{
	ResetImageArea(&m_posItemStreamString, m_pImageBase, m_pScreen);
	DisplayWord(&m_posItemStreamString, "Stream Starting", true);
	return 0;
}

int CGraphicsUI::OnStreamOpening()
{
	ResetImageArea(&m_posItemStreamString, m_pImageBase, m_pScreen);
	DisplayWord(&m_posItemStreamString, "Stream Opening", true);
	return 0;
}

int CGraphicsUI::OnStreamOpeningError()
{
	ResetImageArea(&m_posItemStreamString, m_pImageBase, m_pScreen);
	DisplayWord(&m_posItemStreamString, "Stream Error", true);
	return 0;
}

int CGraphicsUI::OnStreamOpeningSuccess()
{
	ResetImageArea(&m_posItemStreamString, m_pImageBase, m_pScreen);
	DisplayWord(&m_posItemStreamString, "Stream Opened", true);
	SetButton(m_themeItemLoad, UIBUTTONSTATE_OFF);
	return 0;
}

int CGraphicsUI::OnVBlank()
{
	return 0;
}

int CGraphicsUI::OnNewSongData(CPSPSoundStream::MetaData *pData)
{
	char szTmp[50];
	
	ResetImageArea(&m_posItemFileNameString, m_pImageBase, m_pScreen);
	DisplayWord(&m_posItemFileNameString, pData->strURI, true);	
	
	ResetImageArea(&m_posItemFileTitleString, m_pImageBase, m_pScreen);
	DisplayWord(&m_posItemFileTitleString, pData->strTitle, true);	
	
	ResetImageArea(&m_posItemURLString, m_pImageBase, m_pScreen);
	DisplayWord(&m_posItemURLString, pData->strURL, true);	
	
	ResetImageArea(&m_posItemSongArtistString, m_pImageBase, m_pScreen);
	DisplayWord(&m_posItemSongArtistString, pData->strArtist, true);
	
	sprintf(szTmp, "Length: %d", pData->iLength);
	ResetImageArea(&m_posItemLengthString, m_pImageBase, m_pScreen);
	DisplayWord(&m_posItemLengthString, szTmp, true);
	
	sprintf(szTmp, "Sample Rate: %d", pData->iSampleRate);
	ResetImageArea(&m_posItemSampleRateString, m_pImageBase, m_pScreen);
	DisplayWord(&m_posItemSampleRateString, szTmp, true);
	
	sprintf(szTmp, "Bit Rate: %d", pData->iBitRate);
	ResetImageArea(&m_posItemBitRateString, m_pImageBase, m_pScreen);
	DisplayWord(&m_posItemBitRateString, szTmp, true);
		
	return 0;
}

int CGraphicsUI::DisplayPLList(CDirList *plList)
{
	if((false == m_posItemPlayListArea.m_bEnabled) ||
		(false == m_posItemPlayListAreaSel.m_bEnabled))
	{
		return 0;
	}
	
	int nItemCount = m_posItemPlayListArea.m_pointSize.y / m_posItemPlayListAreaSel.m_pointSize.y;

	// Reset playlist item count so they reinit
	m_nPlayListItemTextCount = -1;
	
	if(-1 == m_nPlayListTextCount)
	{
		m_nPlayListTextCount = plList->GetList()->size();
		
		list<CDirList::directorydata>::iterator dataIter = plList->GetList()->begin();
			
		while(dataIter != plList->GetList()->end())
		{
			char *szTemp = dataIter->strURI;
			int nIndex = dataIter->iItemIndex;
			UnLoadImage(&m_pPlayListText[nIndex]);		
			m_pPlayListText[nIndex] = DisplayWord(basename(szTemp));
			dataIter++;
		}
	}
	
	list<CDirList::directorydata>::iterator dataIter = *plList->GetCurrentElementIterator();
	
		
	for (int i = 0; i < nItemCount/2; i++)
	{
		if (dataIter == plList->GetList()->begin())
			break;
		dataIter--;
	}
	
	
	ResetImageArea(&m_posItemPlayListArea, m_pImageBase, m_pScreen);		
	
	int count = 0;
	
	for (; dataIter != plList->GetList()->end() ; dataIter++)
	{
		if (count > nItemCount-1)
		{
			break;
		}

		CGraphicsUIPosItem posDst;	
			
		posDst.m_pointDst.x = m_posItemPlayListArea.m_pointDst.x;
		posDst.m_pointDst.y = m_posItemPlayListArea.m_pointDst.y + (count * m_posItemPlayListAreaSel.m_pointSize.y);
		posDst.m_pointSize.x = m_posItemPlayListAreaSel.m_pointSize.x;
		posDst.m_pointSize.y = m_posItemPlayListAreaSel.m_pointSize.y;
			
		CopySurface(m_pPlayListText[dataIter->iItemIndex], m_pScreen, &posDst, false);
		
		if(dataIter == *plList->GetCurrentElementIterator())
		{
			SetButton(m_posItemPlayListAreaSel, posDst);
		}
		
		count++;
	}	
		
	return 0;
}

int CGraphicsUI::DisplayPLEntries(CPlayList *PlayList)
{
	if((false == m_posItemPlayListItemArea.m_bEnabled) ||
		(false == m_posItemPlayListItemAreaSel.m_bEnabled))
	{
		return 0;
	}
	
	int nItemCount = m_posItemPlayListItemArea.m_pointSize.y / m_posItemPlayListItemAreaSel.m_pointSize.y;

	if(-1 == m_nPlayListItemTextCount)
	{
		m_nPlayListItemTextCount = PlayList->GetList()->size();
		
		list<CPSPSoundStream::MetaData>::iterator dataIter = PlayList->GetList()->begin();
			
		while(dataIter != PlayList->GetList()->end())
		{
			char *szTemp;
			int nIndex = dataIter->iItemIndex;
			
			UnLoadImage(&m_pPlayListItemText[nIndex]);			
			
			if(0 < strlen(dataIter->strTitle))
			{
				szTemp = dataIter->strTitle;
			}
			else
			{
				szTemp = dataIter->strURI;
			}
			
			m_pPlayListItemText[nIndex] = DisplayWord(szTemp);		
			
			dataIter++;
		}
	}
	
	list<CPSPSoundStream::MetaData>::iterator dataIter = *PlayList->GetCurrentElementIterator();
	
		
	for (int i = 0; i < nItemCount/2; i++)
	{
		if (dataIter == PlayList->GetList()->begin())
			break;
		dataIter--;
	}

	ResetImageArea(&m_posItemPlayListItemArea, m_pImageBase, m_pScreen);		
	
	int count = 0;
	
	for (; dataIter != PlayList->GetList()->end() ; dataIter++)
	{
		if (count > nItemCount-1)
		{
			break;
		}

		CGraphicsUIPosItem posDst;	
			
		posDst.m_pointDst.x = m_posItemPlayListItemArea.m_pointDst.x;
		posDst.m_pointDst.y = m_posItemPlayListItemArea.m_pointDst.y + (count * m_posItemPlayListItemAreaSel.m_pointSize.y);
		posDst.m_pointSize.x = m_posItemPlayListItemAreaSel.m_pointSize.x;
		posDst.m_pointSize.y = m_posItemPlayListItemAreaSel.m_pointSize.y;
			
		CopySurface(m_pPlayListItemText[dataIter->iItemIndex], m_pScreen, &posDst, false);
		
		if(dataIter == *PlayList->GetCurrentElementIterator())
		{
			SetButton(m_posItemPlayListItemAreaSel, posDst);
		}
		
		count++;
	}	
		
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
	if(0 != m_theme.GetImagePath(szBaseImage, sizeof(szBaseImage)))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme image path");
		return FALSE;
	}	
	
	sprintf(m_szThemeImagePath, "%s%s", szThemePath, szBaseImage);
	
	/** Get the theme items */
	if(0 != m_theme.GetItem("background", &m_themeItemBackground))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme background");
		return FALSE;
	}
	
	if(0 != m_theme.GetItem("settings", &m_themeItemSettings))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme settings");
		return FALSE;
	}
	
	
	if(0 != m_theme.GetItem("play", &m_themeItemPlay))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme play");
		return FALSE;
	}
	
	if(0 != m_theme.GetItem("pause", &m_themeItemPause))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme pause");
		return FALSE;
	}
	
	if(0 != m_theme.GetItem("stop", &m_themeItemStop))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme stop");
		return FALSE;
	}
	
	if(0 != m_theme.GetItem("load", &m_themeItemLoad))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme load");
		return FALSE;
	}
	
	if(0 != m_theme.GetItem("sound", &m_themeItemSound))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme sound");
		return FALSE;
	}
	
	if(0 != m_theme.GetItem("volume", &m_themeItemVolume))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme volume");
		return FALSE;
	}
	
	if(0 != m_theme.GetLettersAndNumbers("letters", "numbers", &m_themeItemABC123))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme letters and numbers");
		return FALSE;
	}
	
	/** Get the string positions from ini file. If the value is not found we */
	/** will just disable that string item. */
	if(0 != m_theme.GetPosItem("stringpos:settingarea", &m_posItemSettingsArea))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos settingsarea disabling");
		m_posItemSettingsArea.m_bEnabled = false;
	}
	
	if(0 != m_theme.GetPosItem("stringpos:filename", &m_posItemFileNameString))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos filename disabling");
		m_posItemFileNameString.m_bEnabled = false;
	}
	
	if(0 != m_theme.GetPosItem("stringpos:filetitle", &m_posItemFileTitleString))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos filetitle disabling");
		m_posItemFileTitleString.m_bEnabled = false;
	}
	
	if(0 != m_theme.GetPosItem("stringpos:uri", &m_posItemURLString))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos url disabling");
		m_posItemURLString.m_bEnabled = false;
	}
	
	if(0 != m_theme.GetPosItem("stringpos:songtitle", &m_posItemSongTitleString))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos songtitle disabling");
		m_posItemSongTitleString.m_bEnabled = false;
	}
	
	if(0 != m_theme.GetPosItem("stringpos:songauthor", &m_posItemSongArtistString))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos songauthor disabling");
		m_posItemSongArtistString.m_bEnabled = false;
	}
	
	if(0 != m_theme.GetPosItem("stringpos:length", &m_posItemLengthString))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos length disabling");
		m_posItemLengthString.m_bEnabled = false;
	}
	
	if(0 != m_theme.GetPosItem("stringpos:samplerate", &m_posItemSampleRateString))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos samplerate disabling");
		m_posItemSampleRateString.m_bEnabled = false;
	}

	if(0 != m_theme.GetPosItem("stringpos:bitrate", &m_posItemBitRateString))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos bitrate disabling");
		m_posItemBitRateString.m_bEnabled = false;
	}
	
	if(0 != m_theme.GetPosItem("stringpos:mpeglayer", &m_posItemMPEGLayerString))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos mpeglayer disabling");
		m_posItemMPEGLayerString.m_bEnabled = false;
	}
	
	if(0 != m_theme.GetPosItem("stringpos:error", &m_posItemErrorString))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos error disabling");
		m_posItemErrorString.m_bEnabled = false;
	}

	if(0 != m_theme.GetPosItem("stringpos:stream", &m_posItemStreamString))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos stream disabling");
		m_posItemStreamString.m_bEnabled = false;
	}

	if(0 != m_theme.GetPosItem("stringpos:network", &m_posItemNetworkString))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos network disabling");
		m_posItemNetworkString.m_bEnabled = false;
	}
	
	if(0 != m_theme.GetPosItem("stringpos:buffer", &m_posItemBufferString))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos buffer disabling");
		m_posItemBufferString.m_bEnabled = false;
	}
	
	if(0 != m_theme.GetPosItem("itempos:playlistarea", &m_posItemPlayListArea))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos playlistarea disabling");
		m_posItemPlayListArea.m_bEnabled = false;
	}
	
	if(0 != m_theme.GetPosItem("itempos:playlistareasel", &m_posItemPlayListAreaSel))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos playlistareasel disabling");
		m_posItemPlayListAreaSel.m_bEnabled = false;
	}
	
	if(0 != m_theme.GetPosItem("itempos:playlistitemarea", &m_posItemPlayListItemArea))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos playlistitemarea disabling");
		m_posItemPlayListItemArea.m_bEnabled = false;
	}
	
	if(0 != m_theme.GetPosItem("itempos:playlistitemareasel", &m_posItemPlayListItemAreaSel))
	{
		Log(LOG_ERROR, "InitializeTheme: error getting theme string pos playlistitemareasel disabling");
		m_posItemPlayListItemAreaSel.m_bEnabled = false;
	}

	return TRUE;
}

bool CGraphicsUI::InitializeSDL()
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) 
	{	
		Log(LOG_ERROR, "InitializeSDL: SDL_Init error : %s", SDL_GetError());
		return FALSE;
	}	
	
	SDL_ShowCursor(SDL_DISABLE);
	
	m_nDepth = SDL_VideoModeOK(PSP_RES_WIDTH, PSP_RES_HEIGHT, 32, m_nFlags);
		
 	if(NULL == (m_pScreen = SDL_SetVideoMode(PSP_RES_WIDTH, 
 												PSP_RES_HEIGHT, 
 												m_nDepth, 
 												m_nFlags)))
 	{
		Log(LOG_ERROR, "InitializeSDL: SDL_SetVideoMode error %dx%dx%d video mode: %s\n",
			PSP_RES_WIDTH, PSP_RES_HEIGHT, m_nDepth, SDL_GetError());
		return FALSE;
 	}
			
	return TRUE;
}

bool CGraphicsUI::InitializeImages()
{
	if(NULL == (m_pImageBase = LoadImage(m_szThemeImagePath)))
	{
		Log(LOG_ERROR, "InitializeImages: error loading base image");
		return FALSE;
	}	
	
	SDL_SetColorKey(m_pImageBase, SDL_SRCCOLORKEY, SDL_MapRGB(m_pImageBase->format, 255, 0, 255)); 
	SDL_SetColorKey(m_pScreen, SDL_SRCCOLORKEY, SDL_MapRGB(m_pImageBase->format, 255, 0, 255)); 

	return TRUE;
}

SDL_Surface *CGraphicsUI::DisplayWord(char *szWord)
{
	int nFontWidth = m_themeItemABC123.m_pointSize.x;
	int nFontHeight = m_themeItemABC123.m_pointSize.y + 2;
	int nCurrentXPos = 5; /** JPF added a little offset to start of string */
	int nCurrentYPos = 0;	
	SDL_Surface *pSurface = NULL;
	
	pSurface = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCCOLORKEY,
									nFontWidth * strlen(szWord) + 5, /** JPF added a little offset to start of string */
									nFontHeight,
									m_pImageBase->format->BitsPerPixel,
									m_pImageBase->format->Rmask,
									m_pImageBase->format->Gmask,
									m_pImageBase->format->Bmask,
									m_pImageBase->format->Amask);
																		
	if(NULL == pSurface)
	{
		return NULL;
	}
	
	SDL_SetColorKey(pSurface, SDL_SRCCOLORKEY, SDL_MapRGB(m_pImageBase->format, 255, 0, 255)); 
	SDL_FillRect(pSurface, NULL, SDL_MapRGB(m_pImageBase->format, 255, 0, 255));

	for(size_t x = 0; x != strlen(szWord); x++)
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
			
		SDL_BlitSurface(m_pImageBase, &src, pSurface, &dst);
		
		nCurrentXPos += nFontWidth;		
	}
	
	return pSurface;
}

void CGraphicsUI::DisplayWord(CGraphicsUIPosItem *pPosItem,
								char *szWord, 
								bool bCenter)
{
	
	if(pPosItem == NULL || false == pPosItem->m_bEnabled)
	{
		return;
	}
	
	
	SDL_Surface *pWordSurface = DisplayWord(szWord);
	CopySurface(pWordSurface, m_pScreen, pPosItem, bCenter);
	SDL_FreeSurface(pWordSurface);	
}

void CGraphicsUI::CopySurface(SDL_Surface *pSrcSurface, 
								SDL_Surface *pDstSurface, 
								CGraphicsUIPosItem *pDstPosItem,
								bool bCenter)
{
	int nXPos = 0;
	int nYPos = 0;
	int nWidth = 0;
	int nHeight = 0;
	
	if((NULL == pSrcSurface) || (NULL == pDstSurface) || (NULL == pDstPosItem))
	{
		Log(LOG_ERROR, "CopySurface: error arguments are NULL");
		return;
	}
		
	if(pSrcSurface->w > pDstPosItem->m_pointSize.x)
	{
		nWidth = pDstPosItem->m_pointSize.x - 1;
	}
	else
	{
		nWidth = pSrcSurface->w;
	}
	
	nHeight = pSrcSurface->h;
	nXPos = pDstPosItem->m_pointDst.x;
	nYPos = pDstPosItem->m_pointDst.y + (pDstPosItem->m_pointSize.y / 2);	
		
	if(true == bCenter)
	{
		nXPos = pDstPosItem->m_pointDst.x + ((pDstPosItem->m_pointSize.x/2) - (nWidth/2));
	}
	
	SDL_Rect src = 	{
						0,
						0,
						nWidth,
						nHeight
					};
	
	SDL_Rect dst = 	{ 
						nXPos,
						nYPos,
					};
	
	SDL_BlitSurface(pSrcSurface, &src, pDstSurface, &dst);	
}

void CGraphicsUI::ResetImageArea(CGraphicsUIPosItem *pSrcPosItem, 
									CGraphicsUIPosItem *pDstPosItem,
									SDL_Surface *pSrcSurface, 
									SDL_Surface *pDstSurface)
{
	if((NULL == pSrcPosItem) || 
		(NULL == pDstPosItem) || 
		(NULL == pSrcSurface) || 
		(NULL == pDstSurface))
	{
		Log(LOG_ERROR, "ResetImageArea: error pPosItem is NULL");
		return;
	}
	
	if((false == pSrcPosItem->m_bEnabled) ||
		(false == pDstPosItem->m_bEnabled))
	{
		return;
	}	
	
	SDL_Rect src = 	{ 
						pSrcPosItem->m_pointDst.x,
						pSrcPosItem->m_pointDst.y,
						pSrcPosItem->m_pointSize.x,
						pSrcPosItem->m_pointSize.y
					};
						
	SDL_Rect dst = 	{ 
						pDstPosItem->m_pointDst.x,
						pDstPosItem->m_pointDst.y,
					};
			
	SDL_BlitSurface(pSrcSurface, &src, pDstSurface, &dst);			
}

void CGraphicsUI::ResetImageArea(CGraphicsUIPosItem *pPosItem, 
									SDL_Surface *pSrcSurface, 
									SDL_Surface *pDstSurface)
{
	ResetImageArea(pPosItem, pPosItem, pSrcSurface, pDstSurface);	
}
