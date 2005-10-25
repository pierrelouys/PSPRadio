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
#include "GraphicsUITheme.h"
//#include "Logging.h"
//#include "Tools.h"

#ifdef WIN32
	#include <windows.h>
#else
	#include <stdarg.h>
#endif


#define CURRENT_VERSION "0.2"

StringPosType g_StringPosArray[] =
{
	{ "filetitle", {0,0,0,0}, true, JUST_LEFT, 0 },
	{ "uri", {0,0,0,0}, true, JUST_LEFT, 0 },
	{ "url", {0,0,0,0}, true, JUST_LEFT, 0 },
	{ "samplerate", {0,0,0,0}, true, JUST_LEFT, 0 },
	{ "mpeglayer", {0,0,0,0}, true, JUST_LEFT, 0 },
	{ "genre", {0,0,0,0}, true, JUST_LEFT, 0 },
	{ "songauthor", {0,0,0,0}, true, JUST_LEFT, 0 },
	{ "length", {0,0,0,0}, true, JUST_LEFT, 0 },
	{ "bitrate", {0,0,0,0}, true, JUST_LEFT, 0 },
	{ "channels", {0,0,0,0}, true, JUST_LEFT, 0 },
	{ "error", {0,0,0,0}, true, JUST_LEFT, 0 },
};

ButtonPosType g_ButtonPosArray[] =
{
	{ "play", NULL, {0,0,0,0}, 0, true, 2 },
	{ "pause", NULL, {0,0,0,0}, 0, true, 2  },
	{ "stop", NULL, {0,0,0,0}, 0, true, 2  },
	{ "load", NULL, {0,0,0,0}, 0, true, 2  },
	{ "sound", NULL, {0,0,0,0}, 0, true, 2  },
	{ "volume", NULL, {0,0,0,0}, 0, true, -1 },
	{ "buffer", NULL, {0,0,0,0}, 0, true, -1 },
	{ "network", NULL, {0,0,0,0}, 0, true, -1 },
	{ "stream", NULL, {0,0,0,0}, 0, true, -1 }
};

OutputAreaType g_OutputAreaArray[] =
{
	{ "playlist", -1, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, false, true, 0 },
	{ "playlistitem", -1, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, false, true, 0 },
	{ "settings", 1, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, false, true, 0 }
};

//*****************************************************************************
// 
//*****************************************************************************
//
//*****************************************************************************
CGraphicsUITheme::CGraphicsUITheme() : m_pIniTheme(NULL)
{
	m_nColorDepth = 32;
	m_nPSPResWidth = 480;
	m_nPSPResHeight = 272;
	m_nFlags = SDL_HWSURFACE;
	m_pPSPSurface = NULL;
	m_pImageSurface = NULL;
	m_szMsg = (char *)malloc(256);
}

//*****************************************************************************
// 
//*****************************************************************************
//
//
//*****************************************************************************
CGraphicsUITheme::~CGraphicsUITheme()
{
	Terminate();
}

