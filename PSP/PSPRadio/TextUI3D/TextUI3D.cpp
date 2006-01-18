/*
	PSPRadio / Music streaming client for the PSP. (Initial Release: Sept. 2005)
	PSPRadio Copyright (C) 2005 Rafael Cabezas a.k.a. Raf
	TextUI3D Copyright (C) 2005 Jesper Sandberg & Raf


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
#include <math.h>
#include <time.h>
#include <iniparser.h>
#include <Tools.h>
#include <stdarg.h>
#include <Logging.h>

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <psprtc.h>
#include <psppower.h>

#include "TextUI3D.h"
#include "TextUI3D_Panel.h"
#include "jsaTextureCache.h"

#define RGB2BGR(x) (((x>>16)&0xFF) | (x&0xFF00) | ((x<<16)&0xFF0000))

/* Global texture cache */
jsaTextureCache		tcache;

/* Settings */
Settings 	LocalSettings;
gfx_sizes	GfxSizes;

CTextUI3D::CTextUI3D()
{
	m_state	= CScreenHandler::PSPRADIO_SCREENSHOT_NOT_ACTIVE;
}

CTextUI3D::~CTextUI3D()
{
	if (m_Settings)
	{
		delete(m_Settings);
		m_Settings = NULL;
	}
}

int CTextUI3D::Initialize(char *strCWD)
{
char *strCfgFile = NULL;

	/* Load settings from config file */
	strCfgFile = (char *)malloc(strlen(strCWD) + strlen("TextUI3D/TextUI3D.cfg") + 10);
	sprintf(strCfgFile, "%s/%s", strCWD, "TextUI3D/TextUI3D.cfg");
	m_Settings = new CIniParser(strCfgFile);
	free (strCfgFile), strCfgFile = NULL;
	memset(&LocalSettings, 0, sizeof(LocalSettings));
	GetSettings();

	/* Allocate space in VRAM for 2 displaybuffer and the Zbuffer */
	jsaVRAMManager::jsaVRAMManagerInit((unsigned long)0x154000);

	/* Initialize WindowManager */
	m_wmanager.Initialize(strCWD);

	// setup GU
	m_wmanager.WM_SendEvent(WM_EVENT_GU_INIT, NULL);

	sceKernelDcacheWritebackAll();

	return 0;
}

void CTextUI3D::GetConfigPair(char *strKey, int *x, int *y)
{
 	sscanf(m_Settings->GetStr(strKey), "%d,%d", x, y);
}

int CTextUI3D::GetConfigColor(char *strKey)
{
	int iRet;
	sscanf(m_Settings->GetStr(strKey), "%x", &iRet);

	iRet = RGB2BGR(iRet);
	return iRet | 0xFF000000;
}

