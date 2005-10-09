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
#include <pspdebug.h>

#include <pspgu.h>
#include <pspgum.h>

#include "SandbergUI.h"

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
#define PIXEL_SIZE (4) /* change this if you change to another screenmode */
#define FRAME_SIZE (BUF_WIDTH * SCR_HEIGHT * PIXEL_SIZE)
#define ZBUF_SIZE (BUF_WIDTH SCR_HEIGHT * 2) /* zbuffer seems to be 16-bit? */

static unsigned int __attribute__((aligned(16))) list[262144];

CSandbergUI::CSandbergUI()
{
	framebuffer = 0;
}

CSandbergUI::~CSandbergUI()
{
}

int CSandbergUI::Initialize(char *strCWD)
{	
	Log(LOG_LOWLEVEL, "Initialize:");

	pspDebugScreenInit();
	// setup GU

	sceGuInit();

	sceGuStart(GU_DIRECT,::list);
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
	sceGuEnable(GU_CLIP_PLANES);

	sceGuEnable(GU_LIGHTING);
	sceGuEnable(GU_LIGHT0);
	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);

	sceKernelDcacheWritebackAll();

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
	return 0;
}

int CSandbergUI::DisplayMessage_NetworkSelection(int iProfileID, char *strProfileName)
{
	return 0;
}

int CSandbergUI::DisplayMessage_DisablingNetwork()
{
	return 0;
}

int CSandbergUI::DisplayMessage_NetworkReady(char *strIP)
{
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
			break;
		
		case CPSPSound::PLAY:
			break;
		
		case CPSPSound::PAUSE:
			break;
	}
	
	return 0;
}

int CSandbergUI::DisplayErrorMessage(char *strMsg)
{
	return 0;
}

int CSandbergUI::DisplayBufferPercentage(int iPercentage)
{
	return 0;
}

int CSandbergUI::OnNewStreamStarted()
{
	return 0;
}

int CSandbergUI::OnStreamOpening()
{
	return 0;
}

int CSandbergUI::OnStreamOpeningError()
{
	return 0;
}

int CSandbergUI::OnStreamOpeningSuccess()
{
	return 0;
}

int CSandbergUI::OnVBlank()
{
	sceGuStart(GU_DIRECT,::list);
/*
	sceGuDisable(GU_TEXTURE_2D);
	sceGuDisable(GU_ALPHA_TEST);
*/
	sceGuClearColor(0x00AA6633);
	sceGuClearDepth(0);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);

	{
		ScePspFVector3 pos = { 0, 0, 0.0f };
		sceGuLight(0,GU_POINTLIGHT,GU_AMBIENT_AND_DIFFUSE,&pos);
		sceGuLightColor(0,GU_AMBIENT_AND_DIFFUSE,0xffffffff);
		sceGuLightAtt(0,4.0f,4.0f,0.0f);
	}
	sceGuAmbient(0xFFFFFFFF);

	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadIdentity();
	sceGumPerspective(75.0f,16.0f/9.0f,0.5f,1000.0f);

	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();

	RenderFX();
	RenderPL();

	sceGuFinish();
	sceGuSync(0,0);

	framebuffer = sceGuSwapBuffers();
	return 0;
}

int CSandbergUI::OnNewSongData(CPlayList::songmetadata *pData)
{
	return 0;
}

int CSandbergUI::DisplayPLList(CDirList *plList)
{
	return 0;
}

int CSandbergUI::DisplayPLEntries(CPlayList *PlayList)
{
	return 0;
}

int CSandbergUI::OnConnectionProgress()
{
	return 0;
}
