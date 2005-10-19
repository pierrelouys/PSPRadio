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

struct char_map
{
	char	char_index;
	float	min_x;
	float	min_y;
	float	max_x;
	float	max_y;
};

enum fx_list_enum
{
	FX_CUBES,
	FX_HEART
};

enum icon_list_enum
{
	ICON_NETWORK,
	ICON_LOAD,
	ICON_SOUND,
	ICON_PLAY,
	ICON_STOP
};

typedef struct IconStr
{
	float		x1, y1;
	float		x2, y2;
	unsigned int	color;
	unsigned char	*texture;
};

class CSandbergUI : public IPSPRadio_UI
{
public:
	CSandbergUI();
	virtual ~CSandbergUI();
	
public:
	int Initialize(char *strCWD);
	void Terminate();

	int SetTitle(char *strTitle);
	int DisplayMessage_EnablingNetwork();
	int DisplayMessage_DisablingNetwork();
	int DisplayMessage_NetworkReady(char *strIP);
	int DisplayMessage_NetworkSelection(int iProfileID, char *strProfileName);
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
	virtual	int DisplayPLEntries(CPlayList *PlayList);

private:

	struct char_map	* CSandbergUI::FindCharMap(char index);

	void CSandbergUI::InitPL(void);
	void CSandbergUI::InitFX(void);

	void CSandbergUI::RenderFX(void);
	void CSandbergUI::RenderFX_1(void);
	void CSandbergUI::RenderFX_2(void);
	void CSandbergUI::RenderLogo(void);
	void CSandbergUI::RenderCommands(void);
	void CSandbergUI::RenderPL(void);
	void CSandbergUI::RenderState(void);
	void CSandbergUI::RenderPLName(void);
	void CSandbergUI::RenderPLEntry(void);
	void CSandbergUI::RenderNetwork(void);
	void CSandbergUI::RenderLoad(void);
	void CSandbergUI::RenderSound(void);
	void CSandbergUI::RenderIcon(IconStr *icon_info);

private:
	char*	pl_name;
	char*	pl_entry;
	void* 	framebuffer;
	float 	start, curr;
	struct	timeval tval;
	int	current_fx;
};



#endif
