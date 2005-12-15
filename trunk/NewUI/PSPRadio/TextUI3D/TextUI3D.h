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
#ifndef _PSPRADIOTEXTUI3D_
#define _PSPRADIOTEXTUI3D_

#include "IPSPRadio_UI.h"
#include "jsaVRAMManager.h"
#include "jsaTextureCache.h"


class CTextUI3D : public IPSPRadio_UI
{
public:
	enum texture_enum
	{
		TEX_BACKGROUND,
		TEX_CORNER,
		TEX_VERTICAL,
		TEX_HORIZONTAL,
		TEX_FILL,
		TEX_FONT,
	};

	typedef struct texture_file
	{
		int		ID;
		int		format;
		int		width;
		int		height;
		bool	swizzle;
		char	filename[MAXPATHLEN];
	};

	struct NCVertex
	{
		float u, v;
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

	struct TexCoord
	{
		int  		x1, y1;
		int  		x2, y2;
	};

public:
	CTextUI3D();
	~CTextUI3D();
	
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
	int OnNewSongData(MetaData *pData);
	void DisplayContainers(CMetaDataContainer *Container);
	void DisplayElements(CMetaDataContainer *Container);
	void OnCurrentContainerSideChange(CMetaDataContainer *Container);
	void OnTimeChange(pspTime *LocalTime);
	void OnBatteryChange(int Percentage);

	/** Screen Handling */
	void Initialize_Screen(CScreenHandler::Screen screen);
	void UpdateOptionsScreen(list<OptionsScreen::Options> &OptionsList, 
										 list<OptionsScreen::Options>::iterator &CurrentOptionIterator);
	void OnScreenshot(CScreenHandler::ScreenShotState state);

private:
	enum screen_state_enum
	{
		SCREEN_PLAYING,
		SCREEN_OPTIONS
	};

	enum text_enum
	{
		TEXT_STATIC,
		TEXT_PLAYLIST,
		TEXT_LOCAL_LIST,
		TEXT_OPTIONS,
	};

	struct StoredOptionItem
	{
		int				ID;
		int  			x, y;
		unsigned int	color;
		char 			strText[MAXPATHLEN];
	};

	void *LoadFile(char *filename);
	void UpdateTextItem(int ID, int x, int y, char *strText, unsigned int color);
	void LoadTextures(char *strCWD);
	int FindFirstEntry(int list_cnt, int current);
	void RenderFrame1(void);
	void RenderFrame2(void);

private:
	jsaTextureCache						tcache;
	list<StoredOptionItem>				OptionsItems;
	CScreenHandler::ScreenShotState 	m_state;
	unsigned char						*backimage;

	void* 	framebuffer;
	int		screen_state;
};

#endif