void CTextUI3D::GetSettings()
{
	/* Settings */
	LocalSettings.EventThreadPrio = m_Settings->GetInteger("SETTINGS:EVENT_THREAD_PRIO", 80);
	GetConfigPair("SETTINGS:PROGRAM_VERSION_XY", &LocalSettings.VersionX, &LocalSettings.VersionY);
	GetConfigPair("SETTINGS:IP_XY", &LocalSettings.IPX, &LocalSettings.IPY);
	GetConfigPair("SETTINGS:BATTERY_ICON_XY", &LocalSettings.BatteryIconX, &LocalSettings.BatteryIconY);
	GetConfigPair("SETTINGS:WIFI_ICON_XY", &LocalSettings.WifiIconX, &LocalSettings.WifiIconY);
	GetConfigPair("SETTINGS:LIST_ICON_XY", &LocalSettings.ListIconX, &LocalSettings.ListIconY);
	GetConfigPair("SETTINGS:VOLUME_ICON_XY", &LocalSettings.VolumeIconX, &LocalSettings.VolumeIconY);
	GetConfigPair("SETTINGS:FONT_SIZE", &LocalSettings.FontWidth, &LocalSettings.FontHeight);
	GetConfigPair("SETTINGS:CLOCK_XY", &LocalSettings.ClockX, &LocalSettings.ClockY);
	LocalSettings.ClockFormat = m_Settings->GetInteger("SETTINGS:CLOCK_FORMAT", 24);
	GetConfigPair("SETTINGS:PROGRESS_BAR_XY", &LocalSettings.ProgressBarX, &LocalSettings.ProgressBarY);
	GetConfigPair("SETTINGS:OPTIONS_XY", &LocalSettings.OptionsX, &LocalSettings.OptionsY);
	LocalSettings.OptionsItemX = m_Settings->GetInteger("SETTINGS:OPTIONS_ITEM_X", 256);
	LocalSettings.OptionsLinespace = m_Settings->GetInteger("SETTINGS:OPTIONS_LINESPACE", 10);
	LocalSettings.OptionsColorNotSelected = GetConfigColor("SETTINGS:OPTIONS_COLOR_NOT_SELECTED");
	LocalSettings.OptionsColorSelected = GetConfigColor("SETTINGS:OPTIONS_COLOR_SELECTED");
	LocalSettings.OptionsColorSelectedActive = GetConfigColor("SETTINGS:OPTIONS_COLOR_SELECTED_ACTIVE");
	LocalSettings.OptionsColorActive = GetConfigColor("SETTINGS:OPTIONS_COLOR_ACTIVE");
	GetConfigPair("SETTINGS:LIST_XY", &LocalSettings.ListX, &LocalSettings.ListY);
	LocalSettings.ListLines = m_Settings->GetInteger("SETTINGS:LIST_LINES", 22);
	LocalSettings.ListLinespace = m_Settings->GetInteger("SETTINGS:LIST_LINESPACE", 8);
	LocalSettings.ListMaxChars = m_Settings->GetInteger("SETTINGS:LIST_MAX_CHARS", 40);
	LocalSettings.ListColorSelected = GetConfigColor("SETTINGS:LIST_COLOR_SELECTED");
	LocalSettings.ListColorNotSelected = GetConfigColor("SETTINGS:LIST_COLOR_NOT_SELECTED");
	LocalSettings.ShowFileExtension = m_Settings->GetInteger("SETTINGS:ShowFileExtension", 0);
	GetConfigPair("SETTINGS:SONGTITLE_XY", &LocalSettings.SongTitleX, &LocalSettings.SongTitleY);
	LocalSettings.SongTitleWidth = m_Settings->GetInteger("SETTINGS:SONGTITLE_WIDTH", 236);
	LocalSettings.SongTitleColor = GetConfigColor("SETTINGS:SONGTITLE_COLOR");
	GetConfigPair("SETTINGS:BUFFER_XY", &LocalSettings.BufferX, &LocalSettings.BufferY);
	GetConfigPair("SETTINGS:BITRATE_XY", &LocalSettings.BitrateX, &LocalSettings.BitrateY);
	GetConfigPair("SETTINGS:USB_ICON_XY", &LocalSettings.USBIconX, &LocalSettings.USBIconY);
	GetConfigPair("SETTINGS:PLAYER_STATE_XY", &LocalSettings.PlayerstateX, &LocalSettings.PlayerstateY);

	/* Graphics sizes */
	GetConfigPair("GRAPHICS:GFX_WIFI_SIZE", &GfxSizes.wifi_w, &GfxSizes.wifi_h);
	GfxSizes.wifi_y = m_Settings->GetInteger("GRAPHICS:GFX_WIFI_H", 8);
	GetConfigPair("GRAPHICS:GFX_POWER_SIZE", &GfxSizes.power_w, &GfxSizes.power_h);
	GfxSizes.power_y = m_Settings->GetInteger("GRAPHICS:GFX_POWER_H", 8);
	GetConfigPair("GRAPHICS:GFX_VOLUME_SIZE", &GfxSizes.volume_w, &GfxSizes.volume_h);
	GfxSizes.volume_y = m_Settings->GetInteger("GRAPHICS:GFX_VOLUME_H", 16);
	GetConfigPair("GRAPHICS:GFX_ICONS_SIZE", &GfxSizes.icons_w, &GfxSizes.icons_h);
	GfxSizes.icons_y = m_Settings->GetInteger("GRAPHICS:GFX_ICONS_H", 40);
	GetConfigPair("GRAPHICS:GFX_PROGRESS_SIZE", &GfxSizes.progress_w, &GfxSizes.progress_h);
	GfxSizes.progress_y = m_Settings->GetInteger("GRAPHICS:GFX_PROGRESS_H", 8);
	GetConfigPair("GRAPHICS:GFX_USB_SIZE", &GfxSizes.usb_w, &GfxSizes.usb_h);
	GfxSizes.usb_y = m_Settings->GetInteger("GRAPHICS:GFX_USB_H", 16);
	GetConfigPair("GRAPHICS:GFX_PLAYSTATE_SIZE", &GfxSizes.playstate_w, &GfxSizes.playstate_h);
	GfxSizes.playstate_y = m_Settings->GetInteger("GRAPHICS:GFX_PLAYSTATE_H", 8);
}

