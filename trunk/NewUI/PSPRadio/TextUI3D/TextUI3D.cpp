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

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
#define PIXEL_SIZE (4)
#define FRAME_SIZE (BUF_WIDTH * SCR_HEIGHT * PIXEL_SIZE)
#define ZBUF_SIZE (BUF_WIDTH SCR_HEIGHT * 2) /* zbuffer seems to be 16-bit? */


static CTextUI3D::texture_file __attribute__((aligned(16))) texture_list[] =
	{
	{CTextUI3D::TEX_BACKGROUND, GU_PSM_8888, 480, 272, true, "BackgroundImage.png"},
	{CTextUI3D::TEX_CORNER, 	GU_PSM_8888,  32,  32, true, "Corner.png"},
	{CTextUI3D::TEX_VERTICAL, 	GU_PSM_8888,  32,  32, true, "Vertical.png"},
	{CTextUI3D::TEX_HORIZONTAL, GU_PSM_8888,  32,  32, true, "Horizontal.png"},
	{CTextUI3D::TEX_FILL, 		GU_PSM_8888,  32,  32, true, "Fill.png"},
	{CTextUI3D::TEX_FONT, 		GU_PSM_8888, 512,   8, true, "SmallFont.png"},
	};

#define	TEXTURE_COUNT		(sizeof(texture_list) / sizeof(CTextUI3D::texture_file))

static unsigned int __attribute__((aligned(16))) gu_list[262144];

CTextUI3D::CTextUI3D()
{
	framebuffer 	= 0;
	screen_state	= SCREEN_PLAYING;
	m_state			= CScreenHandler::PSPRADIO_SCREENSHOT_NOT_ACTIVE;
}

CTextUI3D::~CTextUI3D()
{
}

void CTextUI3D::LoadTextures(char *strCWD)
{
	char								filename[MAXPATHLEN];
	unsigned char 						*filebuffer;
	jsaTextureCache::jsaTextureInfo		texture;

	sprintf(filename, "%s/TextUI3D/%s", strCWD, texture_list[0].filename);
	backimage = (unsigned char *) memalign(16, SCR_WIDTH * SCR_HEIGHT * PIXEL_SIZE);

	if (backimage == NULL)
		{
		Log(LOG_ERROR, "Memory allocation error for background image: %s", filename);
		return;
		}

	if (tcache.jsaTCacheLoadPngImage((const char *)filename, (u32 *)backimage) == -1)
		{
		Log(LOG_ERROR, "Failed loading background image: %s", filename);
		free(backimage);
		return;
		}

	sceKernelDcacheWritebackAll();

	/*  Load Textures to memory, skip background */
	for (unsigned int i = 1 ; i < TEXTURE_COUNT ; i++)
	{
		bool success;

		sprintf(filename, "%s/TextUI3D/%s", strCWD, texture_list[i].filename);
		filebuffer = (unsigned char *) memalign(16, (int)(texture_list[i].width * texture_list[i].height * tcache.jsaTCacheTexturePixelSize(texture_list[i].format)));
		if (tcache.jsaTCacheLoadPngImage((const char *)filename, (u32 *)filebuffer) == -1)
			{
			Log(LOG_ERROR, "Failed loading png file: %s", filename);
			free(filebuffer);
			continue;
			}
		texture.format		= texture_list[i].format;
		texture.width		= texture_list[i].width;
		texture.height		= texture_list[i].height;
		texture.swizzle		= texture_list[i].swizzle;
		success = tcache.jsaTCacheStoreTexture(texture_list[i].ID, &texture, filebuffer);
		if (!success)
		{
			Log(LOG_ERROR, "Failed storing texture in VRAM : %s", filename);
		}
		sceKernelDcacheWritebackAll();
		free(filebuffer);
	}
}

