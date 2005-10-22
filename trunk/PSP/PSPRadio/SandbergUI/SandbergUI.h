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
#ifndef _PSPRADIOSANDBERGUI_
#define _PSPRADIOSANDBERGUI_

#include "IPSPRadio_UI.h"
#include "jsaTextureCache.h"



class CSandbergUI : public IPSPRadio_UI
{
public:
	enum texture_enum
	{
		TEX_LOGO,
		TEX_COMMANDS,
		TEX_PLATE,
		TEX_FONT_LARGE,
		TEX_FONT_SMALL,
		TEX_PLAY,
		TEX_STOP,
		TEX_NETWORK,
		TEX_LOAD,
		TEX_SOUND,
		TEX_OPTIONS
	};

	enum fx_list_enum
	{
		FX_CUBES,
		FX_HEART
	};

	typedef struct texture_file
	{
		int	ID;
		int	format;
		int	width;
		int	height;
		char	filename[MAXPATHLEN];
	};

	typedef struct IconStr
	{
		float		x1, y1;
		float		x2, y2;
		unsigned int	color;
		int		ID;
	};

	struct NCVertex
	{
		float u, v;
		unsigned int color;
		float nx,ny,nz;
		float x,y,z;
	};

	struct NVertex
	{
		unsigned int color;
		float nx,ny,nz;
		float x,y,z;
	};

	struct Vertex
	{
		float u, v;
		unsigned int color;
		float x,y,z;
	};

	struct SVertex
	{
		float x,y,z;
	};

	struct TexCoord
	{
		int  		x1, y1;
		int  		x2, y2;
	};

public:
	CSandbergUI();
	~CSandbergUI();
	
public:
	int Initialize(char *strCWD);
	void Terminate();

	int SetTitle(char *strTitle);
	int DisplayMessage_EnablingNetwork();
	int DisplayMessage_DisablingNetwork();
	int DisplayMessage_NetworkReady(char *strIP);
	int DisplayMainCommands();
	int DisplayActiveCommand(CPSPSound::pspsound_state playingstate);
	int DisplayErrorMessage(char *strMsg);
	int DisplayBufferPercentage(int a);

	/** these are listed in sequential order */
	int OnNewStreamStarted();
	int OnStreamOpening();
	int OnConnectionProgress();
	int OnStreamOpeningError();
	int OnStreamOpeningSuccess();
	int OnVBlank();
	int OnNewSongData(CPSPSoundStream::MetaData *pData);
	int DisplayPLList(CDirList *plList);
	int DisplayPLEntries(CPlayList *PlayList);
	int OnCurrentPlayListSideSelectionChange(CScreenHandler::PlayListSide CurrentPlayListSideSelection);

	/** Screen Handling */
	void Initialize_Screen(CScreenHandler::Screen screen);
	void UpdateOptionsScreen(list<CScreenHandler::Options> &OptionsList, 
							list<CScreenHandler::Options>::iterator &CurrentOptionIterator);

private:
	enum screen_state_enum
	{
		SCREEN_PLAYING,
		SCREEN_OPTIONS
	};

	enum render_option_enum
	{
		RENDER_OPTIONS,
		RENDER_METADATA
	};

	enum icon_list_enum
	{
		ICON_NETWORK,
		ICON_LOAD,
		ICON_SOUND,
		ICON_PLAY,
		ICON_STOP
	};

	enum text_enum
	{
		TEXT_OPTION,
		TEXT_IP,
		TEXT_ERROR,
		TEXT_BUFFER,
		TEXT_STREAM_FORMAT,
		TEXT_STREAM_URL,
		TEXT_STREAM_NAME,
		TEXT_STREAM_ARTIST,
	};

	struct StoredOptionItem
	{
		int		ID;
		int  		x, y;
		unsigned int	color;
		char 		strText[MAXPATHLEN];
	};

	void *LoadFile(char *filename);
	void FindSmallFontTexture(char index, struct TexCoord *texture_offset);
	void UpdateTextItem(int ID, int x, int y, char *strText);
	void LoadTextures(char *strCWD);

	void InitFX(void);

	void RenderFX(void);
	void RenderFX_1(void);
	void RenderFX_2(void);
	void RenderLogo(void);
	void RenderCommands(void);
	void RenderPL(void);
	void RenderState(void);
	void RenderScroller(char *text, struct NCVertex *vertices, int rotate, float x_offset);
	void RenderNetwork(void);
	void RenderLoad(void);
	void RenderSound(void);
	void RenderIcon(IconStr *icon_info);
	void RenderFrame(TexCoord &area, unsigned int tex_color);
	void RenderActiveList(void);

	void RenderPlayScreen(void);
	void RenderOptionScreen(void);
	void RenderOptionLogo(void);
	void StoreOption(int y, bool active_item, char *strName, char *strStates[], int iNumberOfStates, int iSelectedState, int iActiveState);
	void RenderOptions(int render_option);

private:
	jsaTextureCache		tcache;
	list<StoredOptionItem>	OptionsItems;

	char*	pl_name;
	char*	pl_entry;
	void* 	framebuffer;
	float 	start, curr;
	struct	timeval tval;
	int	current_fx;
	int	screen_state;
	int	select_state, select_target;
};

#endif
