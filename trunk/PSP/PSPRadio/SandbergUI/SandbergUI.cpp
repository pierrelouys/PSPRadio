/* 
	PSPRadio / Music streaming client for the PSP. (Initial Release: Sept. 2005)
	PSPRadio Copyright (C) 2005 Rafael Cabezas a.k.a. Raf
	SandbergUI Copyright (C) 2005 Jesper Sandberg


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

#include "SandbergUI.h"

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
#define PIXEL_SIZE (4) /* change this if you change to another screenmode */
#define FRAME_SIZE (BUF_WIDTH * SCR_HEIGHT * PIXEL_SIZE)
#define ZBUF_SIZE (BUF_WIDTH SCR_HEIGHT * 2) /* zbuffer seems to be 16-bit? */

#define META_NAME_X	 9
#define META_NAME_Y	 6
#define META_ARTIST_X	 9
#define META_ARTIST_Y	 7
#define META_URL_X	 9
#define META_URL_Y	 8
#define META_BUFFER_X	 9
#define META_BUFFER_Y	 9
#define META_FORMAT_X	23
#define META_FORMAT_Y	 9
#define META_ERROR_X	 9
#define META_ERROR_Y	10


#define NETWORK_INACTIVE_COLOR		0xFF444444
#define NETWORK_CONNECTING_COLOR	0xFF444444
#define NETWORK_CONNECTED_COLOR		0xFFFFFFFF

#define SOUND_ACTIVE_COLOR		0xFFFFFFFF
#define SOUND_MUTE_COLOR		0xFF444444

#define LOAD_ACTIVE_COLOR		0xFFFFFFFF
#define LOAD_INACTIVE_COLOR		0xFF444444

#define ACTIVE_COLOR			0xFFFFFFFF
#define INACTIVE_COLOR			0xFF444444


static CSandbergUI::IconStr __attribute__((aligned(16))) icon_list[] =
	{
	{440,   8, 472,  40, NETWORK_INACTIVE_COLOR, CSandbergUI::TEX_NETWORK},
	{440,  48, 472,  80, LOAD_INACTIVE_COLOR,    CSandbergUI::TEX_LOAD},
	{440,  88, 472, 120, SOUND_ACTIVE_COLOR,     CSandbergUI::TEX_SOUND},
	{204, 232, 236, 264, INACTIVE_COLOR,         CSandbergUI::TEX_PLAY},
	{244, 232, 280, 264, ACTIVE_COLOR,           CSandbergUI::TEX_STOP},
	};

static CSandbergUI::texture_file __attribute__((aligned(16))) texture_list[] =
	{
	{CSandbergUI::TEX_LOGO,		GU_PSM_8888, 256, 64, "ui_logo.raw"},
	{CSandbergUI::TEX_COMMANDS,	GU_PSM_8888,  64, 64, "commands.raw"},
	{CSandbergUI::TEX_PLATE,	GU_PSM_8888,  64, 64, "plate.raw"},
	{CSandbergUI::TEX_FONT_SMALL,	GU_PSM_8888, 512, 16, "font_small.raw"},
	{CSandbergUI::TEX_PLAY,		GU_PSM_8888,  32, 32, "play.raw"},
	{CSandbergUI::TEX_STOP,		GU_PSM_8888,  32, 32, "stop.raw"},
	{CSandbergUI::TEX_NETWORK,	GU_PSM_8888,  32, 32, "network.raw"},
	{CSandbergUI::TEX_LOAD,		GU_PSM_8888,  32, 32, "load.raw"},
	{CSandbergUI::TEX_SOUND,	GU_PSM_8888,  32, 32, "sound.raw"},
	{CSandbergUI::TEX_OPTIONS,	GU_PSM_8888, 128, 64, "options.raw"},
	};

#define	TEXTURE_COUNT		(sizeof(texture_list) / sizeof(CSandbergUI::texture_file))

static unsigned int __attribute__((aligned(16))) gu_list[262144];


CSandbergUI::CSandbergUI()
{
	framebuffer 	= 0;
	screen_state	= SCREEN_PLAYING;
	select_state	= 0;
	select_target	= 0;
}

CSandbergUI::~CSandbergUI()
{
	if (object_info.vertices)
	{
		free(object_info.vertices);
	}
	if (object_info.faces)
	{
		free(object_info.faces);
	}
}