int CTextUI3D::Initialize(char *strCWD)
{
char local_time[64];

	Log(LOG_LOWLEVEL, "Initialize:");

	/* Allocate space in VRAM for 2 displaybuffer and the Zbuffer */
	jsaVRAMManager::jsaVRAMManagerInit((unsigned long)0x154000);

	/*  Load all textures to VRAM */
	LoadTextures(strCWD);

	// setup GU
	sceGuInit();

	sceGuStart(GU_DIRECT,::gu_list);
	sceGuDrawBuffer(GU_PSM_8888,(void*)0,BUF_WIDTH);
	sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,(void*)0x88000,BUF_WIDTH);
	sceGuDepthBuffer((void*)0x110000,BUF_WIDTH);
	sceGuOffset(2048 - (SCR_WIDTH/2),2048 - (SCR_HEIGHT/2));
	sceGuViewport(2048,2048,SCR_WIDTH,SCR_HEIGHT);
	sceGuDepthRange(0xc350,0x2710);
	sceGuScissor(0,0,SCR_WIDTH,SCR_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuDepthFunc(GU_GEQUAL);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuFrontFace(GU_CW);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_CULL_FACE);
	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);

	sceKernelDcacheWritebackAll();

	sprintf(local_time, "%02d:%02d", 0, 0);
	UpdateTextItem(TEXT_STATIC, 0, 1, local_time, 0xFFFFFFFF);
	sprintf(local_time, "Battery:%03d%%", scePowerGetBatteryLifePercent());
	UpdateTextItem(TEXT_STATIC, 0, 2, local_time, 0xFFFFFFFF);

	Log(LOG_LOWLEVEL, "Initialize: completed");
	return 0;
}

void CTextUI3D::Terminate()
{
	if (backimage)
		{
		free(backimage);
		}

	sceGuTerm();
}

int CTextUI3D::SetTitle(char *strTitle)
{
	UpdateTextItem(TEXT_STATIC, 0, 0, strTitle, 0xFFFFFFFF);
	return 0;
}

int CTextUI3D::DisplayMessage_EnablingNetwork()
{
	return 0;
}

int CTextUI3D::DisplayMessage_DisablingNetwork()
{
	return 0;
}

int CTextUI3D::DisplayMessage_NetworkReady(char *strIP)
{
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
		case CPSPSound::PAUSE:
			{
			}
			break;
		case CPSPSound::PLAY:
			{
			}
			break;
	}
	return 0;
}


int CTextUI3D::DisplayErrorMessage(char *strMsg)
{
	return 0;
}

int CTextUI3D::DisplayBufferPercentage(int iPercentage)
{
	return 0;
}

int CTextUI3D::OnNewStreamStarted()
{
	return 0;
}

int CTextUI3D::OnStreamOpening()
{
	return 0;
}

int CTextUI3D::OnStreamOpeningError()
{
	return 0;
}

int CTextUI3D::OnStreamOpeningSuccess()
{
	return 0;
}

void CTextUI3D::OnScreenshot(CScreenHandler::ScreenShotState state)
{
	m_state = state;
}

int CTextUI3D::OnVBlank()
{
static	bool skip_first = true;

	if (m_state == CScreenHandler::PSPRADIO_SCREENSHOT_NOT_ACTIVE)
	{
		sceGuStart(GU_DIRECT,::gu_list);

		sceGuClearColor(0x00000000);
		sceGuClearDepth(0);
		sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);

		sceGumMatrixMode(GU_PROJECTION);
		sceGumLoadIdentity();
		sceGumPerspective(75.0f,16.0f/9.0f,0.5f,1000.0f);

		sceGumMatrixMode(GU_VIEW);
		sceGumLoadIdentity();

		/* Copy Background using the GE. Don't render on first frame since the framebuffer is unknown */

		if (skip_first)
			{
			skip_first = false;
			}

		else
			{
//			sceGuCopyImage(GU_PSM_8888, 0, 0, 480, 272, 480, backimage, 0, 0, 512, framebuffer);
			sceGuCopyImage(GU_PSM_8888, 0, 0, 480, 272, 480, backimage, 0, 0, 512, (void*)(0x04000000+(u32)framebuffer));
//			memcpy(framebuffer, backimage, SCR_WIDTH * SCR_HEIGHT * PIXEL_SIZE);
			}

		RenderFrame1();

		switch (screen_state)
		{
			case SCREEN_PLAYING:
				{
				}
				break;
			case SCREEN_OPTIONS:
				{
				}
				break;
			default:
				{
				}
				break;
		}

		sceGuFinish();
		sceGuSync(0,0);

		framebuffer = sceGuSwapBuffers();
	}
	return 0;
}

