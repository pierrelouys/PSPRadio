/*
	PSPRadio / Music streaming client for the PSP. (Initial Release: Sept. 2005)
	PSPRadio Copyright (C) 2005 Rafael Cabezas a.k.a. Raf
	TextUI3D Copyright (C) 2005 Jesper Sandberg & Raf

	This HSM implementation is based on the C version created by Jens Schwarzer.


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

#include <stdlib.h>
#include <assert.h>

#include <stdio.h>
#include <malloc.h>
#include <Logging.h>
#include <PSPApp.h>

#include "pspgu.h"
#include "pspgum.h"
#include "pspdisplay.h"
#include "pspkernel.h"
#include "psppower.h"
#include "TextUI3D.h"
#include "TextUI3D_WindowManager.h"
#include "TextUI3D_WindowManager_HSM.h"
#include "jsaTextureCache.h"
#include "TextUI3D_Panel.h"

#define		PANEL_MAX_W			312
#define		PANEL_MAX_H			168

#define		PANEL_MAX_X			64
#define		PANEL_MAX_Y			32

#define		PANEL_HIDE_Y		272
#define		PANEL_HIDE_X		480

#define		PANEL_SCALE_FLOAT	0.025f
#define		PANEL_SCALE_INT		4


#define	LOG_HSM		LOG_VERYLOW

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
#define PIXEL_SIZE (4)

/* Global texture cache */
extern jsaTextureCache		tcache;

/* Settings */
extern Settings		LocalSettings;
extern gfx_sizes	GfxSizes;

static unsigned int __attribute__((aligned(16))) gu_list[262144];

class WindowHandlerHSM *pWindowHandlerHSM = NULL;

static WindowHandlerHSM::texture_file __attribute__((aligned(16))) texture_list[] =
	{
	{WindowHandlerHSM::TEX_CORNER_UL, 		GU_PSM_8888,  16,  16, true, WindowHandlerHSM::FT_PNG, "Corner_UL.png"},
	{WindowHandlerHSM::TEX_CORNER_UR, 		GU_PSM_8888,  16,  16, true, WindowHandlerHSM::FT_PNG, "Corner_UR.png"},
	{WindowHandlerHSM::TEX_CORNER_LL, 		GU_PSM_8888,  16,  16, true, WindowHandlerHSM::FT_PNG, "Corner_LL.png"},
	{WindowHandlerHSM::TEX_CORNER_LR, 		GU_PSM_8888,  16,  16, true, WindowHandlerHSM::FT_PNG, "Corner_LR.png"},
	{WindowHandlerHSM::TEX_FRAME_T, 		GU_PSM_8888,  16,  16, true, WindowHandlerHSM::FT_PNG, "Frame_T.png"},
	{WindowHandlerHSM::TEX_FRAME_B, 		GU_PSM_8888,  16,  16, true, WindowHandlerHSM::FT_PNG, "Frame_B.png"},
	{WindowHandlerHSM::TEX_FRAME_L, 		GU_PSM_8888,  16,  16, true, WindowHandlerHSM::FT_PNG, "Frame_L.png"},
	{WindowHandlerHSM::TEX_FRAME_R, 		GU_PSM_8888,  16,  16, true, WindowHandlerHSM::FT_PNG, "Frame_R.png"},
	{WindowHandlerHSM::TEX_FILL, 			GU_PSM_8888,  16,  16, true, WindowHandlerHSM::FT_PNG, "Fill.png"},
	{WindowHandlerHSM::TEX_FONT, 			GU_PSM_8888, 512,   8, true, WindowHandlerHSM::FT_PNG, "SmallFont.png"},
	{WindowHandlerHSM::TEX_WIFI, 			GU_PSM_8888, 128,  16, true, WindowHandlerHSM::FT_PNG, "Icon_WiFi.png"},
	{WindowHandlerHSM::TEX_POWER,			GU_PSM_8888, 128,  64, true, WindowHandlerHSM::FT_PNG, "Icon_Power.png"},
	{WindowHandlerHSM::TEX_VOLUME,			GU_PSM_8888,  64, 128, true, WindowHandlerHSM::FT_PNG, "Icon_Volume.png"},
	{WindowHandlerHSM::TEX_LIST_ICON,		GU_PSM_8888,  64, 256, true, WindowHandlerHSM::FT_PNG, "Icon_Lists.png"},
	{WindowHandlerHSM::TEX_ICON_PROGRESS,	GU_PSM_8888,  128, 64, true, WindowHandlerHSM::FT_PNG, "Icon_Progress.png"},
	{WindowHandlerHSM::TEX_ICON_USB,		GU_PSM_8888,   64, 32, true, WindowHandlerHSM::FT_PNG, "Icon_USB.png"},
	{WindowHandlerHSM::TEX_ICON_PLAYSTATE,	GU_PSM_8888,   32, 32, true, WindowHandlerHSM::FT_PNG, "Icon_Playstate.png"},
	};

#define	TEXTURE_COUNT		(sizeof(texture_list) / sizeof(WindowHandlerHSM::texture_file))

static char __attribute__((aligned(16))) char_list[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
														'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5',
														'6', '7', '8', '9', ':', '.', ',', '-', '+', '(', ')', '[', ']', '?', '=', ';',
														'\\', '/', '<', '>', '!', '"', '#', '&', '$', '@', '{', '}', '*', '\'', '%', ' '};


CState::CState(CState *super, tStateHandler handler)
	{
	m_super = super;
	m_handler = handler;
	}


WindowHandlerHSM::WindowHandlerHSM() : 	top(NULL, &WindowHandlerHSM::top_handler),
										playlist(&top, &WindowHandlerHSM::playlist_handler),
										playlist_list(&playlist, &WindowHandlerHSM::playlist_list_handler),
										playlist_entries(&playlist, &WindowHandlerHSM::playlist_entries_handler),
										shoutcast(&top, &WindowHandlerHSM::shoutcast_handler),
										shoutcast_list(&shoutcast, &WindowHandlerHSM::shoutcast_list_handler),
										shoutcast_entries(&shoutcast, &WindowHandlerHSM::shoutcast_entries_handler),
										localfiles(&top, &WindowHandlerHSM::localfiles_handler),
										localfiles_list(&localfiles, &WindowHandlerHSM::localfiles_list_handler),
										localfiles_entries(&localfiles, &WindowHandlerHSM::localfiles_entries_handler),
										options(&top, &WindowHandlerHSM::options_handler)
	{
	pWindowHandlerHSM = this;
	m_HSMActive = false;
	m_mbxthread = -1;
	m_backimage = NULL;
	m_framebuffer = 0;

    m_State = 0;		/* Current state */
    m_Source = 0;		/* Source of the transition (used during transitions) */

	m_Event.Signal = 0;
	m_Event.Data = NULL;

    MakeEntry(&top);
    ActivateState(&top);
	}

WindowHandlerHSM::~WindowHandlerHSM()
	{
	int		wait_thread_count;

	m_HSMActive = false;
	/* Stop activity for message box */
	sceKernelCancelReceiveMbx(HSMMessagebox, &wait_thread_count);
	/* Wait until MbxThread has ended */
	sceKernelWaitThreadEnd(m_mbxthread, NULL);

	/* Destroy the message box */
	sceKernelDeleteMbx(HSMMessagebox);

	if (m_backimage)
		{
		free(m_backimage);
		}
	}

