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

#ifndef _TEXTUI3D_WINDOWMANAGER_HSM_
#define _TEXTUI3D_WINDOWMANAGER_HSM_

#include <psprtc.h>
#include <pspthreadman.h>
#include "TextUI3D_Panel.h"
#include <unistd.h>
#include <list>

class WindowHandlerHSM;

typedef void *(WindowHandlerHSM::*tStateHandler)();

class CState
	{
	CState			*m_super;

public:
	tStateHandler	m_handler;
	CState(CState *super, tStateHandler handler);

	friend	class WindowHandlerHSM;
	};


class WindowHandlerHSM
	{
public:
	WindowHandlerHSM();
	~WindowHandlerHSM();

	/* Dispatch an event to a Machine. If an event is already being processed then
	 * the event is enqueued */
	void Dispatch(int signal, void *data);

	void Initialize(char *cwd);

public:
	enum hsm_events
	{
	/* Note: The value of zero is reserved for internal use! */
	OnEntry = -3,	/* To execute possible entry action */
	OnExit,			/* To execute possible exit action */
	InitTrans,		/* To make the possible initial transition */
	UserEvt,		/* Users should enumerate their events starting from here */
	};

	enum texture_enum
	{
		TEX_CORNER_UL = 1,
		TEX_CORNER_UR,
		TEX_CORNER_LL,
		TEX_CORNER_LR,
		TEX_FRAME_T,
		TEX_FRAME_B,
		TEX_FRAME_L,
		TEX_FRAME_R,
		TEX_FILL,
		TEX_FONT,
		TEX_WIFI,
		TEX_POWER,
		TEX_VOLUME,
		TEX_LIST_ICON,
		TEX_ICON_PROGRESS,
		TEX_ICON_USB,
		TEX_ICON_PLAYSTATE,
		TEX_ERRORNOTE,
	};

	enum filetype_enum
	{
		FT_RAW,
		FT_PNG,
	};

	typedef struct texture_file
	{
		int		ID;
		int		format;
		int		width;
		int		height;
		bool	swizzle;
		int		filetype;
		char	filename[MAXPATHLEN];
	};

	typedef struct TextItem
	{
		int				ID;
		int  			x, y;
		unsigned int	color;
		char 			strText[MAXPATHLEN];
	};

private:

	typedef struct eEvent
		{
		int				Signal;
		void			*Data;
		} tEvent, *pEvent;

	typedef struct _MbxEvent
		{
		struct _MbxEvent	*next;
		int					Signal;
		void				*Data;
		} MbxEvent;

	typedef struct Vertex
	{
		float u, v;
		unsigned int color;
		float x,y,z;
	};

	struct TexCoord
	{
		int  		x1, y1;
		int  		x2, y2;
	};

	typedef enum error_events
	{
		ERROR_EVENT_SHOW,
		ERROR_EVENT_HIDE,
		ERROR_EVENT_RENDER,
	};

	typedef enum error_states
	{
		ERROR_STATE_SHOW,
		ERROR_STATE_HIDE,
		ERROR_STATE_SHOWING,
		ERROR_STATE_HIDING,
	};

protected:
	/* Transition from current state (via Source) to Target */
	void Trans(CState *target);
	/* Used to inject control signals into a state implementation */
	CState *SendCtrlSignal(CState *State, int Signal);
	/* Execute the possible entry action(s) of State */
	void MakeEntry(CState *State);
	/* Execute the possible exit action(s) of State */
	void MakeExit(CState *State);
	/* Set Machine in State and make possible initial transition for State */
	void ActivateState(CState *State);
	/* Returns the level of State */
	int GetLevel(CState *State);
	/* Find Least Common Anchester */
	void Lca(CState *Source, CState *Target, int Delta);

/* State handlers and transition-actions */
private:
	void *top_handler();
	void *playlist_handler();
	void *playlist_list_handler();
	void *playlist_entries_handler();
	void *shoutcast_handler();
	void *shoutcast_list_handler();
	void *shoutcast_entries_handler();
	void *localfiles_handler();
	void *localfiles_list_handler();
	void *localfiles_entries_handler();
	void *options_handler();

	void InitTextures();
	void LoadTextures(char *strCWD);
	void LoadBackground(char *strCWD);
	void UpdateWindows();
	void UpdateValue(int *current, int *target);
	void UpdateValue(float *current, float *target);
	void UpdatePanel(CTextUI3D_Panel::PanelState *current_state, CTextUI3D_Panel::PanelState *target_state);
	void SetMax(CTextUI3D_Panel::PanelState *state);
	void SetHideRight(CTextUI3D_Panel::PanelState *state);
	void SetHideBottom(CTextUI3D_Panel::PanelState *state);
	void SetError(char *errorStr);

	void UpdateTextItem(list<TextItem> *current, int ID, int x, int y, char *strText, unsigned int color);
	void FindSmallFontTexture(char index, struct TexCoord *texture_offset);
	void RenderList(list<TextItem> *current, int x_offset, int y_offset, float opacity);

	void RenderIcon(int IconID, int x, int y, int width, int height, int x_offset);
	void RenderIconAlpha(int IconID, int x, int y, int width, int height, int x_offset);
	void RenderNetwork();
	void RenderBattery();
	void RenderListIcon();
	void RenderVolume();
	void BatteryMapLevel(int level);
	void SetClock(pspTime *current_time);
	void RenderProgressBar(bool reset);
	void RenderTitle();
	void RenderUSB();
	void SetBuffer(long unsigned int percentage);
	void SetBitrate(long unsigned int bitrate);
	void RenderPlaystateIcon();
	void RenderBackground();
	void RenderError(WindowHandlerHSM::error_events event);

	void GUInit();
	void GUInitDisplayList();
	void GUEndDisplayList();

	/* Message Box handling */
	void MessageHandler(int Signal, void *Data);
	static int mbxThread(SceSize args, void *argp);
	int MbxThread();

	enum panel_enum
	{
		PANEL_PLAYLIST_LIST,
		PANEL_PLAYLIST_ENTRIES,
		PANEL_LOCALFILES_LIST,
		PANEL_LOCALFILES_ENTRIES,
		PANEL_SHOUTCAST_LIST,
		PANEL_SHOUTCAST_ENTRIES,
		PANEL_OPTIONS,
		PANEL_COUNT
	};

	enum list_icons
	{
		LIST_ICON_OPTIONS,
		LIST_ICON_LOCALFILES,
		LIST_ICON_SHOUTCAST,
		LIST_ICON_PLAYLIST,
	};

	enum playstate_icons
	{
		PLAYSTATE_ICON_PAUSE,
		PLAYSTATE_ICON_STOP,
		PLAYSTATE_ICON_PLAY,
	};


private:
    CState	*m_State;	/* Current state */
    CState	*m_Source;	/* Source of the transition (used during transitions) */
    tEvent	m_Event;	/* Current event */
	u8		*m_backimage;
	void*	m_framebuffer;

	CTextUI3D_Panel					m_panels[PANEL_COUNT];
	CTextUI3D_Panel::PanelState		m_panel_state[PANEL_COUNT];

	list<TextItem>					m_StaticTextItems;
	list<TextItem>					m_OptionItems;
	list<TextItem>					m_PlaylistContainer;
	list<TextItem>					m_PlaylistEntries;
	list<TextItem>					m_ShoutcastContainer;
	list<TextItem>					m_ShoutcastEntries;
	list<TextItem>					m_LocalfilesContainer;
	list<TextItem>					m_LocalfilesEntries;
	list<TextItem>					m_ErrorList;
	TextItem						m_songtitle;

	bool							m_network_state;
	bool							m_usb_enabled;
	int								m_level;
	pspTime							m_LastLocalTime;
	bool							m_option_render;
	bool							m_progress_render;
	int								m_list_icon;
	int								m_playstate_icon;
	int								m_scrolloffset;

	SceUID							HSMMessagebox;
	SceUID							m_mbxthread;
	bool							m_HSMActive;
	bool							m_HSMInitialized;

private:
	CState	top;
	CState	playlist, playlist_list, playlist_entries;
	CState	shoutcast, shoutcast_list, shoutcast_entries;
	CState	localfiles, localfiles_list, localfiles_entries;
	CState	options;
	};

#endif