//*****************************************************************************
// 
//*****************************************************************************
//
//
//*****************************************************************************
int CGraphicsUITheme::Initialize(char *szThemeFileName, bool bFullScreen)
{	
	// Check to see if we have already initialized INI
	if(NULL != m_pIniTheme)
	{
		LogError("Initialize: ERROR m_pIniTheme already initiaized");
		return -1;
	}
	
	if(true == bFullScreen)
	{
 		m_nFlags |= SDL_FULLSCREEN;
	}	
	
	// Create INI object	
	m_pIniTheme = new CIniParser(szThemeFileName);
		
	// Check to make sure it was created succesfully
	if(NULL == m_pIniTheme)
	{
		LogError("Initialize: ERROR unabled to allocate m_pIniTheme");
		return -1;
	}

	// Initialize SDL
	if(0 > SDL_Init(SDL_INIT_VIDEO))
	{
		LogError("Initialize: ERROR Initializing SDL [%s]", SDL_GetError());
		return -1;
	}

	// Enable the cursor
	SDL_ShowCursor(SDL_DISABLE);

	// Check Video Mode
	m_nColorDepth = SDL_VideoModeOK(m_nPSPResWidth,
									m_nPSPResHeight,
									m_nColorDepth,
									m_nFlags);

	// Set Video Mode
	m_pPSPSurface = SDL_SetVideoMode(m_nPSPResWidth,
										m_nPSPResHeight,
										m_nColorDepth,
										m_nFlags);

	// Make sure the screen was created
	if(NULL == m_pPSPSurface)
	{
		LogError("Initialize: ERROR Initializing m_pPSPSurface [%s]", SDL_GetError());
		return -1;
	}

	// TODO: Get Version and make sure it is compatable

	// Get the base theme image
	m_pImageSurface = SDL_LoadBMP(m_pIniTheme->GetString("main:themeimage", "--"));

	if(NULL == m_pImageSurface)
	{
		LogError("Initialize: ERROR Initializing m_pImageSurface [%s]", SDL_GetError());
		return -1;
	}

	// Load the fonts
	if(-1 == GetFonts())
	{
		LogError("Initialize: ERROR Initializing fonts");
		return -1;
	}

	// Load String Positions
	if(-1 == GetStringPos())
	{
		LogError("Initialize: ERROR Initializing string positions");
		return -1;
	}

	// Get Button Positions
	if(-1 == GetButtonPos())
	{
		LogError("Initialize: ERROR Initializing button positions");
		return -1;
	}

	// Get Output Areas
	if(-1 == GetOutputAreaPos())
	{
		Log(LOG_ERROR, "Initialize: ERROR Initializing output area positions");
		return -1;
	}

	// Get Transparency color
	if(-1 == GetIniColor("main:transparency", &m_TransparencyColor))
	{
		Log(LOG_ERROR, "Initialize: ERROR getting transparency color using default");
		m_TransparencyColor.r = 255;
		m_TransparencyColor.g = 0;
		m_TransparencyColor.b = 255;
	}

	SDL_SetColorKey(m_pPSPSurface, SDL_SRCCOLORKEY, SDL_MapRGB(m_pPSPSurface->format, m_TransparencyColor.r, m_TransparencyColor.g, m_TransparencyColor.b)); 
	SDL_SetColorKey(m_pImageSurface, SDL_SRCCOLORKEY, SDL_MapRGB(m_pImageSurface->format, m_TransparencyColor.r, m_TransparencyColor.g, m_TransparencyColor.b)); 

	return 0;
}

//*****************************************************************************
// 
//*****************************************************************************
//
//
//*****************************************************************************
void CGraphicsUITheme::Terminate()
{
	// Free up the button src rects
	for(int x = 0; x < BP_ITEM_COUNT; x++)
	{
		SAFE_FREE(g_ButtonPosArray[x].pSrcRect);
	}

	SAFE_DELETE(m_pIniTheme);
	SAFE_FREE_SURFACE(m_pImageSurface);
	SAFE_FREE_SURFACE(m_pPSPSurface);
	SAFE_FREE(m_szMsg);
	SDL_Quit();
}

//*****************************************************************************
// 
//*****************************************************************************
//
//
//*****************************************************************************
int CGraphicsUITheme::GetLineCount(OutputAreaEnum posEnum)
{
	return g_OutputAreaArray[posEnum].nLineCount;
}

//*****************************************************************************
// 
//*****************************************************************************
//
//
//*****************************************************************************
int CGraphicsUITheme::GetButtonStateCount(ButtonPosEnum posEnum)
{
	return g_ButtonPosArray[posEnum].nButtonCount;
}

//*****************************************************************************
// 
//*****************************************************************************
//
//
//*****************************************************************************
void CGraphicsUITheme::DisplayMainScreen()
{
	SDL_Rect rectSrc;

	if(0 == GetIniRect("screens:main", &rectSrc))
	{
		SDL_BlitSurface(m_pImageSurface, &rectSrc, m_pPSPSurface, NULL);
	}
}

//*****************************************************************************
// 
//*****************************************************************************
//
//
//*****************************************************************************
void CGraphicsUITheme::DisplayShoutcastScreen()
{
	SDL_Rect rectSrc;

	if(0 == GetIniRect("screens:shoutcast", &rectSrc))
	{
		SDL_BlitSurface(m_pImageSurface, &rectSrc, m_pPSPSurface, NULL);
	}
}

//*****************************************************************************
// 
//*****************************************************************************
//
//
//*****************************************************************************
void CGraphicsUITheme::DisplaySettingScreen()
{
	SDL_Rect rectSrc;

	if(0 == GetIniRect("screens:settings", &rectSrc))
	{
		SDL_BlitSurface(m_pImageSurface, &rectSrc, m_pPSPSurface, NULL);
	}
}