void CTextUI3D::PrepareShutdown()
{
	/* Prepare for shutdown -> Don't render anymore */
	m_state = CScreenHandler::PSPRADIO_SCREENSHOT_ACTIVE;
}

void CTextUI3D::Terminate()
{
	sceGuTerm();
}

int CTextUI3D::SetTitle(char *strTitle)
{
	m_wmanager.WM_SendEvent(WM_EVENT_TEXT_TITLE, strTitle);
	return 0;
}

int CTextUI3D::DisplayMessage_EnablingNetwork()
{
	m_wmanager.WM_SendEvent(WM_EVENT_NETWORK_ENABLE, NULL);
	return 0;
}

int CTextUI3D::DisplayMessage_DisablingNetwork()
{
	m_wmanager.WM_SendEvent(WM_EVENT_NETWORK_DISABLE, NULL);
	return 0;
}

int CTextUI3D::DisplayMessage_NetworkReady(char *strIP)
{
	m_wmanager.WM_SendEvent(WM_EVENT_NETWORK_IP, strIP);
	return 0;
}

int CTextUI3D::DisplayMainCommands()
{
	return 0;
}

int CTextUI3D::DisplayActiveCommand(CPSPSound::pspsound_state playingstate)
{
	switch(playingstate)
	{
		case CPSPSound::STOP:
			{
			m_wmanager.WM_SendEvent(WM_EVENT_PLAYER_STOP, NULL);
			}
			break;
		case CPSPSound::PAUSE:
			{
			m_wmanager.WM_SendEvent(WM_EVENT_PLAYER_PAUSE, NULL);
			}
			break;
		case CPSPSound::PLAY:
			{
			m_wmanager.WM_SendEvent(WM_EVENT_PLAYER_START, NULL);
			}
			break;
	}
	return 0;
}


int CTextUI3D::DisplayErrorMessage(char *strMsg)
{
	m_wmanager.WM_SendEvent(WM_EVENT_TEXT_ERROR, strMsg);
	return 0;
}

int CTextUI3D::DisplayMessage(char *strMsg)
{
//	m_wmanager.WM_SendEvent(WM_EVENT_TEXT_ERROR, strMsg);
	return 0;
}

int CTextUI3D::DisplayBufferPercentage(int iPercentage)
{
	m_wmanager.WM_SendEvent(WM_EVENT_BUFFER, (void *) iPercentage);
	return 0;
}

int CTextUI3D::OnNewStreamStarted()
{
	m_wmanager.WM_SendEvent(WM_EVENT_STREAM_START, NULL);
	return 0;
}

int CTextUI3D::OnStreamOpening()
{
	m_wmanager.WM_SendEvent(WM_EVENT_STREAM_OPEN, NULL);
	return 0;
}

int CTextUI3D::OnConnectionProgress()
{
	m_wmanager.WM_SendEvent(WM_EVENT_STREAM_CONNECTING, NULL);
	return 0;
}

int CTextUI3D::OnStreamOpeningError()
{
	m_wmanager.WM_SendEvent(WM_EVENT_STREAM_ERROR, NULL);
	return 0;
}