void WindowHandlerHSM::Initialize(char *cwd)
	{
	CTextUI3D_Panel::PanelState	panelstate;
	CTextUI3D_Panel::PanelState		*state;

	/* Create MessageBox used for sending events to the HSM */
	HSMMessagebox = sceKernelCreateMbx("HSMMBX", 0, 0);
	/* Create and start the thread handling the messages */
	m_HSMActive = true;
	m_mbxthread = sceKernelCreateThread("HSMThread", mbxThread, LocalSettings.EventThreadPrio, 8192, THREAD_ATTR_USER, 0);
	sceKernelStartThread(m_mbxthread, 0, NULL);

	LoadBackground(cwd);
	InitTextures();
	LoadTextures(cwd);

	CTextUI3D_Panel::FrameTextures	textures = {16, 16, {	TEX_CORNER_UL,	TEX_FRAME_T,	TEX_CORNER_UR,
															TEX_FRAME_L,	TEX_FILL,		TEX_FRAME_R,
															TEX_CORNER_LL,	TEX_FRAME_B,	TEX_CORNER_LR}};

	/* Set initial positions and textures for panels */
	panelstate.x		= PANEL_HIDE_X;
	panelstate.y		= PANEL_MAX_Y;
	panelstate.z		= 0;
	panelstate.w		= PANEL_MAX_W;
	panelstate.h		= PANEL_MAX_H;
	panelstate.opacity	= 0.25f;
	panelstate.scale	= 1.00f;

	for (int i = 0 ; i < PANEL_COUNT ; i++)
		{
		m_panels[i].SetFrameTexture(textures);
		m_panels[i].SetState(&panelstate);
		memcpy(&(m_panel_state[i]), &panelstate, sizeof(CTextUI3D_Panel::PanelState));
		}

	/* Options panel start from the bottom */
	state = &(m_panel_state[PANEL_OPTIONS]);
	SetHideBottom(state);

	m_progress_render = false;
	m_usb_enabled = false;
	m_list_icon = LIST_ICON_PLAYLIST;
	m_playstate_icon = PLAYSTATE_ICON_STOP;

	BatteryMapLevel(scePowerGetBatteryLifePercent());
	m_network_state = false;
	sceRtcGetCurrentClockLocalTime(&m_LastLocalTime);
	SetClock(&m_LastLocalTime);
	m_scrolloffset = LocalSettings.SongTitleWidth;

	strcpy(m_songtitle.strText, "NO TRACK PLAYING...");
	m_songtitle.x = LocalSettings.SongTitleX;
	m_songtitle.y = LocalSettings.SongTitleY;
	m_songtitle.color = LocalSettings.SongTitleColor;
}

void WindowHandlerHSM::Dispatch(int Signal, void *Data)
	{
	MbxEvent			*new_event;
	SceKernelMbxInfo	info;
	int					error;

	/* If message box is not empty and this is a vlank signal then discard it. it will be sent
	   again in 16.67 ms */
	error = sceKernelReferMbxStatus(HSMMessagebox, &info);
	if(error < 0)
		{
		Log(LOG_ERROR, "HSM_MBX : Couldn't get MBX status");
		}
	else
		{
		Log(LOG_HSM, "HSM_MBX : Status : nwt=%x, nm=%x", info.numWaitThreads, info.numMessages);
		if ( (info.numMessages == 0) ||
			((info.numMessages != 0) && (Signal != WM_EVENT_VBLANK )))
			{
			new_event = (MbxEvent *)malloc(sizeof(MbxEvent));
			if (new_event)
				{
				new_event->next = 0;
				new_event->Signal = Signal;
				new_event->Data = Data;
				Log(LOG_HSM, "HSM_MBX : Message Sent");
				sceKernelSendMbx(HSMMessagebox, new_event);
				}
			}
		}
	}

void WindowHandlerHSM::MessageHandler(int Signal, void *Data)
	{
	/* User not allowed to send control signals! */
	assert(Signal >= UserEvt);
	Log(LOG_HSM, "Dispatch event : %02X", Signal);

	m_Event.Signal = Signal;
	m_Event.Data   = Data;
	m_Source       = m_State;

	/* Dispatch the event. If not consumed then try the superstate until
	* the top-state is reached */
	do
		{
		m_Source = (CState *)((this->*m_Source->m_handler)());
		}
	while (m_Source);
	}

/* Static member used as a thread */
int WindowHandlerHSM::mbxThread(SceSize args, void *argp)
	{
	return pWindowHandlerHSM->MbxThread();
	}

/* Thread used for receiving messages from the message box and injecting them to the HSM */
int WindowHandlerHSM::MbxThread()
	{
	int			error;
	void		*event;

	Log(LOG_HSM, "HSM_MBX : Thread started");
	sceKernelDelayThread(100000);
	while (m_HSMActive)
		{
		/* Block thread until a message has arrived */
		error = sceKernelReceiveMbx(HSMMessagebox, &event, NULL);

		if(error < 0)
			{
			Log(LOG_ERROR, "HSM_MBX : Error while receiving message : %x", error);
			}
		else
			{
			MbxEvent *recv_event = (MbxEvent *)event;
			/* Pass event to MessageHandler */
			MessageHandler(recv_event->Signal, recv_event->Data);
			free(event);
			}
		}
	/* Exit thread */
	sceKernelExitThread(0);
	return 0;
	}

/* Used to inject control signals into a state implementation */
CState* WindowHandlerHSM::SendCtrlSignal(CState *State, int Signal)
	{
	int SavedSignal;
	CState *RetState;

	SavedSignal = m_Event.Signal; /* Save possible event signal */
	m_Event.Signal = Signal;

	RetState = (CState *)((this->*State->m_handler)());

	m_Event.Signal = SavedSignal; /* Restore possible event signal */

	return RetState;
	}

/* Execute the possible entry action(s) of State */
void WindowHandlerHSM::MakeEntry(CState *State)
	{
	(void)SendCtrlSignal(State, OnEntry);
	}

/* Execute the possible exit action(s) of State */
void WindowHandlerHSM::MakeExit(CState *State)
	{
	(void)SendCtrlSignal(State, OnExit);
	}

/* Set Machine in State and make possible initial transition for State */
void WindowHandlerHSM::ActivateState(CState *State)
	{
	m_State = State;
	m_Source = State; /* Record source of possible initial transition */
	(void)SendCtrlSignal(State, InitTrans);
	}

/* Returns the level of State */
int WindowHandlerHSM::GetLevel(CState *State)
	{
	int Level = 0;

	while (State->m_super)
		{
		Level++;
		State = State->m_super;
		}

	return Level;
	}

/* jschwarz: Note that the function Lca may be divided into three separate
* functions - one to handle Source is above Target, one to handle Source below
* Target and finally where Source and Target are at the same level. In this way
* less testing will be made => faster execution but implementation may be more
* complicated to understand */
void WindowHandlerHSM::Lca(CState *Source, CState *Target, int Delta)
	{
	/* Source above Target */
	if (Delta > 0)
		{
		MakeExit(Source);
		Lca(Source->m_super, Target, Delta - 1);
		}
	/* Source below Target */
	else if (Delta < 0)
		{
		Lca(Source, Target->m_super, Delta + 1);
		MakeEntry(Target);
		}
	/* Source and Target at the same level but does not match */
	else if (Source != Target)
		{
		MakeExit(Source);
		Lca(Source->m_super, Target->m_super, 0);
		MakeEntry(Target);
		}
	}

/* Transition from current state (via Source) to Target */
void WindowHandlerHSM::Trans(CState *Target)
	{
	CState *Source = m_State;

	/* If transition is not triggered by current state then exit down to the
	* triggering superstate (source of transition) */
	while (Source != m_Source)
		{
		MakeExit(Source);
		Source = Source->m_super;
		}

	/* Self-transition */
	if (Source == Target)
		{
		MakeExit(Source);
		MakeEntry(Target);
		}
	/* Other transitions */
	else
		{
		Lca(Source, Target, GetLevel(Source) - GetLevel(Target));
		}

	/* Set Machine in Target and make possible initial transition for Target */
	ActivateState(Target);
	}