void CGraphicsUITheme::OnVBlank()
{
	SDL_Flip(m_pPSPSurface);
}

//*****************************************************************************
// 
//*****************************************************************************
//
//
//*****************************************************************************
int CGraphicsUITheme::GetIniRect(char *szIniTag, SDL_Rect *pRect)
{
	char szRect[30];

	if(NULL == pRect)
	{
		LogError("GetIniRect: ERROR pRect is NULL [%s]", szIniTag);
		return -1;
	}

	strcpy(szRect, m_pIniTheme->GetString(szIniTag, ""));

	if(0 == strlen(szRect))
	{
		LogError("GetIniRect: ERROR error getting ini tag [%s]", szIniTag);		
		return -1;
	}

	return StringToRect(szRect, pRect);
}

//*****************************************************************************
// 
//*****************************************************************************
//
//
//*****************************************************************************
int CGraphicsUITheme::GetIniStringPos(char *szIniTag, StringPosType *pPos)
{
	char szPos[50];

	if(NULL == pPos)
	{
		LogError("GetIniRect: ERROR pPos is NULL [%s]", szIniTag);
		return -1;
	}

	strcpy(szPos, m_pIniTheme->GetString(szIniTag, ""));

	if(0 == strlen(szPos))
	{
		LogError("GetIniRect: ERROR error getting ini tag [%s]", szIniTag);		
		return -1;
	}

	return StringToStringPos(szPos, pPos);
}


//*****************************************************************************
// 
//*****************************************************************************
//
//
//*****************************************************************************
int CGraphicsUITheme::GetIniColor(char *szIniTag, SDL_Color *pColor)
{
	char szColor[30];
	int r, g, b;

	if(NULL == pColor)
	{
		LogError("GetIniColor: ERROR pColor is NULL [%s]", szIniTag);
		return -1;
	}

	strcpy(szColor, m_pIniTheme->GetString(szIniTag, ""));

	if(0 == strlen(szColor))
	{
		LogError("GetIniColor: ERROR error getting ini tag [%s]", szIniTag);		
		return -1;
	}
	
	if(-1 == StringToPoint(szColor, &r, &g, &b))
	{
		LogError("GetIniColor: ERROR error parsing color [%s]", szIniTag);		
		return -1;
	}
	else
	{
		pColor->r = r;
		pColor->g = g;
		pColor->b = b;
	}

	return 0;
}

//*****************************************************************************
// Parse [item1,item2,item3]
//*****************************************************************************
//
//
//*****************************************************************************
int CGraphicsUITheme::StringToPoint(char *szPair, int *pItem1, int *pItem2, int *pItem3)
{
	int rc; 
	int nTemp1;
	int nTemp2;
	int nTemp3;
	
	if((NULL == pItem1) || (NULL == pItem2)|| (NULL == pItem3))
	{
		LogError("StringToPoint: ERROR pItem1 or pItem2 is NULL [%s]", szPair);
		return -1;
	}
	
	rc = sscanf(szPair, "[%d,%d,%d]", &nTemp1, &nTemp2, &nTemp3);		
	
	if(3 == rc)
	{
		*pItem1 = nTemp1;
		*pItem2 = nTemp2;
		*pItem3 = nTemp3;

		return 0;
	}
	else
	{
		LogError("StringToPoint: sscanf failed [%s]", szPair);
	}
	
	return -1;
}

//*****************************************************************************
// Parse [item1,item2]
//*****************************************************************************
//
//
//*****************************************************************************
int CGraphicsUITheme::StringToPoint(char *szPair, int *pItem1, int *pItem2)
{
	int rc; 
	int nTemp1;
	int nTemp2;
	
	if((NULL == pItem1) || (NULL == pItem2))
	{
		LogError("StringToPoint: ERROR pItem1 or pItem2 is NULL [%s]", szPair);
		return -1;
	}
	
	rc = sscanf(szPair, "[%d,%d]", &nTemp1, &nTemp2);		
	
	if(2 == rc)
	{
		*pItem1 = nTemp1;
		*pItem2 = nTemp2;
		return 0;
	}
	else
	{
		LogError("StringToPoint: sscanf failed [%s]", szPair);
	}
	
	return -1;
}

