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
#include <pspdisplay.h>
#include <pspge.h>
//#include <PSPApp.h>
//#include <PSPSound.h>
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <malloc.h>
//#include <iniparser.h>
#include <Tools.h>
#include <stdarg.h>
//#include <Screen.h>
#include <pthread.h>
#include "Scope.h"
//#include <psputility_sysparam.h>

PSP_MODULE_INFO("UI_TEXT", 0, 1, 1);
PSP_HEAP_SIZE_KB(512);

int module_stop(int args, void *argp)
{
};

#define true 1
#define false 0
#define RGB2BGR(x) (((x>>16)&0xFF) | (x&0xFF00) | ((x<<16)&0xFF0000))
#define UNSET_DIRTY(x) {m_isdirty&=~x;}

#define VIS_X1 0
#define VIS_X2 480
#define VIS_Y_MID (272/2)
#define VIS_PCM_SHIFT 8
//#define VIS_X1 170
//#define VIS_X2 308
//#define VIS_Y_MID 128
//#define VIS_PCM_SHIFT 9
#define VIS_WIDTH (VIS_X2 - VIS_X1)
#define IS_BUTTON_PRESSED(i,b) ((i & 0xFFFF) == b)
#define VIS_KEY_PREV (PSP_CTRL_LEFT)
#define VIS_KEY_NEXT (PSP_CTRL_RIGHT)
short *s_pcmbuffer = NULL;

typedef void (*draw_pcm_func)(int iBuffer);

void draw_pcm_bars(int iBuffer);
void draw_pcm_osc(int iBuffer);
void draw_pcm_osc_vl(int iBuffer);
void draw_pcm_osc_v2(int iBuffer);
void draw_pcm_osc_v3(int iBuffer);
void draw_pcm_osc_v4(int iBuffer);
void draw_pcm_osc_v5(int iBuffer);

draw_pcm_func visualizer[] = { 
	draw_pcm_bars,
	draw_pcm_osc,
	draw_pcm_osc_vl,
	draw_pcm_osc_v2,
	draw_pcm_osc_v3,
	draw_pcm_osc_v4,
	draw_pcm_osc_v5
 };

int current_visualizer = 0;
#define number_of_visualizers 7//sizeof(visualizer)/sizeof(draw_pcm_func)

#define m_Width  480
#define m_Height 272
#define m_Pitch  512
#define m_BytesPerPixel 4
#define FRAMESIZE (m_Pitch*m_Height*m_BytesPerPixel)
u32 *m_Buffer[4];
void init()
{
	int i;
	u32 *vram_base = (u32 *) ((u32) sceGeEdramGetAddr());
//	sceDisplaySetMode(0, m_Width, m_Height);
//	sceDisplaySetFrameBuf((void *) g_vram_base, 
//		m_Pitch, m_PixelFormat, PSP_DISPLAY_SETBUF_NEXTFRAME);

	for (i = 0; i < 4; i++)
	{
		m_Buffer[i] = (u32*)((char*)vram_base+FRAMESIZE*i);
	}
}

void Plot(int iBuffer, int x, int y, int color)
{
	u32 *pixel = m_Buffer[iBuffer] + m_Pitch*y + x;
	//*pixel = color;
	//*pixel = *pixel & color;
	*pixel = *pixel | color;
}

void Rectangle(int iBuffer, int x1, int y1, int x2, int y2, int color)
{
	int x,y;
	for (x = x1;x <= x2; x++)
	{
		for (y = y1;y <= y2; y++)
		{
			Plot(iBuffer, x, y, color);
		}
	}
}

void VertLine(int iBuffer, int x, int y1, int y2, int color)
{
	int y;
	if (y1 < y2)
	{
		for (y = y1<0?0:y1; y <= y2; y++)
		{
			Plot(iBuffer, x, y, color);
		}
	}
	else if (y2 < y1)
	{
		for (y = y2<0?0:y2; y <= y1; y++)
		{
			Plot(iBuffer, x, y, color);
		}
	}
	else 
	{
		Plot(iBuffer, x, y1, color);
	}
	
}