void WindowHandlerHSM::InitTextures()
{
	/* Overwrite default values */
	Log(LOG_ERROR, "%d, %d, %d", GfxSizes.wifi_w, GfxSizes.wifi_h, GfxSizes.wifi_y);
	texture_list[10].width 	= GfxSizes.wifi_w;
	texture_list[10].height = GfxSizes.wifi_h;
	Log(LOG_ERROR, "%d, %d, %d", GfxSizes.power_w, GfxSizes.power_h, GfxSizes.power_y);
	texture_list[11].width 	= GfxSizes.power_w;
	texture_list[11].height = GfxSizes.power_h;
	Log(LOG_ERROR, "%d, %d, %d", GfxSizes.volume_w, GfxSizes.volume_h, GfxSizes.volume_y);
	texture_list[12].width 	= GfxSizes.volume_w;
	texture_list[12].height = GfxSizes.volume_h;
	Log(LOG_ERROR, "%d, %d, %d", GfxSizes.icons_w, GfxSizes.icons_h, GfxSizes.icons_y);
	texture_list[13].width 	= GfxSizes.icons_w;
	texture_list[13].height = GfxSizes.icons_h;
	Log(LOG_ERROR, "%d, %d, %d", GfxSizes.progress_w, GfxSizes.progress_h, GfxSizes.progress_y);
	texture_list[14].width 	= GfxSizes.progress_w;
	texture_list[14].height = GfxSizes.progress_h;
	Log(LOG_ERROR, "%d, %d, %d", GfxSizes.usb_w, GfxSizes.usb_h, GfxSizes.usb_y);
	texture_list[15].width 	= GfxSizes.usb_w;
	texture_list[15].height	= GfxSizes.usb_h;
	Log(LOG_ERROR, "%d, %d, %d", GfxSizes.playstate_w, GfxSizes.playstate_h, GfxSizes.playstate_y);
	texture_list[16].width 	= GfxSizes.playstate_w;
	texture_list[16].height	= GfxSizes.playstate_h;
	sceKernelDcacheWritebackAll();
}