//*****************************************************************************
// Parse [xpos,ypos] [width,height]
//*****************************************************************************
//
//
//*****************************************************************************
int CGraphicsUITheme::StringToRect(char *szRect, SDL_Rect *pSdlRect)
{
	char *token = strtok(szRect, " ");

	int nCount = 0;
	int nTemp1;
	int nTemp2;
	
	if(NULL == pSdlRect)
	{
		LogError("StringToRect: ERROR pSdlRect is NULL [%s]", szRect);
		return -1;
	}
	
	if(NULL != token)
	{
		// Get the x and y pos
		if(0 == StringToPoint(token, &nTemp1, &nTemp2))
		{
			pSdlRect->x = nTemp1;
			pSdlRect->y = nTemp2;
			nCount++;			
		}	
		else
		{
			LogError("StringToRect: ERROR getting StringToPoint 1 for [%s]", szRect);
		}
		
		token = strtok(NULL, "");
		
		if(NULL != token)
		{
			// Get the width and height
			if(0 == StringToPoint(token, &nTemp1, &nTemp2))
			{
				pSdlRect->w = nTemp1;
				pSdlRect->h = nTemp2;
				nCount++;			
			}	
			else
			{
				LogError("StringToRect: ERROR getting StringToPoint 2 for [%s]", szRect);
			}
		}	
		else
		{
			LogError("StringToRect: ERROR token is null 2 for [%s]", szRect);
		}
	}
	else
	{
		LogError("StringToRect: ERROR token is null 1 for [%s]", szRect);
	}

	if(2 == nCount)
	{
		return 0;
	}
	
	return -1;
}

//*****************************************************************************
// Parse [xpos,ypos] [width,height] JUST FONTINDEX
//*****************************************************************************
//
//
//*****************************************************************************
int CGraphicsUITheme::StringToStringPos(char *szPos, StringPosType *pPos)
{
	char *token = NULL;
	int nCount = 0;
	int nTemp1;
	int nTemp2;
	
	if(NULL == pPos)
	{
		LogError("StringToStringPos: ERROR pPos is NULL [%s]", szPos);
		return -1;
	}
	
	// Get the x and y pos
	token = strtok(szPos, " ");
	if(NULL != token)
	{
		if(0 == StringToPoint(token, &nTemp1, &nTemp2))
		{
			pPos->rectPos.x = nTemp1;
			pPos->rectPos.y = nTemp2;
			nCount++;			
		}	
		else
		{
			LogError("StringToStringPos: ERROR getting StringToPoint 1 for [%s]", szPos);
			return -1;
		}
	}
	else
	{
		LogError("StringToRect: ERROR token is null 1 for [%s]", szPos);
		return -1;
	}
		
	// Get the width and height
	token = strtok(NULL, " ");		
	if(NULL != token)
	{
		if(0 == StringToPoint(token, &nTemp1, &nTemp2))
		{
			pPos->rectPos.w = nTemp1;
			pPos->rectPos.h = nTemp2;
			nCount++;			
		}	
		else
		{
			LogError("StringToStringPos: ERROR getting StringToPoint 2 for [%s]", szPos);
			return -1;
		}
	}
	else
	{
		LogError("StringToRect: ERROR token is null 2 for [%s]", szPos);
		return -1;
	}

	// Get the Justification
	token = strtok(NULL, " ");
	if(NULL != token)
	{
		if(0 == strcmp(token, "CENTER"))
		{
			pPos->fontJust = JUST_CENTER;
		}
		else if(0 == strcmp(token, "RIGHT"))
		{
			pPos->fontJust = JUST_RIGHT;
		}
		else
		{
			pPos->fontJust = JUST_LEFT;
		}
	}
	else
	{
		LogError("StringToStringPos: ERROR getting StringToPoint 2 for [%s]", szPos);
		return -1;
	}

	// Get font index
	token = strtok(NULL, "");
	if(NULL != token)
	{
		pPos->nFontIndex = atoi(token);
	}
	else
	{
		LogError("StringToStringPos: ERROR getting StringToPoint 2 for [%s]", szPos);
		return -1;
	}

	return 0;
}