void CSandbergUI::LoadTextures(char *strCWD)
{
	char					filename[MAXPATHLEN];
	unsigned char 				*filebuffer;
	jsaTextureCache::jsaTextureInfo		texture;

	/*  Load Textures to memory */
	for (unsigned int i = 0 ; i < TEXTURE_COUNT ; i++)
	{
		bool success;

		sprintf(filename, "%s/SandbergUI/%s", strCWD, texture_list[i].filename);
		filebuffer = (unsigned char *) LoadFile(filename);
		texture.format		= texture_list[i].format;
		texture.x		= 0;
		texture.y		= 0;
		texture.width		= texture_list[i].width;
		texture.height		= texture_list[i].height;
		texture.source_width	= texture_list[i].width;
		success = tcache.jsaTCacheStoreTexture(texture_list[i].ID, &texture, filebuffer);
		if (!success)
		{
			Log(LOG_INFO, "Failed loading texture: %s", filename);
		}
		sceKernelDcacheWritebackAll();
		free(filebuffer);
	}
}

int CSandbergUI::Initialize(char *strCWD)
{
	Log(LOG_LOWLEVEL, "Initialize:");

	/* Allocate space in VRAM for 2 displaybuffer and the Zbuffer */
	tcache.jsaTCacheInit((unsigned long)0x154000);

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
	sceGuEnable(GU_LIGHTING);
	sceGuEnable(GU_LIGHT0);
	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);

	sceKernelDcacheWritebackAll();

	InitFX(strCWD);

	Log(LOG_LOWLEVEL, "Initialize: completed");
	return 0;
}

void CSandbergUI::Terminate()
{
	Log(LOG_INFO, "Terminate:");
	sceGuTerm();
	Log(LOG_INFO, "Terminate: completed");
}

int CSandbergUI::SetTitle(char *strTitle)
{
	return 0;
}

int CSandbergUI::DisplayMessage_EnablingNetwork()
{
	icon_list[ICON_NETWORK].color = NETWORK_CONNECTING_COLOR;
	return 0;
}

int CSandbergUI::DisplayMessage_DisablingNetwork()
{
	icon_list[ICON_NETWORK].color = NETWORK_INACTIVE_COLOR;
	return 0;
}

int CSandbergUI::DisplayMessage_NetworkReady(char *strIP)
{
	icon_list[ICON_NETWORK].color = NETWORK_CONNECTED_COLOR;
	UpdateTextItem(TEXT_ERROR, META_ERROR_X, META_ERROR_Y, " ", 0xFFFFFFFF);
	return 0;
}

int CSandbergUI::DisplayMainCommands()
{
	return 0;
}

int CSandbergUI::DisplayActiveCommand(CPSPSound::pspsound_state playingstate)
{
	switch(playingstate)
	{
		case CPSPSound::STOP:
		case CPSPSound::PAUSE:
			{
			icon_list[ICON_STOP].color = ACTIVE_COLOR;
			icon_list[ICON_PLAY].color = INACTIVE_COLOR;
			}
			break;
		case CPSPSound::PLAY:
			{
			icon_list[ICON_PLAY].color = ACTIVE_COLOR;
			icon_list[ICON_STOP].color = INACTIVE_COLOR;
			}
			break;
	}
	return 0;
}


int CSandbergUI::DisplayErrorMessage(char *strMsg)
{
	UpdateTextItem(TEXT_ERROR, META_ERROR_X, META_ERROR_Y, strMsg, 0xFFFFFFFF);

	return 0;
}

int CSandbergUI::DisplayBufferPercentage(int iPercentage)
{
	char	perStr[MAXPATHLEN];

	sprintf(perStr, "Buffer: %03d%c%c", iPercentage, 37, 37);
	UpdateTextItem(TEXT_BUFFER, META_BUFFER_X, META_BUFFER_Y, perStr, 0xFFFFFFFF);

	return 0;
}

int CSandbergUI::OnNewStreamStarted()
{
	return 0;
}

int CSandbergUI::OnStreamOpening()
{
	UpdateTextItem(TEXT_STREAM_URL, META_URL_X, META_URL_Y, "Opening stream", 0xFFFFFFFF);
	UpdateTextItem(TEXT_ERROR, META_ERROR_X, META_ERROR_Y, " ", 0xFFFFFFFF);
	return 0;
}

int CSandbergUI::OnStreamOpeningError()
{
	UpdateTextItem(TEXT_STREAM_URL, META_URL_X, META_URL_Y, "Error opening stream", 0xFFFFFFFF);
	return 0;
}

int CSandbergUI::OnStreamOpeningSuccess()
{
	icon_list[ICON_LOAD].color = LOAD_INACTIVE_COLOR;
	UpdateTextItem(TEXT_STREAM_URL, META_URL_X, META_URL_Y, "Stream opened succesfully", 0xFFFFFFFF);
	UpdateTextItem(TEXT_ERROR, META_ERROR_X, META_ERROR_Y, " ", 0xFFFFFFFF);
	return 0;
}