#if 0
	/* Start Render Thread */
	{
		pthread_t pthid;
		pthread_attr_t pthattr;
		struct sched_param shdparam;
		pthread_attr_init(&pthattr);
		shdparam.sched_policy = SCHED_OTHER;
		shdparam.sched_priority = 45;
		pthread_attr_setschedparam(&pthattr, &shdparam);
		s_exit = false;
		pthread_create(&pthid, &pthattr, render_thread, NULL);
	}
#endif

int m_isdirty = 0;
#define DIRTY_PCM 1
void NewPCMBuffer(short *pcmbuffer)
{
	s_pcmbuffer = pcmbuffer;
	m_isdirty |= DIRTY_PCM;
}

#if 0
void render_thread(void *)
{
	static int iBuffer = 0;
	bool draw_background = true;
	
	/* For FPS Calculation: */
	clock_t time1, time2;
	int frame_count = 0;
	int total_time = 0;
	int fps = 0;
	int message_frames = 0;

	for (;;)
	{
		if (s_exit)
		{
			sceKernelDelayThread(1); /* yield */
			break;
		}
		s_ui->m_RenderLock->Lock();
		if (s_ui->m_isdirty)// && (s_ui->m_ScreenShotState == CScreenHandler::PSPRADIO_SCREENSHOT_NOT_ACTIVE))
		{
			time1 = sceKernelLibcClock();

			if (s_ui->m_isdirty & DIRTY_BACKGROUND)
			{
				UNSET_DIRTY(DIRTY_BACKGROUND);
				CopyRectangle(BACKGROUND_BUFFER, OFFLINE_BUFFER, 
					0, 0, m_Width, m_Height);
				s_ui->PrintProgramVersion(OFFLINE_BUFFER);
				draw_background  = false;
			}
			else
			{
				draw_background = true;
			}
			
			if (s_ui->m_isdirty & DIRTY_TIME)
			{
				UNSET_DIRTY(DIRTY_TIME);
				s_ui->PrintTime(OFFLINE_BUFFER, draw_background);
			}
			if (s_ui->m_isdirty & DIRTY_BATTERY)
			{
				UNSET_DIRTY(DIRTY_BATTERY);
				s_ui->PrintBattery(OFFLINE_BUFFER, draw_background);
			}
			if (s_ui->m_isdirty & DIRTY_BUFFER_PERCENTAGE)
			{
				UNSET_DIRTY(DIRTY_BUFFER_PERCENTAGE);
				s_ui->PrintBufferPercentage(OFFLINE_BUFFER, draw_background);
			}
			if (s_ui->m_isdirty & DIRTY_SONG_DATA)
			{
				UNSET_DIRTY(DIRTY_SONG_DATA);
				s_ui->PrintSongData(OFFLINE_BUFFER, draw_background);
			}
			if (s_ui->m_isdirty & DIRTY_STREAM_TIME)
			{
				UNSET_DIRTY(DIRTY_STREAM_TIME);
				s_ui->PrintStreamTime(OFFLINE_BUFFER, draw_background);
			}
			if (s_ui->m_isdirty & DIRTY_CONTAINERS)
			{
				UNSET_DIRTY(DIRTY_CONTAINERS);
				s_ui->PrintContainers(OFFLINE_BUFFER, draw_background);
			}
			if (s_ui->m_isdirty & DIRTY_ELEMENTS)
			{
				UNSET_DIRTY(DIRTY_ELEMENTS);
				s_ui->PrintElements(OFFLINE_BUFFER, draw_background);
			}
			if (s_ui->m_isdirty & DIRTY_OPTIONS)
			{
				UNSET_DIRTY(DIRTY_OPTIONS);
				s_ui->PrintOptionsScreen(OFFLINE_BUFFER, draw_background);
			}
			if (s_ui->m_isdirty & DIRTY_ACTIVE_COMMAND)
			{
				UNSET_DIRTY(DIRTY_ACTIVE_COMMAND);
				s_ui->PrintActiveCommand(OFFLINE_BUFFER, draw_background);
			}
			
			/* Copy buffer OFFLINE_BUFFER to back-buffer */
			CopyFromToBuffer(OFFLINE_BUFFER, iBuffer);
	
			/* Do effects to back-buffer */
			if ((s_ui->m_isdirty & DIRTY_PCM) && s_pcmbuffer)
			{
				UNSET_DIRTY(DIRTY_PCM);
				//draw_pcm(iBuffer);
				visualizer[current_visualizer](iBuffer);
			}
			
			if (s_ui->m_isdirty & DIRTY_MESSAGE)
			{
				//int x = (MAX_COL - strlen(s_ui->m_Message)) / 2;
				UNSET_DIRTY(DIRTY_MESSAGE);
				message_frames = 1;
			}
			
			if (message_frames > 0)
			{
				s_ui->PrintMessage(iBuffer);//, s_ui->m_Message);//s_ui->uiPrintf(iBuffer, 100,100, 0xFFFFFF, s_ui->m_Message);
				if (message_frames++ >= 60)
				{
					message_frames = 0;
				}
			}

			/* FPS Calculation */
			time2 = sceKernelLibcClock();
			total_time += (time2 - time1);
			if (++frame_count == 10)
			{
				fps = (frame_count * CLOCKS_PER_SEC) / total_time;
				frame_count = 0;
				total_time = 0;

				{
					SceCtrlData pad;
				
					sceCtrlPeekBufferPositive(&pad, 1);
					
					if (IS_BUTTON_PRESSED(pad.Buttons, VIS_KEY_PREV))
					{
						current_visualizer = (current_visualizer - 1 < 0)?(number_of_visualizers - 1):(current_visualizer - 1);
					}
					else if (IS_BUTTON_PRESSED(pad.Buttons, VIS_KEY_NEXT))
					{
						current_visualizer = (current_visualizer + 1) % number_of_visualizers;
					}
				}
			}
			s_ui->uiPrintf(iBuffer, 10, 262, 0xFFFFFFFF, "fps:%03d vis:%d", fps, current_visualizer);

			///Buffer is configured in sync mode already... 
			//sceDisplayWaitVblankStart();
			//Flip Buffers
			sceKernelDcacheWritebackAll(); 
			SetFrameBuffer(iBuffer);
			iBuffer = 1 - iBuffer;
		}
		s_ui->m_RenderLock->Unlock();
		sceKernelDelayThread(1); /* yield */
	}
	s_ui->m_RenderExitBlocker->UnBlock(); /* Let the destructor continue */
}
#endif