//*****************************************************************************
// 
//*****************************************************************************
//
//
//*****************************************************************************
int CGraphicsUITheme::GetFonts()
{
	int nFontCount = m_pIniTheme->GetInteger("font:fontCount", -1);

	if(-1 == nFontCount)
	{
		LogError("GetFonts: ERROR no font items found");
		return -1;
	}

	for(int x = 0; x < nFontCount; x++)
	{
		char szFontTitle[20];

		char szFontItem[40];
		sprintf(szFontTitle, "font%d", x+1);

		sprintf(szFontItem, "%s:%s", szFontTitle, "lineCount");
		int nLineCount = m_pIniTheme->GetInteger(szFontItem, -1);
		
		if(-1 == nLineCount)
		{
			LogError("GetFonts: ERROR no lines found for %s", szFontTitle);
			return -1;
		}

		// Clear the font rect map
		m_FontMap[x].clear();
		
		for(int y = 0; y < nLineCount; y++)
		{
			char szValue[50];
			SDL_Rect fontSrcRect;

			sprintf(szFontItem, "%s:srcLine%d", szFontTitle, y+1);

			if(0 != GetIniRect(szFontItem, &fontSrcRect))
			{
				LogError("GetFonts: ERROR getting ini rect [%s]", szValue);
				return -1;
			}

			// Copy size into our font size array
			if(0 == y)
			{
				memcpy(&m_FontSize[x], &fontSrcRect, sizeof(SDL_Rect));
			}

			sprintf(szFontItem, "%s:keyLine%d", szFontTitle, y+1);
			strcpy(szValue, m_pIniTheme->GetString(szFontItem, ""));
			for(size_t z = 0; z != strlen(szValue); z++)
			{
				SDL_Rect rectItem;
				memcpy(&rectItem, &fontSrcRect, sizeof(SDL_Rect));

				// Update XPos for current font
				rectItem.x = fontSrcRect.x + (fontSrcRect.w * z);

				// Add to font map
				m_FontMap[x][szValue[z]] = rectItem;                
			}
		}
	}

	return 0;
}

//*****************************************************************************
// 
//*****************************************************************************
//
//
//*****************************************************************************
int CGraphicsUITheme::GetStringPos()
{
	for(int x = 0; x < SP_ITEM_COUNT; x++)
	{
		char szIniTag[50];
		StringPosType tmpStringPos;

		sprintf(szIniTag, "stringpos:%s", g_StringPosArray[x].szIniName);

		if(0 != GetIniStringPos(szIniTag, &tmpStringPos))
		{
			LogError("GetFonts: WARNING string pos [%s] not found disabling", szIniTag);
			g_StringPosArray[x].bEnabled = false;
		}
		else
		{
			g_StringPosArray[x].bEnabled = true;
			g_StringPosArray[x].fontJust = tmpStringPos.fontJust;
			g_StringPosArray[x].nFontIndex = tmpStringPos.nFontIndex-1;
			memcpy(&g_StringPosArray[x].rectPos, &tmpStringPos.rectPos, sizeof(SDL_Rect));
		}
	}

	return 0;
}

//*****************************************************************************
// 
//*****************************************************************************
//
//
//*****************************************************************************
int CGraphicsUITheme::GetButtonPos()
{
	char szIniTag[50];
	SDL_Rect tmpButtonPos;

	// Get the regular buttons
	for(int x = 0; x < BP_ITEM_COUNT; x++)
	{
		// Allocate the Src State Buttons
		if(-1 != g_ButtonPosArray[x].nButtonCount)
		{
			g_ButtonPosArray[x].pSrcRect = (SDL_Rect*)malloc(sizeof(SDL_Rect) * g_ButtonPosArray[x].nButtonCount);
		}
		else
		{
			// Get On Pos
			sprintf(szIniTag, "%s:stateCount", g_ButtonPosArray[x].szIniName);
			g_ButtonPosArray[x].nButtonCount = m_pIniTheme->GetInteger(szIniTag, -1);
			
			if(-1 == g_ButtonPosArray[x].nButtonCount)
			{
				LogError("GetButtonPos: ERROR unable to find state count for %s", szIniTag);
				g_ButtonPosArray[x].bEnabled = false;
				continue;
			}

			g_ButtonPosArray[x].pSrcRect = (SDL_Rect*)malloc(sizeof(SDL_Rect) * g_ButtonPosArray[x].nButtonCount);

		}

		// Get the src rects
		for(int y = 0; y < g_ButtonPosArray[x].nButtonCount; y++)
		{
			// Get On Pos
			sprintf(szIniTag, "%s:src%d", g_ButtonPosArray[x].szIniName,y+1);
			if(0 != GetIniRect(szIniTag, &tmpButtonPos))
			{
				LogError("GetButtonPos: ERROR getting on pos [%s] disabling", szIniTag);
				g_ButtonPosArray[x].bEnabled = false;
				break;
			}
			memcpy(&g_ButtonPosArray[x].pSrcRect[y], &tmpButtonPos, sizeof(SDL_Rect));
		}

		// If we did not disable to button above get the dst position
		if(true == g_ButtonPosArray[x].bEnabled)
		{
			// Get Dst Pos
			sprintf(szIniTag, "%s:dst", g_ButtonPosArray[x].szIniName);
			if(0 != GetIniRect(szIniTag, &tmpButtonPos))
			{
				LogError("GetButtonPos: ERROR getting dst pos [%s] disabling", szIniTag);
				g_ButtonPosArray[x].bEnabled = false;
				continue;
			}
			memcpy(&g_ButtonPosArray[x].dstRect, &tmpButtonPos, sizeof(SDL_Rect));
			g_ButtonPosArray[x].bEnabled = true;
		}
	}

	return 0;
}