int CTextUI3D::OnStreamOpeningSuccess()
{
	m_wmanager.WM_SendEvent(WM_EVENT_STREAM_SUCCESS, NULL);
	return 0;
}

void CTextUI3D::OnScreenshot(CScreenHandler::ScreenShotState state)
{
	m_state = state;
}

int CTextUI3D::OnVBlank()
{
	if (m_state == CScreenHandler::PSPRADIO_SCREENSHOT_NOT_ACTIVE)
	{
		/* Render windows */
		m_wmanager.WM_SendEvent(WM_EVENT_VBLANK, NULL);
	}
	return 0;
}

int CTextUI3D::OnNewSongData(MetaData *pData)
{
	if (strlen(pData->strTitle))
	{
		m_wmanager.AddTitleText(LocalSettings.SongTitleX, LocalSettings.SongTitleY, LocalSettings.SongTitleColor, pData->strTitle);
	}
	else
	{
		if (pData->strURL && strlen(pData->strURL))
		{
		m_wmanager.AddTitleText(LocalSettings.SongTitleX, LocalSettings.SongTitleY, LocalSettings.SongTitleColor, pData->strURL);
		}
		else
		{
		m_wmanager.AddTitleText(LocalSettings.SongTitleX, LocalSettings.SongTitleY, LocalSettings.SongTitleColor, pData->strURI);
		}
	}

	m_wmanager.WM_SendEvent(WM_EVENT_BITRATE, (void *) (pData->iBitRate/1000));

	return 0;
}

void CTextUI3D::Initialize_Screen(IScreen *Screen)
{
	/* Pass to WindowManager */
	if (pPSPApp->GetProgramVersion())
	{
		SetTitle(pPSPApp->GetProgramVersion());
	}

	if (pPSPApp->GetMyIP())
	{
		DisplayMessage_NetworkReady(pPSPApp->GetMyIP());
	}

	switch ((CScreenHandler::Screen)Screen->GetId())
	{
		case CScreenHandler::PSPRADIO_SCREEN_SHOUTCAST_BROWSER:
			m_wmanager.WM_SendEvent(WM_EVENT_SHOUTCAST, NULL);
			break;
		case CScreenHandler::PSPRADIO_SCREEN_PLAYLIST:
			m_wmanager.WM_SendEvent(WM_EVENT_PLAYLIST, NULL);
			break;
		case CScreenHandler::PSPRADIO_SCREEN_LOCALFILES:
			m_wmanager.WM_SendEvent(WM_EVENT_LOCALFILES, NULL);
			break;
		case CScreenHandler::PSPRADIO_SCREEN_OPTIONS:
			m_wmanager.WM_SendEvent(WM_EVENT_OPTIONS, NULL);
			break;
	}
}

void CTextUI3D::OnTimeChange(pspTime *LocalTime)
{
	/* Pass to WindowManager */
	m_wmanager.WM_SendEvent(WM_EVENT_TIME, LocalTime);
}

void CTextUI3D::OnBatteryChange(int Percentage)
{
	/* Pass to WindowManager */
	m_wmanager.WM_SendEvent(WM_EVENT_BATTERY, (void *)Percentage);
}

void CTextUI3D::OnUSBEnable()
{
	/* Pass to WindowManager */
	m_wmanager.WM_SendEvent(WM_EVENT_USB_ENABLE, NULL);
}

void CTextUI3D::OnUSBDisable()
{
	/* Pass to WindowManager */
	m_wmanager.WM_SendEvent(WM_EVENT_USB_DISABLE, NULL);
}