void draw_pcm_bars(int iBuffer)
{
	int x;
	for (x = VIS_X1; x < VIS_X2; x++)
	{
		//convert fixed point int to int (the integer part is the most significant byte)
		// (fixed_point >> 8) == integer part. We get a range from 0 < y < 128
		Rectangle(iBuffer, x, VIS_Y_MID - (s_pcmbuffer[x*5] >> VIS_PCM_SHIFT), 
										   x+4, VIS_Y_MID, 0xAAAAAA);
	}
}

void draw_pcm_osc(int iBuffer)
{
	//m_Screen->DrawBackground(iBuffer, 0, 0, 100, 100);
	//m_Screen->Rectangle(iBuffer, 0,0, 128, 128, 0);
	int x;
	int y1, y2;
	for (x = VIS_X1; x < VIS_X2; x++)
	{
		//convert fixed point int to int (the integer part is the most significant byte)
		// (fixed_point >> 8) == integer part. We get a range from 0 < y < 256
		y1 = VIS_Y_MID + (s_pcmbuffer[x*5] >> VIS_PCM_SHIFT);
		y2 = VIS_Y_MID - (s_pcmbuffer[x*5+1] >> VIS_PCM_SHIFT);
		Plot(iBuffer, x, (y1 >= 0 && y1 < VIS_Y_MID*2)?y1:VIS_Y_MID, 0xAAAAAA);
		Plot(iBuffer, x, (y2 >= 0 && y2 < VIS_Y_MID*2)?y2:VIS_Y_MID, 0xAAAAAA);
	}
}