//*****************************************************************************
// 
//*****************************************************************************
//
//
//*****************************************************************************
int CGraphicsUITheme::GetOutputAreaPos()
{
	char szIniTag[50];
	SDL_Rect tmpButtonPos;

	// Get the regular buttons
	for(int x = 0; x < OA_ITEM_COUNT; x++)
	{
		// Get Line Count
		sprintf(szIniTag, "%s:lineCount", g_OutputAreaArray[x].szIniName);
		g_OutputAreaArray[x].nLineCount = m_pIniTheme->GetInteger(szIniTag, -1);
		
		if(-1 == g_OutputAreaArray[x].nLineCount)
		{
			LogError("GetOutputAreaPos: ERROR getting %s", szIniTag);
			g_OutputAreaArray[x].bEnabled = false;
			continue;
		}
		
		sprintf(szIniTag, "%s:font", g_OutputAreaArray[x].szIniName);
		g_OutputAreaArray[x].nFontIndex = m_pIniTheme->GetInteger(szIniTag, -1);		
		
		if(-1 == g_OutputAreaArray[x].nFontIndex)
		{
			LogError("GetOutputAreaPos: ERROR getting %s", szIniTag);
			g_OutputAreaArray[x].bEnabled = false;
			continue;
		}
		else
		{
			g_OutputAreaArray[x].nFontIndex = g_OutputAreaArray[x].nFontIndex - 1;
		}

		// Get Line Size
		sprintf(szIniTag, "%s:lineSize", g_OutputAreaArray[x].szIniName);
		if(0 != GetIniRect(szIniTag, &tmpButtonPos))
		{
			LogError("GetOutputAreaPos: ERROR getting on pos [%s] disabling", szIniTag);
			g_OutputAreaArray[x].bEnabled = false;
			continue;
		}
		memcpy(&g_OutputAreaArray[x].lineSize, &tmpButtonPos, sizeof(SDL_Rect));


		// Get Src Pos
		sprintf(szIniTag, "%s:src", g_OutputAreaArray[x].szIniName);
		if(0 != GetIniRect(szIniTag, &tmpButtonPos))
		{
			LogError("GetOutputAreaPos: ERROR getting on pos [%s] disabling", szIniTag);
			g_OutputAreaArray[x].bEnabled = false;
			continue;
		}
		memcpy(&g_OutputAreaArray[x].srcRect, &tmpButtonPos, sizeof(SDL_Rect));

		// Get Dst Pos
		sprintf(szIniTag, "%s:dst", g_OutputAreaArray[x].szIniName);
		if(0 != GetIniRect(szIniTag, &tmpButtonPos))
		{
			LogError("GetOutputAreaPos: ERROR getting on pos [%s] disabling", szIniTag);
			g_OutputAreaArray[x].bEnabled = false;
			continue;
		}
		memcpy(&g_OutputAreaArray[x].dstRect, &tmpButtonPos, sizeof(SDL_Rect));

		// Get Selector Pos
		sprintf(szIniTag, "%s:selector", g_OutputAreaArray[x].szIniName);
		if(0 != GetIniRect(szIniTag, &tmpButtonPos))
		{
			LogError("GetOutputAreaPos: ERROR getting selector pos [%s] disabling", szIniTag);
			g_OutputAreaArray[x].bHasSelector = false;
		}
		else
		{
			memcpy(&g_OutputAreaArray[x].selectorPos, &tmpButtonPos, sizeof(SDL_Rect));
			g_OutputAreaArray[x].bHasSelector = true;
		}
	}

	return 0;
}