/* Utility methods */
void WindowHandlerHSM::LoadTextures(char *strCWD)
{
	char								filename[MAXPATHLEN];
	unsigned char 						*filebuffer;
	jsaTextureCache::jsaTextureInfo		texture;

	/*  Load Textures to memory */
	for (unsigned int i = 0 ; i < TEXTURE_COUNT ; i++)
	{
		bool success;

		sprintf(filename, "%s/TextUI3D/%s", strCWD, texture_list[i].filename);
		filebuffer = (unsigned char *) memalign(16, (int)(texture_list[i].width * texture_list[i].height * tcache.jsaTCacheTexturePixelSize(texture_list[i].format)));

		if (texture_list[i].filetype == FT_PNG)
		{
			if (tcache.jsaTCacheLoadPngImage((const char *)filename, (u32 *)filebuffer) == -1)
				{
				Log(LOG_ERROR, "Failed loading png file: %s", filename);
				free(filebuffer);
				continue;
				}
		}
		else
		{
			if (tcache.jsaTCacheLoadRawImage((const char *)filename, (u32 *)filebuffer) == -1)
				{
				Log(LOG_ERROR, "Failed loading raw file: %s", filename);
				free(filebuffer);
				continue;
				}
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

void WindowHandlerHSM::LoadBackground(char *strCWD)
{
	char filename[MAXPATHLEN];

	sprintf(filename, "%s/TextUI3D/%s", strCWD, "BackgroundImage.png");
	m_backimage = (unsigned char *) memalign(16, SCR_WIDTH * SCR_HEIGHT * PIXEL_SIZE);

	if (m_backimage == NULL)
		{
		Log(LOG_ERROR, "Memory allocation error for background image: %s", filename);
		return;
		}

	if (tcache.jsaTCacheLoadPngImage((const char *)filename, (u32 *)m_backimage) == -1)
		{
		Log(LOG_ERROR, "Failed loading background image: %s", filename);
		free(m_backimage);
		return;
		}

	sceKernelDcacheWritebackAll();
}

void WindowHandlerHSM::UpdateWindows()
{
CTextUI3D_Panel::PanelState		*current_state;
CTextUI3D_Panel::PanelState		*target_state;

	for (int i = 0 ; i < PANEL_COUNT ; i++)
		{
		target_state = &(m_panel_state[i]);
		current_state = (CTextUI3D_Panel::PanelState *) m_panels[i].GetState();
		UpdatePanel(current_state,	target_state);
		m_panels[i].SetState(current_state);
		}

	sceKernelDcacheWritebackAll();
}

void WindowHandlerHSM::UpdateValue(int *current, int *target)
{
	if (*current != *target)
		{
		if (*current < *target)
			{
			(*current) += PANEL_SCALE_INT;
			if (*current > *target)
				{
				(*current) = *target;
				}
			}
		else
			{
			(*current) -= PANEL_SCALE_INT;
			if (*current < *target)
				{
				(*current) = *target;
				}
			}
		}
}

void WindowHandlerHSM::UpdateValue(float *current, float *target)
{
	if (*current != *target)
		{
		if (*current < *target)
			{
			*current += PANEL_SCALE_FLOAT;
			if (*current > *target)
				{
				(*current) = *target;
				}
			}
		else
			{
			*current -= PANEL_SCALE_FLOAT;
			if (*current < *target)
				{
				(*current) = *target;
				}
			}
		}
}

void WindowHandlerHSM::UpdateTextItem(list<TextItem> *current, int ID, int x, int y, char *strText, unsigned int color)
{
	TextItem					Option;
	list<TextItem>::iterator 	OptionIterator;
	bool						found = false;

	if (current->size() > 0)
	{
		for (OptionIterator = current->begin() ; OptionIterator != current->end() ; OptionIterator++)
		{
			if ((*OptionIterator).ID == ID)
			{
				strcpy((*OptionIterator).strText, strText);
				strupr((*OptionIterator).strText);
				(*OptionIterator).color = color;
				(*OptionIterator).x = x;
				(*OptionIterator).y = y;
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
		current->push_back(Option);
	}
}

void WindowHandlerHSM::UpdatePanel(CTextUI3D_Panel::PanelState *current_state,
									CTextUI3D_Panel::PanelState *target_state)
{
	UpdateValue(&current_state->x, &target_state->x);
	UpdateValue(&current_state->y, &target_state->y);
	UpdateValue(&current_state->z, &target_state->z);
	UpdateValue(&current_state->w, &target_state->w);
	UpdateValue(&current_state->h, &target_state->h);

	UpdateValue(&current_state->scale, &target_state->scale);
	UpdateValue(&current_state->opacity, &target_state->opacity);
}

void WindowHandlerHSM::SetMax(CTextUI3D_Panel::PanelState *state)
{
	state->x		= PANEL_MAX_X;
	state->y		= PANEL_MAX_Y;
	state->scale	= 1.0f;
	state->opacity	= 1.0f;
}

void WindowHandlerHSM::SetHideRight(CTextUI3D_Panel::PanelState *state)
{
	state->x		= PANEL_HIDE_X;
	state->y		= PANEL_MAX_Y;
	state->opacity	= 0.25f;
}

void WindowHandlerHSM::SetHideBottom(CTextUI3D_Panel::PanelState *state)
{
	state->x		= PANEL_MAX_X;
	state->y		= PANEL_HIDE_Y;
	state->opacity	= 0.25f;
}

void WindowHandlerHSM::RenderIcon(int IconID, int x, int y, int width, int height, int y_offset)
{
	sceGuEnable(GU_TEXTURE_2D);
	sceGuDepthFunc(GU_ALWAYS);

	sceGuAlphaFunc(GU_GREATER,0x0,0xff);
	sceGuEnable(GU_ALPHA_TEST);

	sceGuTexFilter(GU_LINEAR, GU_LINEAR);
	sceGuTexWrap(GU_CLAMP, GU_CLAMP);

	(void)tcache.jsaTCacheSetTexture(IconID);
	sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);

	struct Vertex* c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	c_vertices[0].u = 0; c_vertices[0].v = y_offset;
	c_vertices[0].x = x; c_vertices[0].y = y; c_vertices[0].z = 0;
	c_vertices[0].color = 0xFFFFFFFF;
	c_vertices[1].u = width; c_vertices[1].v = y_offset + height;
	c_vertices[1].x = x + width; c_vertices[1].y = y + height; c_vertices[1].z = 0;
	c_vertices[1].color = 0xFFFFFFFF;
	sceGuDrawArray(GU_SPRITES, GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, c_vertices);

	sceGuDisable(GU_ALPHA_TEST);
	sceGuDisable(GU_TEXTURE_2D);
	sceGuDepthFunc(GU_GEQUAL);
}

void WindowHandlerHSM::RenderIconAlpha(int IconID, int x, int y, int width, int height, int y_offset)
{
	sceGuEnable(GU_TEXTURE_2D);

	sceGuAlphaFunc( GU_GREATER, 0, 0xff );
	sceGuEnable( GU_ALPHA_TEST );

	sceGuTexFunc(GU_TFX_BLEND,GU_TCC_RGBA);
	sceGuTexEnvColor(0xFF000000);

	sceGuDepthFunc(GU_ALWAYS);

	sceGuBlendFunc( GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0 );
	sceGuEnable( GU_BLEND );

	sceGuTexWrap(GU_REPEAT, GU_REPEAT);
	sceGuTexFilter(GU_LINEAR, GU_LINEAR);

	(void)tcache.jsaTCacheSetTexture(IconID);

	struct Vertex* c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	c_vertices[0].u = 0; c_vertices[0].v = y_offset;
	c_vertices[0].x = x; c_vertices[0].y = y; c_vertices[0].z = 0;
	c_vertices[0].color = 0xFF000000;
	c_vertices[1].u = width; c_vertices[1].v = y_offset + height;
	c_vertices[1].x = x + width; c_vertices[1].y = y + height; c_vertices[1].z = 0;
	c_vertices[1].color = 0xFF000000;
	sceGuDrawArray(GU_SPRITES, GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, c_vertices);

	sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);
	sceGuDrawArray(GU_SPRITES, GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, c_vertices);

	sceGuDisable(GU_BLEND);
	sceGuDisable(GU_ALPHA_TEST);
	sceGuDisable(GU_TEXTURE_2D);
	sceGuDepthFunc(GU_GEQUAL);
}

void WindowHandlerHSM::RenderBackground()
{
static bool skip_first = true;

	if (skip_first)
		{
		skip_first = false;
		}
	else
		{
		sceGuCopyImage(GU_PSM_8888, 0, 0, 480, 272, 480, m_backimage, 0, 0, 512, (void*)(0x04000000+(u32)m_framebuffer));
		}
}

void WindowHandlerHSM::RenderNetwork()
{
	if (m_network_state == false)
		{
		RenderIcon(TEX_WIFI, LocalSettings.WifiIconX, LocalSettings.WifiIconY, GfxSizes.wifi_w, GfxSizes.wifi_y, GfxSizes.wifi_y);
		}
	else
		{
		RenderIcon(TEX_WIFI, LocalSettings.WifiIconX, LocalSettings.WifiIconY, GfxSizes.wifi_w, GfxSizes.wifi_y, 0);
		}
}

void WindowHandlerHSM::RenderUSB()
{
	if (pPSPApp->IsUSBEnabled() == false)
		{
 		RenderIconAlpha(TEX_ICON_USB, LocalSettings.USBIconX, LocalSettings.USBIconY, GfxSizes.usb_w, GfxSizes.usb_y, GfxSizes.usb_y);
		}
	else
		{
		RenderIconAlpha(TEX_ICON_USB, LocalSettings.USBIconX, LocalSettings.USBIconY, GfxSizes.usb_w, GfxSizes.usb_y, 0);
		}
}

void WindowHandlerHSM::RenderListIcon()
{
	RenderIconAlpha(TEX_LIST_ICON, LocalSettings.ListIconX, LocalSettings.ListIconY, GfxSizes.icons_w, GfxSizes.icons_y, m_list_icon * GfxSizes.icons_y);
}

void WindowHandlerHSM::RenderPlaystateIcon()
{
	RenderIconAlpha(TEX_ICON_PLAYSTATE, LocalSettings.PlayerstateX, LocalSettings.PlayerstateY, GfxSizes.playstate_w, GfxSizes.progress_y, m_playstate_icon * GfxSizes.progress_y);
}

void WindowHandlerHSM::RenderProgressBar(bool reset)
{
	static int progress_frame = 0;
	static int progress_timing = 0;

	if (reset)
		{
		progress_frame = 0;
		progress_timing = 0;
		}
	else
		{
		if (m_progress_render)
			{
			RenderIcon(TEX_ICON_PROGRESS, LocalSettings.ProgressBarX, LocalSettings.ProgressBarY, GfxSizes.progress_w, GfxSizes.progress_y, progress_frame * GfxSizes.progress_y);
			if (progress_timing == 10)
				{
				progress_timing = 0;
				progress_frame = (progress_frame+1) % 7;
				}
			else
				{
				progress_timing++;
				}
			}
		else
			{
			RenderIcon(TEX_ICON_PROGRESS, LocalSettings.ProgressBarX, LocalSettings.ProgressBarY, GfxSizes.progress_w, GfxSizes.progress_y, 7 * GfxSizes.progress_y);
			}
		}
}

void WindowHandlerHSM::BatteryMapLevel(int level)
{
	if (level == 100)
		{
		level--;
		}
	m_level  = (int)((float)level / (100.0f / 7.0f));
	m_level = 6 - m_level;
}

void WindowHandlerHSM::RenderBattery()
{
	RenderIcon(TEX_POWER, LocalSettings.BatteryIconX, LocalSettings.BatteryIconY, GfxSizes.power_w, GfxSizes.power_y, m_level*GfxSizes.power_y);
}

void WindowHandlerHSM::RenderVolume()
{
	RenderIconAlpha(TEX_VOLUME, LocalSettings.VolumeIconX, LocalSettings.VolumeIconY, GfxSizes.volume_w, GfxSizes.volume_y, 6 * GfxSizes.volume_y);
}

void WindowHandlerHSM::RenderList(list<TextItem> *current, int x_offset, int y_offset, float opacity)
{
	TextItem					Option;
	list<TextItem>::iterator 	OptionIterator;
	char 						strText[MAX_OPTION_LENGTH];

	if ((x_offset < PANEL_HIDE_X) && (y_offset < PANEL_HIDE_Y))
	{
		sceGuEnable(GU_TEXTURE_2D);

		sceGuAlphaFunc( GU_GREATER, 0, 0xff );
		sceGuEnable( GU_ALPHA_TEST );

		sceGuTexFunc(GU_TFX_BLEND,GU_TCC_RGBA);
		sceGuTexEnvColor(0xFF000000);

		sceGuDepthFunc(GU_ALWAYS);

		sceGuBlendFunc( GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0 );
		sceGuEnable( GU_BLEND );

		sceGuTexWrap(GU_REPEAT, GU_REPEAT);
		sceGuTexFilter(GU_LINEAR, GU_LINEAR);

		// setup texture
		(void)tcache.jsaTCacheSetTexture(TEX_FONT);
		sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);

		if (current->size() > 0)
		{
			for (OptionIterator = current->begin() ; OptionIterator != current->end() ; OptionIterator++)
			{
				int 				strsize;
				int 				sx, sy;
				struct TexCoord		texture_offset;
				int					color;

				Option = (*OptionIterator);

				/* Calculate opacity */
				color = Option.color & 0xFFFFFF;
				color = color | ((int)(opacity*0xFF) << 24);

				sprintf(strText, Option.strText);
				strsize = strlen(strText);
				sx = Option.x + x_offset;
				sy = Option.y + y_offset;
				struct Vertex* c_vertices = (struct Vertex*)sceGuGetMemory(strsize * 2 * sizeof(struct Vertex));
				for (int i = 0, index = 0 ; i < strsize ; i++)
					{
					char	letter = strText[i];
					FindSmallFontTexture(letter, &texture_offset);

					c_vertices[index+0].u 		= texture_offset.x1;
					c_vertices[index+0].v 		= texture_offset.y1;
					c_vertices[index+0].x 		= sx;
					c_vertices[index+0].y 		= sy;
					c_vertices[index+0].z 		= 0;
					c_vertices[index+0].color 	= color;

					c_vertices[index+1].u 		= texture_offset.x2;
					c_vertices[index+1].v 		= texture_offset.y2;
					c_vertices[index+1].x 		= sx + LocalSettings.FontWidth;
					c_vertices[index+1].y 		= sy + LocalSettings.FontHeight;
					c_vertices[index+1].z 		= 0;
					c_vertices[index+1].color 	= color;

					sx 	+= LocalSettings.FontWidth;
					index 	+= 2;
					}
				sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,strsize * 2,0,c_vertices);
			}
		}

		sceGuDisable(GU_BLEND);
		sceGuDisable(GU_ALPHA_TEST);
		sceGuDisable(GU_TEXTURE_2D);
		sceGuDepthFunc(GU_GEQUAL);
	}
}

void WindowHandlerHSM::RenderTitle()
{
	sceGuEnable(GU_TEXTURE_2D);

	sceGuAlphaFunc( GU_GREATER, 0, 0xff );
	sceGuEnable( GU_ALPHA_TEST );

	sceGuTexFunc(GU_TFX_BLEND,GU_TCC_RGBA);
	sceGuTexEnvColor(0xFF000000);

	sceGuDepthFunc(GU_ALWAYS);

	sceGuBlendFunc( GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0 );
	sceGuEnable( GU_BLEND );

	sceGuTexWrap(GU_REPEAT, GU_REPEAT);
	sceGuTexFilter(GU_LINEAR, GU_LINEAR);

	// setup texture
	(void)tcache.jsaTCacheSetTexture(TEX_FONT);
	sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);

	sceGuScissor(LocalSettings.SongTitleX - 1, LocalSettings.SongTitleY - 1, LocalSettings.SongTitleX + LocalSettings.SongTitleWidth, LocalSettings.SongTitleY + LocalSettings.FontHeight);

	if (strlen(m_songtitle.strText))
	{
		int 				strsize;
		int 				sx, sy;
		struct TexCoord		texture_offset;

		strsize = strlen(m_songtitle.strText);
		sx = m_songtitle.x + m_scrolloffset--;
		if (m_scrolloffset == -strsize*LocalSettings.FontWidth)
			{
			m_scrolloffset = LocalSettings.SongTitleWidth;
			}
		sy = m_songtitle.y;

		struct Vertex* c_vertices = (struct Vertex*)sceGuGetMemory(strsize * 2 * sizeof(struct Vertex));
		for (int i = 0, index = 0 ; i < strsize ; i++)
			{
			char	letter = m_songtitle.strText[i];
			FindSmallFontTexture(letter, &texture_offset);

			c_vertices[index+0].u 		= texture_offset.x1;
			c_vertices[index+0].v 		= texture_offset.y1;
			c_vertices[index+0].x 		= sx;
			c_vertices[index+0].y 		= sy;
			c_vertices[index+0].z 		= 0;
			c_vertices[index+0].color 	= m_songtitle.color;

			c_vertices[index+1].u 		= texture_offset.x2;
			c_vertices[index+1].v 		= texture_offset.y2;
			c_vertices[index+1].x 		= sx + LocalSettings.FontWidth;
			c_vertices[index+1].y 		= sy + LocalSettings.FontHeight;
			c_vertices[index+1].z 		= 0;
			c_vertices[index+1].color 	= m_songtitle.color;

			sx 	+= LocalSettings.FontWidth;
			index 	+= 2;
			}
		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,strsize * 2,0,c_vertices);
	}

	sceGuScissor(0,0,480,272);

	sceGuDisable(GU_BLEND);
	sceGuDisable(GU_ALPHA_TEST);
	sceGuDisable(GU_TEXTURE_2D);
	sceGuDepthFunc(GU_GEQUAL);
}