void draw_pcm_osc_vl(int iBuffer)
{
	int x;
	int y1, y2;
	for (x = VIS_X1; x < VIS_X2; x++)
	{
		//convert fixed point int to int (the integer part is the most significant byte)
		// (fixed_point >> 8) == integer part. But I'll use >> 9 to get a range from 64 < y < 192
		y1 = VIS_Y_MID - (s_pcmbuffer[x*5+1] >> VIS_PCM_SHIFT); // L component
		y2 = VIS_Y_MID + (s_pcmbuffer[x*5] >> VIS_PCM_SHIFT);   // R component
		VertLine(iBuffer, x, y1, y2, 0xAAAAAA);
	}
}

void draw_pcm_osc_v2(int iBuffer)
{
	int x;
	int y1, y2;
	for (x = VIS_X1; x < VIS_X2; x++)
	{
		//convert fixed point int to int (the integer part is the most significant byte)
		// (fixed_point >> 8) == integer part. But I'll use >> 9 to get a range from 64 < y < 192
		y1 = VIS_Y_MID - (s_pcmbuffer[x*5] >> VIS_PCM_SHIFT); // L component
		y2 = VIS_Y_MID + (s_pcmbuffer[x*5+1] >> VIS_PCM_SHIFT);   // R component
		VertLine(iBuffer, x, y1, y2, 0xAAAAAA);
	}
}

void draw_pcm_osc_v3(int iBuffer)
{
	int x;
	int y, old_y;
	old_y = VIS_Y_MID;
	for (x = VIS_X1; x < VIS_X2; x++)
	{
		//convert fixed point int to int (the integer part is the most significant byte)
		// (fixed_point >> 8) == integer part. But I'll use >> 9 to get a range from 64 < y < 192
		y = VIS_Y_MID + (s_pcmbuffer[x*5] >> VIS_PCM_SHIFT); // L component
		VertLine(iBuffer, x, old_y, y, 0xAAAAAA);
		old_y = y;
	}
}

void draw_pcm_osc_v4(int iBuffer)//, u32 *pcmbuffer)
{
	int x;
	int y, old_y;
	old_y = VIS_Y_MID;
	static u32 prev_pcm[VIS_WIDTH];
	static bool first_time = true;
	if (first_time)
	{
		first_time = false;
	}
	else
	{
		for (x = VIS_X1; x < VIS_X2; x++)
		{
			//convert fixed point int to int (the integer part is the most significant byte)
			// (fixed_point >> 8) == integer part. But I'll use >> 9 to get a range from 64 < y < 192
			y = prev_pcm[x - VIS_X1]; // L component
			VertLine(iBuffer, x, old_y, y, 0xFFA0A0);
			old_y = y;
		}
	}
	for (x = VIS_X1; x < VIS_X2; x++)
	{
		//convert fixed point int to int (the integer part is the most significant byte)
		// (fixed_point >> 8) == integer part. But I'll use >> 9 to get a range from 64 < y < 192
		y = VIS_Y_MID + (s_pcmbuffer[x*5] >> VIS_PCM_SHIFT); // L component
		prev_pcm[x - VIS_X1] = y;
		VertLine(iBuffer, x, old_y, y, 0x0AFF0A);
		old_y = y;
	}
}

void draw_pcm_osc_v5(int iBuffer)
{
	int x;
	int yL, yR;
	int old_yL = VIS_Y_MID;
	int old_yR = VIS_Y_MID;
	for (x = VIS_X1; x < VIS_X2; x++)
	{
		//convert fixed point int to int (the integer part is the most significant byte)
		// (fixed_point >> 8) == integer part. But I'll use >> 9 to get a range from 64 < y < 192
		yL = VIS_Y_MID + (s_pcmbuffer[x*5] >> VIS_PCM_SHIFT); // L component
		yR = VIS_Y_MID + (s_pcmbuffer[x*5+1] >> VIS_PCM_SHIFT); // L component
		VertLine(iBuffer, x, old_yL, yL, 0xFF0A0A);
		VertLine(iBuffer, x, old_yR, yR, 0xA0FFA0);
		old_yL = yL;
		old_yR = yR;
	}
}