//*****************************************************************************
// 
//*****************************************************************************
//
//
//*****************************************************************************
void CGraphicsUITheme::DisplayString(char *szWord, StringPosEnum posEnum)
{
	DisplayStringSurface(szWord, &g_StringPosArray[posEnum]);
}

//*****************************************************************************
// 
//*****************************************************************************
//
//
//*****************************************************************************
void CGraphicsUITheme::ResetOutputArea(OutputAreaEnum posEnum)
{
	if(false == g_OutputAreaArray[posEnum].bEnabled)
	{
		return;
	}

	SDL_BlitSurface(m_pImageSurface, 
					&g_OutputAreaArray[posEnum].srcRect, 
					m_pPSPSurface, 
					&g_OutputAreaArray[posEnum].dstRect);
}

//*****************************************************************************
// 
//*****************************************************************************
//
//
//*****************************************************************************
void CGraphicsUITheme::DisplayString(char *szWord, OutputAreaEnum posEnum, int nLineNumber, bool bHighlight)
{
	SDL_Rect dst;
	SDL_Rect src;

	if(false == g_OutputAreaArray[posEnum].bEnabled)
	{
		return;
	}

	if(((nLineNumber - 1) >= g_OutputAreaArray[posEnum].nLineCount) ||
		((nLineNumber - 1) < 0))
	{
		return;
	}

	SDL_Surface *pSurface = GetStringSurface(szWord, g_OutputAreaArray[posEnum].nFontIndex);

	if(NULL != pSurface)
	{
		memcpy(&dst, &g_OutputAreaArray[posEnum].dstRect, sizeof(SDL_Rect));
		memcpy(&src, &g_OutputAreaArray[posEnum].srcRect, sizeof(SDL_Rect));

		src.y = src.y + (g_OutputAreaArray[posEnum].lineSize.h * (nLineNumber - 1));
		src.h = g_OutputAreaArray[posEnum].lineSize.h;
		
		dst.y = dst.y + (g_OutputAreaArray[posEnum].lineSize.h * (nLineNumber - 1));
		dst.h = g_OutputAreaArray[posEnum].lineSize.h;

		
		// Clear out area before repaint
		SDL_BlitSurface(m_pImageSurface, &src, m_pPSPSurface, &dst);

		if(true == bHighlight)
		{
			if(true == g_OutputAreaArray[posEnum].bHasSelector)
			{
				SDL_BlitSurface(m_pImageSurface, 
								&g_OutputAreaArray[posEnum].selectorPos,
								m_pPSPSurface,
								&dst);
			}
			else
			{
				SDL_FillRect(m_pPSPSurface, 
								&dst, 
								SDL_MapRGBA(m_pPSPSurface->format, 0, 0, 255, 100));
			}
		}

		src.w = pSurface->w;
		src.h = pSurface->h;
		src.x = 0;
		src.y = 0;

		if(src.w > dst.w)
		{
			src.w = dst.w;
		}

		// Vertically align word;
		dst.y += (pSurface->h / 2);

		SDL_BlitSurface(pSurface, &src, m_pPSPSurface, &dst);
		SDL_FreeSurface(pSurface);
	}
}

//*****************************************************************************
// 
//*****************************************************************************
//
//
//*****************************************************************************
void CGraphicsUITheme::DisplayButton(ButtonPosEnum posEnum)
{
	DisplayButton(posEnum, (g_ButtonPosArray[posEnum].nCurrentState+1)%g_ButtonPosArray[posEnum].nButtonCount);
}

//*****************************************************************************
// 
//*****************************************************************************
//
//
//*****************************************************************************
void CGraphicsUITheme::DisplayButton(ButtonPosEnum posEnum, int nState)
{
	SDL_Rect *pRect = NULL;

	if(false == g_ButtonPosArray[posEnum].bEnabled)
	{
		return;
	}

	pRect = &g_ButtonPosArray[posEnum].pSrcRect[nState];
	g_ButtonPosArray[posEnum].nCurrentState = nState;
	
	SDL_BlitSurface(m_pImageSurface, 
						pRect,
						m_pPSPSurface,
						&g_ButtonPosArray[posEnum].dstRect);

}