void WindowHandlerHSM::FindSmallFontTexture(char index, struct TexCoord *texture_offset)
{
	int maxsize = sizeof(::char_list);
	bool found_char = false;

	texture_offset->x1 = 0;
	texture_offset->y1 = 0;
	texture_offset->x2 = LocalSettings.FontWidth;
	texture_offset->y2 = LocalSettings.FontHeight;

	for (int i = 0 ; i < maxsize ; i++)
	{
		if (::char_list[i] == index)
		{
		found_char = true;
		break;
		}
		texture_offset->x1 += LocalSettings.FontWidth;
		texture_offset->x2 += LocalSettings.FontWidth;
	}

	/* If we didn't find a matching character then return the index of the last char (space) */
	if (!found_char)
	{
		texture_offset->x1 -= LocalSettings.FontWidth;
		texture_offset->x2 -= LocalSettings.FontWidth;
	}
}

void WindowHandlerHSM::SetClock(pspTime *current_time)
{
	char clock[64];

	if (LocalSettings.ClockFormat == 24)
		{
		sprintf(clock, "%02d:%02d", current_time->hour, current_time->minutes);
		}
	else
		{
		bool bIsPM = (current_time->hour)>12;
		sprintf(clock, "%02d:%02d %s", bIsPM?(current_time->hour-12):(current_time->hour), current_time->minutes, bIsPM?"PM":"AM");
		}
	UpdateTextItem(&m_StaticTextItems, WM_EVENT_TIME, LocalSettings.ClockX, LocalSettings.ClockY, clock, 0xFFFFFFFF);
	m_LastLocalTime = *current_time;
}

void WindowHandlerHSM::SetBuffer(long unsigned int percentage)
{
	char value[64];

	sprintf(value, "%03lu%%", percentage);
	UpdateTextItem(&m_StaticTextItems, WM_EVENT_BUFFER, LocalSettings.BufferX, LocalSettings.BufferY, value, 0xFFFFFFFF);
}

void WindowHandlerHSM::SetBitrate(long unsigned int bitrate)
{
	char value[64];

	sprintf(value, "%03lu", bitrate);
	UpdateTextItem(&m_StaticTextItems, WM_EVENT_BITRATE, LocalSettings.BitrateX, LocalSettings.BitrateY, value, 0xFFFFFFFF);
}

void WindowHandlerHSM::GUInit()
{
	Log(LOG_HSM, "GUInit");
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
}

void WindowHandlerHSM::GUInitDisplayList()
{
	sceGuStart(GU_DIRECT,::gu_list);

	sceGuClearColor(0x00335588);
	sceGuClearDepth(0);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);

	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadIdentity();
	sceGumPerspective(75.0f,16.0f/9.0f,0.5f,1000.0f);

	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();
}

void WindowHandlerHSM::GUEndDisplayList()
{
	sceGuFinish();
	sceGuSync(0,0);

	m_framebuffer = sceGuSwapBuffers();
}

/* State handlers and transition-actions */