int CSandbergUI::OnVBlank()
{
	sceGuStart(GU_DIRECT,::gu_list);

	sceGuClearColor(0xFFAA6633);
	sceGuClearDepth(0);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);

	{
		ScePspFVector3 pos = { 0, 0, 1.0f };
		sceGuLight(0,GU_DIRECTIONAL,GU_DIFFUSE_AND_SPECULAR,&pos);
		sceGuLightColor(0,GU_DIFFUSE,0xffffffff);
		sceGuLightColor(0,GU_SPECULAR,0xffffffff);
		sceGuLightAtt(0,1.0f,1.0f,0.0f);
	}
	sceGuSpecular(1.0f);
	sceGuAmbient(0x202020);

	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadIdentity();
	sceGumPerspective(75.0f,16.0f/9.0f,0.5f,1000.0f);

	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();

	switch (screen_state)
	{
		case SCREEN_PLAYING:
		{
			RenderPlayScreen();
		}
		break;
		case SCREEN_OPTIONS:
		{
			RenderOptionScreen();
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
	return 0;
}

void CSandbergUI::RenderPlayScreen(void)
{
	RenderLogo();
	RenderCommands();
	RenderFX();
	RenderPL();
	RenderState();
	RenderNetwork();
	RenderLoad();
	RenderSound();
}

int CSandbergUI::OnNewSongData(CPSPSoundStream::MetaData *pData)
{
	char	strBuf[MAXPATHLEN];

	UpdateTextItem(TEXT_ERROR, META_ERROR_X, META_ERROR_Y, " ", 0xFFFFFFFF);

	if (strlen(pData->strTitle) >= 42)
		pData->strTitle[42] = 0;

	if (strlen(pData->strURL) >= 42)
		pData->strURL[42] = 0;

	if (0 != pData->iSampleRate)
	{
		sprintf(strBuf, "%d kbps %dHz (%d channels)", pData->iBitRate/1000, pData->iSampleRate, pData->iNumberOfChannels);
		UpdateTextItem(TEXT_STREAM_FORMAT, META_FORMAT_X, META_FORMAT_Y, strBuf, 0xFFFFFFFF);
	}

	UpdateTextItem(TEXT_STREAM_URL,  META_URL_X, META_URL_Y, pData->strURI, 0xFFFFFFFF);
	UpdateTextItem(TEXT_STREAM_NAME, META_NAME_X, META_NAME_Y, pData->strTitle, 0xFFFFFFFF);

	if (pData->strArtist && strlen(pData->strArtist))
	{
		UpdateTextItem(TEXT_STREAM_ARTIST, META_ARTIST_X, META_ARTIST_Y, pData->strTitle, 0xFFFFFFFF);
	}

	return 0;
}

int CSandbergUI::OnConnectionProgress()
{
	icon_list[ICON_LOAD].color = LOAD_ACTIVE_COLOR;
	return 0;
}

void CSandbergUI::RenderState(void)
{

	RenderIcon(&icon_list[ICON_PLAY]);
	RenderIcon(&icon_list[ICON_STOP]);
}

void CSandbergUI::RenderNetwork(void)
{
	RenderIcon(&icon_list[ICON_NETWORK]);
}

void CSandbergUI::RenderLoad(void)
{
	RenderIcon(&icon_list[ICON_LOAD]);
}

void CSandbergUI::RenderSound(void)
{
	RenderIcon(&icon_list[ICON_SOUND]);
}

void CSandbergUI::RenderIcon(IconStr *icon_info)
{
	sceGuEnable(GU_TEXTURE_2D);
	sceGuDepthFunc(GU_ALWAYS);

	sceGuAlphaFunc(GU_GREATER,0x80,0xff);
	sceGuEnable(GU_ALPHA_TEST);

	// setup texture
	(void)tcache.jsaTCacheSetTexture(icon_info->ID);
	sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);

	struct Vertex* c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	c_vertices[0].u = 0; c_vertices[0].v = 0;
	c_vertices[0].x = icon_info->x1; c_vertices[0].y = icon_info->y1; c_vertices[0].z = 0;
	c_vertices[0].color = icon_info->color;
	c_vertices[1].u = 32; c_vertices[1].v = 32;
	c_vertices[1].x = icon_info->x2; c_vertices[1].y = icon_info->y2; c_vertices[1].z = 0;
	c_vertices[1].color = icon_info->color;
	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,c_vertices);

	sceGuDisable(GU_ALPHA_TEST);
	sceGuDepthFunc(GU_GEQUAL);
	sceGuDisable(GU_TEXTURE_2D);
}

void CSandbergUI::Initialize_Screen(CScreenHandler::Screen screen)
{
	switch (screen)
	{
		case CScreenHandler::PSPRADIO_SCREEN_SHOUTCAST_BROWSER:
		case CScreenHandler::PSPRADIO_SCREEN_PLAYLIST:
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

void CSandbergUI::UpdateTextItem(int ID, int x, int y, char *strText, unsigned int color)
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

void *CSandbergUI::LoadFile(char *filename)
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