void CTextUI3D::UpdateOptionsScreen(list<OptionsScreen::Options> &OptionsList,
									 list<OptionsScreen::Options>::iterator &CurrentOptionIterator)
{
	bool active_item;
	int y = LocalSettings.OptionsY;
	list<OptionsScreen::Options>::iterator OptionIterator;
	OptionsScreen::Options	Option;

	m_wmanager.WM_SendEvent(WM_EVENT_OPTIONS_CLEAR, NULL);

	if (OptionsList.size() > 0)
	{
		for (OptionIterator = OptionsList.begin() ; OptionIterator != OptionsList.end() ; OptionIterator++)
		{
			if (OptionIterator == CurrentOptionIterator)
			{
			active_item = true;
			}
			else
			{
			active_item = false;
			}
			Option = (*OptionIterator);
			StoreOption(y, active_item, Option.strName, Option.strStates, Option.iNumberOfStates, Option.iSelectedState, Option.iActiveState);
			y += LocalSettings.OptionsLinespace;
		}
	}
	sceKernelDcacheWritebackAll();
}

void CTextUI3D::StoreOption(int y, bool active_item, char *strName, char *strStates[], int iNumberOfStates, int iSelectedState, int iActiveState)
{
	int 			x = LocalSettings.OptionsX;
	unsigned int	color = LocalSettings.OptionsColorSelected;

	if (active_item)
	{
		color = LocalSettings.OptionsColorSelected;
	}
	else
	{
		color = LocalSettings.OptionsColorNotSelected;
	}
	/* The ID field are not used on the option screen */
	m_wmanager.AddOptionText(x, y, color, strName);

	if (iNumberOfStates > 0)
	{
		x = LocalSettings.OptionsItemX;
		for (int iStates = 0; iStates < iNumberOfStates ; iStates++)
		{
			if (iStates+1 == iSelectedState)
			{
				if (iStates+1 == iActiveState)
					{
					color =  LocalSettings.OptionsColorSelectedActive;
					}
				else
					{
					color = LocalSettings.OptionsColorSelected;
					}

				/* For the moment only render the selected item .. No space on screen */
				/* The ID field are not used on the option screen */
				m_wmanager.AddOptionText(x, y, color, strStates[iStates]);
				x += (strlen(strStates[iStates])+1) * 8;
				continue;
			}
		}
	}
}

void CTextUI3D::DisplayContainers(CMetaDataContainer *Container)
{
	int			 	list_cnt, render_cnt;
	int				current = 1;
	int				i = 0;
	unsigned int	color;
	int				y = LocalSettings.ListY;
	int				list_size = LocalSettings.ListLines;
	int				first_entry;
	char 			*strTemp = (char *)malloc (MAXPATHLEN);


	map< string, list<MetaData>* >::iterator ListIterator;
	map< string, list<MetaData>* >::iterator *CurrentElement = Container->GetCurrentContainerIterator();
	map< string, list<MetaData>* > *List = Container->GetContainerList();

	list_cnt = List->size();

	m_wmanager.WM_SendEvent(WM_EVENT_LIST_CLEAR, NULL);

	/*  Find number of current element */
	if (list_cnt > 0)
	{
		for (ListIterator = List->begin() ; ListIterator != List->end() ; ListIterator++, current++)
		{
			if (ListIterator == *CurrentElement)
			{
			break;
			}
		}

		/* Calculate start element in list */
		first_entry = FindFirstEntry(list_cnt, current);

		/* Find number of elements to show */
		render_cnt = (list_cnt > list_size) ? list_size : list_cnt;

		/* Go to start element */
		for (ListIterator = List->begin() ; i < first_entry ; i++, ListIterator++);

		for (i = 0 ; i < list_size ; i++)
		{
			if (i < render_cnt)
			{
				if (ListIterator == *CurrentElement)
				{
					color = LocalSettings.ListColorSelected;
				}
				else
				{
					color = LocalSettings.ListColorNotSelected;
				}
				strncpy(strTemp, ListIterator->first.c_str(), MAXPATHLEN);
				strTemp[MAXPATHLEN - 1] = 0;
				if (strlen(strTemp) > 4 && memcmp(strTemp, "ms0:", 4) == 0)
				{
					char *pStrTemp = basename(strTemp);
					pStrTemp[LocalSettings.ListMaxChars] = 0;
					m_wmanager.AddListText(LocalSettings.ListX, y, color, pStrTemp);
				}
				else
				{
					strTemp[LocalSettings.ListMaxChars] = 0;
					m_wmanager.AddListText(LocalSettings.ListX, y, color, strTemp);
				}
				y += LocalSettings.ListLinespace;
				ListIterator++;
			}
		}
	}
	free (strTemp), strTemp = NULL;
}