void *WindowHandlerHSM::top_handler()
	{

	switch (m_Event.Signal)
		{
		/* VB event */
		case WM_EVENT_VBLANK:
			GUInitDisplayList();
			RenderBackground();
			/* Render static text*/
			RenderList(&m_StaticTextItems, 0, 0, 1.0f);
			RenderTitle();
			/* Render Icons */
			RenderNetwork();
			RenderBattery();
			RenderVolume();
			RenderListIcon();
			RenderUSB();
			RenderPlaystateIcon();
			/* Update panel positions */
			UpdateWindows();
			/* Render panels etc. */
			for (int i = 0 ; i < PANEL_COUNT ; i++)
			{
				if ((m_panels[i].GetPositionX() < PANEL_HIDE_X) && (m_panels[i].GetPositionY() < PANEL_HIDE_Y))
				{
					m_panels[i].Render();
				}
			}
			/* Render text for windows */
			RenderList(&m_OptionItems, 	m_panels[PANEL_OPTIONS].GetPositionX(),	m_panels[PANEL_OPTIONS].GetPositionY(), m_panels[PANEL_OPTIONS].GetOpacity());
			RenderList(&m_PlaylistContainer, m_panels[PANEL_PLAYLIST_LIST].GetPositionX(), m_panels[PANEL_PLAYLIST_LIST].GetPositionY(), m_panels[PANEL_PLAYLIST_LIST].GetOpacity());
			RenderList(&m_PlaylistEntries, m_panels[PANEL_PLAYLIST_ENTRIES].GetPositionX(), m_panels[PANEL_PLAYLIST_ENTRIES].GetPositionY(), m_panels[PANEL_PLAYLIST_ENTRIES].GetOpacity());
			RenderList(&m_ShoutcastContainer, m_panels[PANEL_SHOUTCAST_LIST].GetPositionX(), m_panels[PANEL_SHOUTCAST_LIST].GetPositionY(), m_panels[PANEL_SHOUTCAST_LIST].GetOpacity());
			RenderList(&m_ShoutcastEntries, m_panels[PANEL_SHOUTCAST_ENTRIES].GetPositionX(), m_panels[PANEL_SHOUTCAST_ENTRIES].GetPositionY(), m_panels[PANEL_SHOUTCAST_ENTRIES].GetOpacity());
			RenderList(&m_LocalfilesContainer, m_panels[PANEL_LOCALFILES_LIST].GetPositionX(), m_panels[PANEL_LOCALFILES_LIST].GetPositionY(), m_panels[PANEL_LOCALFILES_LIST].GetOpacity());
			RenderList(&m_LocalfilesEntries, m_panels[PANEL_LOCALFILES_ENTRIES].GetPositionX(), m_panels[PANEL_LOCALFILES_ENTRIES].GetPositionY(), m_panels[PANEL_LOCALFILES_ENTRIES].GetOpacity());
			/* Render progressbar if necessary */
			RenderProgressBar(false);
			GUEndDisplayList();
			return 0;

		/* Static text events */
		case WM_EVENT_TEXT_SONGTITLE:
			Log(LOG_HSM, "TOP:Event WM_EVENT_TEXT_SONGTITLE");
			m_scrolloffset = LocalSettings.SongTitleWidth;
			memcpy(&m_songtitle, (char *) m_Event.Data, sizeof(TextItem));
			free(m_Event.Data);
			return 0;
		case WM_EVENT_TEXT_TITLE:
			Log(LOG_HSM, "TOP:Event WM_EVENT_TEXT_TITLE");
			UpdateTextItem(&m_StaticTextItems, WM_EVENT_TEXT_TITLE, LocalSettings.VersionX, LocalSettings.VersionY, (char *) m_Event.Data, 0xFFFFFFFF);
			return 0;
		case WM_EVENT_TEXT_ERROR:
			Log(LOG_HSM, "TOP:Event WM_EVENT_TEXT_ERROR");
			return 0;

		/* Stream events */
		case WM_EVENT_STREAM_START:
			Log(LOG_HSM, "TOP:Event WM_EVENT_STREAM_START");
			return 0;
		case WM_EVENT_STREAM_OPEN:
			Log(LOG_HSM, "TOP:Event WM_EVENT_STREAM_OPEN");
			m_progress_render = true;
			RenderProgressBar(true);
			return 0;
		case WM_EVENT_STREAM_CONNECTING:
			Log(LOG_HSM, "TOP:Event WM_EVENT_STREAM_CONNECTING");
			return 0;
		case WM_EVENT_STREAM_ERROR:
			Log(LOG_HSM, "TOP:Event WM_EVENT_STREAM_ERROR");
			m_progress_render = false;
			return 0;
		case WM_EVENT_STREAM_SUCCESS:
			Log(LOG_HSM, "TOP:Event WM_EVENT_STREAM_SUCCESS");
			m_progress_render = false;
			return 0;

		/* Network events */
		case WM_EVENT_NETWORK_ENABLE:
			Log(LOG_HSM, "TOP:Event WM_EVENT_NETWORK_ENABLE");
			UpdateTextItem(&m_StaticTextItems, WM_EVENT_NETWORK_IP, LocalSettings.IPX, LocalSettings.IPY, " ", 0xFFFFFFFF);
			m_network_state = false;
			m_progress_render = true;
			RenderProgressBar(true);
			return 0;
		case WM_EVENT_NETWORK_DISABLE:
			Log(LOG_HSM, "TOP:Event WM_EVENT_NETWORK_DISABLE");
			UpdateTextItem(&m_StaticTextItems, WM_EVENT_NETWORK_IP, LocalSettings.IPX, LocalSettings.IPY, " ", 0xFFFFFFFF);
			m_network_state = false;
			return 0;
		case WM_EVENT_NETWORK_IP:
			Log(LOG_HSM, "TOP:Event WM_EVENT_NETWORK_IP");
			UpdateTextItem(&m_StaticTextItems, WM_EVENT_NETWORK_IP, LocalSettings.IPX, LocalSettings.IPY, (char *) m_Event.Data, 0xFFFFFFFF);
			m_network_state = true;
			m_progress_render = false;
			return 0;

		/* Player events */
		case WM_EVENT_PLAYER_STOP:
			Log(LOG_HSM, "TOP:Event WM_EVENT_PLAYER_STOP");
			strcpy(m_songtitle.strText, "NO TRACK PLAYING...");
			UpdateTextItem(&m_StaticTextItems, WM_EVENT_BUFFER, LocalSettings.BufferX, LocalSettings.BufferY, " ", 0xFFFFFFFF);
			UpdateTextItem(&m_StaticTextItems, WM_EVENT_BITRATE, LocalSettings.BitrateX, LocalSettings.BitrateY, " ", 0xFFFFFFFF);
			m_playstate_icon = PLAYSTATE_ICON_STOP;
			return 0;
		case WM_EVENT_PLAYER_PAUSE:
			Log(LOG_HSM, "TOP:Event WM_EVENT_PLAYER_PAUSE");
			m_playstate_icon = PLAYSTATE_ICON_PAUSE;
			return 0;
		case WM_EVENT_PLAYER_START:
			Log(LOG_HSM, "TOP:Event WM_EVENT_PLAYER_START");
			m_playstate_icon = PLAYSTATE_ICON_PLAY;
			return 0;

		/*  Misc events */
		case WM_EVENT_TIME:
			Log(LOG_HSM, "TOP:Event WM_EVENT_TIME");
			SetClock((pspTime *)(m_Event.Data));
			return 0;
		case WM_EVENT_BATTERY:
			{
			Log(LOG_HSM, "TOP:Event WM_EVENT_BATTERY");
			BatteryMapLevel((int )(m_Event.Data));
			}
			return 0;
		case WM_EVENT_BUFFER:
			Log(LOG_HSM, "TOP:Event WM_EVENT_BUFFER");
			SetBuffer((long unsigned int)(m_Event.Data));
			return 0;
		case WM_EVENT_BITRATE:
			Log(LOG_HSM, "TOP:Event WM_EVENT_BITRATE");
			SetBitrate((long unsigned int)(m_Event.Data));
			return 0;
		case WM_EVENT_USB_ENABLE:
			Log(LOG_HSM, "TOP:Event WM_EVENT_USB_ENABLE");
			m_usb_enabled = true;
			return 0;
		case WM_EVENT_USB_DISABLE:
			Log(LOG_HSM, "TOP:Event WM_EVENT_USB_DISABLE");
			m_usb_enabled = false;
			return 0;
		case WM_EVENT_GU_INIT:
			Log(LOG_HSM, "TOP:Event WM_EVENT_GU_INIT");
			GUInit();
			return 0;

		/* List events */
		case WM_EVENT_PLAYLIST:
			Log(LOG_HSM, "TOP:Event WM_EVENT_PLAYLIST");
			Trans(&playlist);
			return 0;
		case WM_EVENT_SHOUTCAST:
			Log(LOG_HSM, "TOP:Event WM_EVENT_SHOUTCAST");
			Trans(&shoutcast);
			return 0;
		case WM_EVENT_LOCALFILES:
			Log(LOG_HSM, "TOP:Event WM_EVENT_LOCALFILES");
			Trans(&localfiles);
			return 0;
		case WM_EVENT_OPTIONS:
			Log(LOG_HSM, "TOP:Event WM_EVENT_OPTIONS");
			Trans(&options);
			return 0;
		}
	return NULL;
	}