//*****************************************************************************
// 
//*****************************************************************************
//
//
//*****************************************************************************
void CGraphicsUITheme::DisplayStringSurface(char *szWord, StringPosType *pPos)
{
	SDL_Rect dst;
	SDL_Rect src;

	if(false == pPos->bEnabled)
	{
		return;
	}

	SDL_Surface *pSurface = GetStringSurface(szWord, pPos->nFontIndex);

	if(NULL != pSurface)
	{
		memcpy(&dst, &pPos->rectPos, sizeof(SDL_Rect));

		src.x = 0;
		src.y = 0;
		src.w = pSurface->w;
		src.h = pSurface->h;

		// Crop Lines that are too long
		if(src.w > dst.w)
		{
			src.w = dst.w;
		}

		// Vertically align word;
		dst.y = dst.y + (src.h / 2) - 1;

		// Justify Font
		switch(pPos->fontJust)
		{
			case JUST_CENTER:
			{
				dst.x = pPos->rectPos.x + (pPos->rectPos.w/2 - src.w/2);
			}break;

			case JUST_RIGHT:
			{
				dst.x = pPos->rectPos.x + pPos->rectPos.w - src.w;
			}break;

			case JUST_LEFT:
			default:
			{
				// Dont need to do anything
			}break;

		}
		
		// Clear out area before repaint
		SDL_BlitSurface(m_pImageSurface, &pPos->rectPos, m_pPSPSurface, &pPos->rectPos);

		SDL_BlitSurface(pSurface, &src, m_pPSPSurface, &dst);
		SDL_FreeSurface(pSurface);
	}
}

//*****************************************************************************
// 
//*****************************************************************************
//
//
//*****************************************************************************
SDL_Surface *CGraphicsUITheme::GetStringSurface(char *szWord, int nFontIndex)
{
	int nFontWidth = m_FontSize[nFontIndex].w;
	int nFontHeight = m_FontSize[nFontIndex].h;
	int nCurrentXPos = 0;
	int nCurrentYPos = 0;	
	
	SDL_Surface *pSurface = NULL;
	
	pSurface = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCCOLORKEY,
									nFontWidth * (int)strlen(szWord),
									nFontHeight,
									m_pImageSurface->format->BitsPerPixel,
									m_pImageSurface->format->Rmask,
									m_pImageSurface->format->Gmask,
									m_pImageSurface->format->Bmask,
									m_pImageSurface->format->Amask);
																		
	if(NULL == pSurface)
	{
		LogError("ERROR: GetStringSurface unable to allocate surface [%s]", SDL_GetError());
		return NULL;
	}
	
	SDL_SetColorKey(pSurface, SDL_SRCCOLORKEY, SDL_MapRGB(pSurface->format, m_TransparencyColor.r, m_TransparencyColor.g, m_TransparencyColor.b)); 
	SDL_FillRect(pSurface, NULL, SDL_MapRGB(pSurface->format, m_TransparencyColor.r, m_TransparencyColor.g, m_TransparencyColor.b));
	for(size_t x = 0; x < strlen(szWord); x++)
	{
		if(m_FontMap[nFontIndex].end() != m_FontMap[nFontIndex].find(szWord[x]))
		{
			SDL_Rect src = m_FontMap[nFontIndex][szWord[x]];

			SDL_Rect dst = 	{ 
								nCurrentXPos,
								nCurrentYPos,
							};
			
			SDL_BlitSurface(m_pImageSurface, &src, pSurface, &dst);		
		}
		nCurrentXPos += nFontWidth;		
	}
	
	return pSurface;
}

void CGraphicsUITheme::LogError(char *szFormat, ...)
{
	va_list args;
	va_start (args, szFormat);
	vsprintf(m_szMsg, szFormat, args);
	Log(LOG_ERROR, m_szMsg);
	va_end (args);
}

void CGraphicsUITheme::LogInfo(char *szFormat, ...)
{
	va_list args;
	va_start (args, szFormat);
	vsprintf(m_szMsg, szFormat, args);
	Log(LOG_INFO, m_szMsg);
	va_end (args);
}