int CTextUI3D::OnNewSongData(MetaData *pData)
{

	return 0;
}

int CTextUI3D::OnConnectionProgress()
{
	return 0;
}

void CTextUI3D::Initialize_Screen(CScreenHandler::Screen screen)
{
	char	strText[64];
	pspTime local_time;

	sceRtcGetCurrentClockLocalTime(&local_time);

	sprintf(strText, "%02d:%02d", local_time.hour, local_time.minutes);
	UpdateTextItem(TEXT_STATIC, 0, 0, strText, 0xFFFFFFFF);
	sprintf(strText, "Battery:%03d%%", scePowerGetBatteryLifePercent());
	UpdateTextItem(TEXT_STATIC, 0, 1, strText, 0xFFFFFFFF);

	switch (screen)
	{
		case CScreenHandler::PSPRADIO_SCREEN_SHOUTCAST_BROWSER:
		case CScreenHandler::PSPRADIO_SCREEN_PLAYLIST:
		case CScreenHandler::PSPRADIO_SCREEN_LOCALFILES:
			{
			screen_state = SCREEN_PLAYING;
			}
			break;

		case CScreenHandler::PSPRADIO_SCREEN_OPTIONS:
		{
			screen_state = SCREEN_OPTIONS;
		}
		break;
	}
}

void CTextUI3D::UpdateTextItem(int ID, int x, int y, char *strText, unsigned int color)
{
	StoredOptionItem			Option;
	list<StoredOptionItem>::iterator 	OptionIterator;
	bool					found = false;

	if (OptionsItems.size() > 0)
	{
		for (OptionIterator = OptionsItems.begin() ; OptionIterator != OptionsItems.end() ; OptionIterator++)
		{
			if ((*OptionIterator).ID == ID)
			{
				strcpy((*OptionIterator).strText, strText);
				strupr((*OptionIterator).strText);
				(*OptionIterator).color = color;
				found = true;
				break;
			}
		}
	}
	if (!found)
	{
		Option.x = x;
		Option.y = y;
		Option.color = 0xFFFFFFFF;
		strcpy(Option.strText, strText);
		strupr(Option.strText);
		Option.color = color;
		Option.ID = ID;
		OptionsItems.push_back(Option);
	}
}

void *CTextUI3D::LoadFile(char *filename)
{
	unsigned char	*mem_buffer = NULL;
	FILE		*fhandle;

	fhandle = fopen(filename, "r");
	if (fhandle != NULL);
	{
		int bytes;
		(void)fseek(fhandle, 0, SEEK_END);
		int filesize = ftell(fhandle);

		if (filesize != -1)
		{
			(void)fseek(fhandle, 0, SEEK_SET);
			mem_buffer = (unsigned char *)memalign(16, filesize);
			if (mem_buffer)
			{
				bytes = fread(mem_buffer, 1, filesize, fhandle);
				if (bytes != filesize)
					{
					free(mem_buffer);
					mem_buffer = NULL;
					}
			}
		}
		fclose(fhandle);
	}
	sceKernelDcacheWritebackAll();
	return mem_buffer;
}

void CTextUI3D::OnTimeChange(pspTime *LocalTime)
{
}

void CTextUI3D::OnBatteryChange(int Percentage)
{
}

void CTextUI3D::UpdateOptionsScreen(list<OptionsScreen::Options> &OptionsList, 
									list<OptionsScreen::Options>::iterator &CurrentOptionIterator)
{
}

void CTextUI3D::DisplayContainers(CMetaDataContainer *Container)
{
}

void CTextUI3D::DisplayElements(CMetaDataContainer *Container)
{
}

void CTextUI3D::OnCurrentContainerSideChange(CMetaDataContainer *Container)
{

	switch (Container->GetCurrentSide())
	{
		case	CMetaDataContainer::CONTAINER_SIDE_CONTAINERS:
			break;
		case	CMetaDataContainer::CONTAINER_SIDE_ELEMENTS:
			break;
		default:
			break;
	}
}