void *WindowHandlerHSM::playlist_handler()
	{
	CTextUI3D_Panel::PanelState		*state;

	switch (m_Event.Signal)
		{
		case OnEntry:
			{
			Log(LOG_HSM, "PLAYLIST:Entry");
			m_list_icon = LIST_ICON_PLAYLIST;
			state = &(m_panel_state[PANEL_PLAYLIST_LIST]);
			SetHideRight(state);
			state = &(m_panel_state[PANEL_PLAYLIST_ENTRIES]);
			SetHideRight(state);
			return 0;
			}

		case OnExit:
			{
			Log(LOG_HSM, "PLAYLIST:Exit");
			state = &(m_panel_state[PANEL_PLAYLIST_LIST]);
			SetHideBottom(state);
			state = &(m_panel_state[PANEL_PLAYLIST_ENTRIES]);
			SetHideBottom(state);
			return 0;
			}

		case WM_EVENT_LIST_CLEAR:
			Log(LOG_HSM, "PLAYLIST:Event WM_EVENT_LIST_CLEAR");
			{
			while(m_PlaylistContainer.size())
				{
				m_PlaylistContainer.pop_front();
				}
			}
			return 0;
		case WM_EVENT_LIST_TEXT:
			Log(LOG_HSM, "PLAYLIST:Event WM_EVENT_LIST_TEXT");
			{
			TextItem	text;

			memcpy(&text, m_Event.Data, sizeof(TextItem));
			m_PlaylistContainer.push_back(text);
			free(m_Event.Data);
			}
			return 0;

		case WM_EVENT_ENTRY_CLEAR:
			Log(LOG_HSM, "PLAYLIST:Event WM_EVENT_ENTRY_CLEAR");
			{
			while(m_PlaylistEntries.size())
				{
				m_PlaylistEntries.pop_front();
				}
			}
			return 0;
		case WM_EVENT_ENTRY_TEXT:
			Log(LOG_HSM, "PLAYLIST:Event WM_EVENT_ENTRY_TEXT");
			{
			TextItem	text;

			memcpy(&text, m_Event.Data, sizeof(TextItem));
			m_PlaylistEntries.push_back(text);
			free(m_Event.Data);
			}
			return 0;

		case WM_EVENT_SELECT_ENTRIES:
			Log(LOG_HSM, "PLAYLIST:Event WM_EVENT_SELECT_ENTRIES");
			Trans(&playlist_entries);
			return 0;

		case WM_EVENT_SELECT_LIST:
			Log(LOG_HSM, "PLAYLIST:Event WM_EVENT_SELECT_LIST");
			Trans(&playlist_list);
			return 0;
		}
		return &top;
	}

void *WindowHandlerHSM::playlist_list_handler()
	{
	CTextUI3D_Panel::PanelState		*state;

	switch (m_Event.Signal)
		{
		case OnEntry:
			{
			Log(LOG_HSM, "PLAYLIST_LIST:Entry");
			state = &(m_panel_state[PANEL_PLAYLIST_LIST]);
			SetMax(state);
			return 0;
			}

		case OnExit:
			{
			Log(LOG_HSM, "PLAYLIST_LIST:Exit");
			state = &(m_panel_state[PANEL_PLAYLIST_LIST]);
			SetHideRight(state);
			return 0;
			}

		case WM_EVENT_SELECT_ENTRIES:
			Log(LOG_HSM, "PLAYLIST_LIST:Event WM_EVENT_SELECT_ENTRIES");
			Trans(&playlist_entries);
			return 0;

		case WM_EVENT_SELECT_LIST:
			return 0;
		}
		return &playlist;
	}

void *WindowHandlerHSM::playlist_entries_handler()
	{
	CTextUI3D_Panel::PanelState		*state;

	switch (m_Event.Signal)
		{
		case OnEntry:
			{
			Log(LOG_HSM, "PLAYLIST_ENTRIES:Entry");
			state = &(m_panel_state[PANEL_PLAYLIST_ENTRIES]);
			SetMax(state);
			return 0;
			}

		case OnExit:
			{
			Log(LOG_HSM, "PLAYLIST_ENTRIES:Exit");
			state = &(m_panel_state[PANEL_PLAYLIST_ENTRIES]);
			SetHideRight(state);
			return 0;
			}

		case WM_EVENT_SELECT_LIST:
			Log(LOG_HSM, "PLAYLIST_ENTRIES:Event WM_EVENT_SELECT_LIST");
			Trans(&playlist_list);
			return 0;

		case WM_EVENT_SELECT_ENTRIES:
			return 0;
		}
		return &playlist;
	}

void *WindowHandlerHSM::shoutcast_handler()
	{
	CTextUI3D_Panel::PanelState		*state;

	switch (m_Event.Signal)
		{
		case OnEntry:
			{
			Log(LOG_HSM, "SHOUTCAST:Entry");
			m_list_icon = LIST_ICON_SHOUTCAST;
			state = &(m_panel_state[PANEL_SHOUTCAST_LIST]);
			SetHideRight(state);
			state = &(m_panel_state[PANEL_SHOUTCAST_ENTRIES]);
			SetHideRight(state);
			return 0;
			}

		case OnExit:
			{
			Log(LOG_HSM, "SHOUTCAST:Exit");
			state = &(m_panel_state[PANEL_SHOUTCAST_LIST]);
			SetHideBottom(state);
			state = &(m_panel_state[PANEL_SHOUTCAST_ENTRIES]);
			SetHideBottom(state);
			return 0;
			}

		case WM_EVENT_LIST_CLEAR:
			Log(LOG_HSM, "SHOUTCAST:Event WM_EVENT_LIST_CLEAR");
			{
			while(m_ShoutcastContainer.size())
				{
				m_ShoutcastContainer.pop_front();
				}
			}
			return 0;
		case WM_EVENT_LIST_TEXT:
			Log(LOG_HSM, "SHOUTCAST:Event WM_EVENT_LIST_TEXT");
			{
			TextItem	text;

			memcpy(&text, m_Event.Data, sizeof(TextItem));
			m_ShoutcastContainer.push_back(text);
			free(m_Event.Data);
			}
			return 0;

		case WM_EVENT_ENTRY_CLEAR:
			Log(LOG_HSM, "SHOUTCAST:Event WM_EVENT_ENTRY_CLEAR");
			{
			while(m_ShoutcastEntries.size())
				{
				m_ShoutcastEntries.pop_front();
				}
			}
			return 0;
		case WM_EVENT_ENTRY_TEXT:
			Log(LOG_HSM, "SHOUTCAST:Event WM_EVENT_ENTRY_TEXT");
			{
			TextItem	text;

			memcpy(&text, m_Event.Data, sizeof(TextItem));
			m_ShoutcastEntries.push_back(text);
			free(m_Event.Data);
			}
			return 0;

		case WM_EVENT_SELECT_ENTRIES:
			Log(LOG_HSM, "SHOUTCAST:Event WM_EVENT_SELECT_ENTRIES");
			Trans(&shoutcast_entries);
			return 0;

		case WM_EVENT_SELECT_LIST:
			Log(LOG_HSM, "SHOUTCAST:Event WM_EVENT_SELECT_LIST");
			Trans(&shoutcast_list);
			return 0;
		}
		return &top;
	}