void CTextUI3D::DisplayElements(CMetaDataContainer *Container)
{
	int			 	list_cnt, render_cnt;
	int				current = 1;
	int				i = 0;
	unsigned int	color;
	int				y = LocalSettings.ListY;
	int				list_size = LocalSettings.ListLines;
	int				first_entry;
	char 			*strTemp = (char *)malloc (MAXPATHLEN+1);


	list<MetaData>::iterator ListIterator;
	list<MetaData>::iterator *CurrentElement = Container->GetCurrentElementIterator();
	list<MetaData> *List = Container->GetElementList();


	list_cnt = List->size();

	m_wmanager.WM_SendEvent(WM_EVENT_ENTRY_CLEAR, NULL);

	/*  Find number of current element */
	if (list_cnt > 0)
	{
		for (ListIterator = List->begin() ; ListIterator != List->end() ; ListIterator++, current++)
		{
			if (ListIterator == *CurrentElement)
			{
			break;
			}
		}

		/* Calculate start element in list */
		first_entry = FindFirstEntry(list_cnt, current);

		/* Find number of elements to show */
		render_cnt = (list_cnt > list_size) ? list_size : list_cnt;

		/* Go to start element */
		for (ListIterator = List->begin() ; i < first_entry ; i++, ListIterator++);

		for (i = 0 ; i < list_size ; i++)
		{
			if (i < render_cnt)
			{
				if (ListIterator == *CurrentElement)
				{
					color = LocalSettings.ListColorSelected;
				}
				else
				{
					color = LocalSettings.ListColorNotSelected;
				}

				if (strlen((*ListIterator).strTitle))
				{
					strncpy(strTemp, (*ListIterator).strTitle, MAXPATHLEN);
				}
				else
				{
					strncpy(strTemp, (*ListIterator).strURI, MAXPATHLEN);
				}

				if (strlen(strTemp) > 4 && memcmp(strTemp, "ms0:", 4) == 0)
				{
					char *pStrTemp = basename(strTemp);
					if (0 == LocalSettings.ShowFileExtension)
					{
						char *ext = strrchr(pStrTemp, '.');
						if(ext)
						{
							ext[0] = 0;
						}
					}
					pStrTemp[LocalSettings.ListMaxChars] = 0;
					m_wmanager.AddEntryText(LocalSettings.ListX, y, color, pStrTemp);
				}
				else
				{
					strTemp[LocalSettings.ListMaxChars] = 0;
					m_wmanager.AddEntryText(LocalSettings.ListX, y, color, strTemp);
				}

				y += LocalSettings.ListLinespace;
				ListIterator++;
			}
		}
	}
	free (strTemp), strTemp = NULL;
}

int CTextUI3D::FindFirstEntry(int list_cnt, int current)
{
	int		first_entry;
	int		list_size = LocalSettings.ListLines;
	int		list_margin = (((list_size-1) / 2) + 1);

	/* Handle start of list */
	if ((list_cnt <= list_size) || (current < list_margin))
	{
		first_entry = 0;
	}
	/* Handle end of list */
	else if ((list_cnt > list_size) && ((list_cnt - current) < list_margin))
	{
		first_entry = list_cnt - list_size;
	}
	/* Handle rest of list */
	else
	{
		first_entry = current - list_margin;
	}

	return first_entry;
}

void CTextUI3D::OnCurrentContainerSideChange(CMetaDataContainer *Container)
{
	/* Pass to WindowManager */

	switch (Container->GetCurrentSide())
	{
		case	CMetaDataContainer::CONTAINER_SIDE_CONTAINERS:
			m_wmanager.WM_SendEvent(WM_EVENT_SELECT_LIST, NULL);
			break;
		case	CMetaDataContainer::CONTAINER_SIDE_ELEMENTS:
			m_wmanager.WM_SendEvent(WM_EVENT_SELECT_ENTRIES, NULL);
			break;
		default:
			break;
	}
}