void *WindowHandlerHSM::shoutcast_list_handler()
	{
	CTextUI3D_Panel::PanelState		*state;

	switch (m_Event.Signal)
		{
		case OnEntry:
			{
			Log(LOG_HSM, "SHOUTCAST_LIST:Entry");
			state = &(m_panel_state[PANEL_SHOUTCAST_LIST]);
			SetMax(state);
			return 0;
			}

		case OnExit:
			{
			Log(LOG_HSM, "SHOUTCAST_LIST:Exit");
			state = &(m_panel_state[PANEL_SHOUTCAST_LIST]);
			SetHideRight(state);
			return 0;
			}

		case WM_EVENT_SELECT_ENTRIES:
			Log(LOG_HSM, "SHOUTCAST_LIST:Event WM_EVENT_SELECT_ENTRIES");
			Trans(&shoutcast_entries);
			return 0;
		case WM_EVENT_SELECT_LIST:
			return 0;
		}
		return &shoutcast;
	}

void *WindowHandlerHSM::shoutcast_entries_handler()
	{

	CTextUI3D_Panel::PanelState		*state;

	switch (m_Event.Signal)
		{
		case OnEntry:
			{
			Log(LOG_HSM, "SHOUTCAST_ENTRIES:Entry");
			state = &(m_panel_state[PANEL_SHOUTCAST_ENTRIES]);
			SetMax(state);
			return 0;
			}

		case OnExit:
			{
			Log(LOG_HSM, "SHOUTCAST_ENTRIES:Exit");
			state = &(m_panel_state[PANEL_SHOUTCAST_ENTRIES]);
			SetHideRight(state);
			return 0;
			}

		case WM_EVENT_SELECT_LIST:
			Log(LOG_HSM, "SHOUTCAST_ENTRIES:Event WM_EVENT_SELECT_LIST");
			Trans(&shoutcast_list);
			return 0;

		case WM_EVENT_SELECT_ENTRIES:
			return 0;
		}
		return &shoutcast;
	}

void *WindowHandlerHSM::localfiles_handler()
	{
	CTextUI3D_Panel::PanelState		*state;

	switch (m_Event.Signal)
		{
		case OnEntry:
			{
			Log(LOG_HSM, "LOCALFILES:Entry");
			m_list_icon = LIST_ICON_LOCALFILES;
			state = &(m_panel_state[PANEL_LOCALFILES_LIST]);
			SetHideRight(state);
			state = &(m_panel_state[PANEL_LOCALFILES_ENTRIES]);
			SetHideRight(state);
			return 0;
			}

		case OnExit:
			{
			Log(LOG_HSM, "LOCALFILES:Exit");
			state = &(m_panel_state[PANEL_LOCALFILES_LIST]);
			SetHideBottom(state);
			state = &(m_panel_state[PANEL_LOCALFILES_ENTRIES]);
			SetHideBottom(state);
			return 0;
			}

		case WM_EVENT_LIST_CLEAR:
			Log(LOG_HSM, "LOCALFILES:Event WM_EVENT_LIST_CLEAR");
			{
			while(m_LocalfilesContainer.size())
				{
				m_LocalfilesContainer.pop_front();
				}
			}
			return 0;
		case WM_EVENT_LIST_TEXT:
			Log(LOG_HSM, "LOCALFILES:Event WM_EVENT_LIST_TEXT");
			{
			TextItem	text;

			memcpy(&text, m_Event.Data, sizeof(TextItem));
			m_LocalfilesContainer.push_back(text);
			free(m_Event.Data);
			}
			return 0;

		case WM_EVENT_ENTRY_CLEAR:
			Log(LOG_HSM, "LOCALFILES:Event WM_EVENT_ENTRY_CLEAR");
			{
			while(m_LocalfilesEntries.size())
				{
				m_LocalfilesEntries.pop_front();
				}
			}
			return 0;
		case WM_EVENT_ENTRY_TEXT:
			Log(LOG_HSM, "LOCALFILES:Event WM_EVENT_ENTRY_TEXT");
			{
			TextItem	text;

			memcpy(&text, m_Event.Data, sizeof(TextItem));
			m_LocalfilesEntries.push_back(text);
			free(m_Event.Data);
			}
			return 0;

		case WM_EVENT_SELECT_ENTRIES:
			Log(LOG_HSM, "LOCALFILES:Event WM_EVENT_SELECT_ENTRIES");
			Trans(&localfiles_entries);
			return 0;

		case WM_EVENT_SELECT_LIST:
			Log(LOG_HSM, "LOCALFILES:Event WM_EVENT_SELECT_LIST");
			Trans(&localfiles_list);
			return 0;
		}
		return &top;
	}

void *WindowHandlerHSM::localfiles_list_handler()
	{
	CTextUI3D_Panel::PanelState		*state;

	switch (m_Event.Signal)
		{
		case OnEntry:
			{
			Log(LOG_HSM, "LOCALFILES_LIST:Entry");
			state = &(m_panel_state[PANEL_LOCALFILES_LIST]);
			SetMax(state);
			return 0;
			}

		case OnExit:
			{
			Log(LOG_HSM, "LOCALFILES_LIST:Exit");
			state = &(m_panel_state[PANEL_LOCALFILES_LIST]);
			SetHideRight(state);
			return 0;
			}

		case WM_EVENT_SELECT_ENTRIES:
			Log(LOG_HSM, "LOCALFILES_LIST:Event WM_EVENT_SELECT_ENTRIES");
			Trans(&localfiles_entries);
			return 0;
		case WM_EVENT_SELECT_LIST:
			return 0;
		}
		return &localfiles;
	}

void *WindowHandlerHSM::localfiles_entries_handler()
	{
	CTextUI3D_Panel::PanelState		*state;

	switch (m_Event.Signal)
		{
		case OnEntry:
			{
			Log(LOG_HSM, "LOCALFILES_ENTRIES:Entry");
			state = &(m_panel_state[PANEL_LOCALFILES_ENTRIES]);
			SetMax(state);
			return 0;
			}

		case OnExit:
			{
			Log(LOG_HSM, "LOCALFILES_ENTRIES:Exit");
			state = &(m_panel_state[PANEL_LOCALFILES_ENTRIES]);
			SetHideRight(state);
			return 0;
			}

		case WM_EVENT_SELECT_LIST:
			Log(LOG_HSM, "LOCALFILES_ENTRIES:Event WM_EVENT_SELECT_LIST");
			Trans(&localfiles_list);
			return 0;
		case WM_EVENT_SELECT_ENTRIES:
			return 0;
		}
		return &localfiles;
	}

void *WindowHandlerHSM::options_handler()
	{
	CTextUI3D_Panel::PanelState		*state;

	switch (m_Event.Signal)
		{
		case OnEntry:
			Log(LOG_HSM, "OPTIONS:Entry");
			m_list_icon = LIST_ICON_OPTIONS;
			state = &(m_panel_state[PANEL_OPTIONS]);
			SetMax(state);
			return 0;
		case OnExit:
			Log(LOG_HSM, "OPTIONS:Exit");
			state = &(m_panel_state[PANEL_OPTIONS]);
			SetHideBottom(state);
			return 0;
		case WM_EVENT_OPTIONS_CLEAR:
			Log(LOG_HSM, "OPTIONS:Event WM_EVENT_OPTIONS_CLEAR");
			{
			while(m_OptionItems.size())
				{
				m_OptionItems.pop_front();
				}
			}
			return 0;
		case WM_EVENT_OPTIONS_TEXT:
			Log(LOG_HSM, "OPTIONS:Event WM_EVENT_OPTIONS_TEXT");
			{
			TextItem	text;
			memcpy(&text, m_Event.Data, sizeof(TextItem));
			m_OptionItems.push_back(text);
			free(m_Event.Data);
			}
			return 0;
		}
		return &top;
	}