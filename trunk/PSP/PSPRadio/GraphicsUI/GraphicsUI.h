#ifndef _PSPRADIOGRAPHICSUI_
#define _PSPRADIOGRAPHICSUI_

#include "IPSPRadio_UI.h"
#include "GraphicsUITheme.h"

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

enum uibuttonstate_enum
{
	UIBUTTONSTATE_ON = 0,
	UIBUTTONSTATE_OFF = 1
};

class CGraphicsUI : public IPSPRadio_UI
{
public:
	CGraphicsUI();
	~CGraphicsUI();
	
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
	int DisplayPLEntries(CPlayList *PlayList);

private:
	SDL_Surface *LoadImage(char *szImageName);
	void UnLoadImage(SDL_Surface **ppImage);

	void SetBaseImage(void);
	void SetPlayButton(uibuttonstate_enum state);
	void SetPauseButton(uibuttonstate_enum state);
	void SetStopButton(uibuttonstate_enum state);	
	void SetSoundButton(uibuttonstate_enum state);
	void SetButton(CGraphicsUIThemeItem themeItem, uibuttonstate_enum state);	
	void SetButton(CGraphicsUIPosItem posSrc, CGraphicsUIPosItem posDst);
	
	bool InitializeTheme(char *szFilename, char *szThemePath);
	bool InitializeSDL();
	bool InitializeImages();
	
	SDL_Surface *DisplayWord(char *szWord);	
	void DisplayWord(CGraphicsUIPosItem *pPosItem, char *szWord, bool bCenter=true);
	void ResetImageArea(CGraphicsUIPosItem *pSrcPosItem, CGraphicsUIPosItem *pDstPosItem, SDL_Surface *pSrcSurface, SDL_Surface *pDstSurface);
	void ResetImageArea(CGraphicsUIPosItem *pPosItem, SDL_Surface *pSrcSurface, SDL_Surface *pDstSurface);
	void CopySurface(SDL_Surface *pSrcSurface, SDL_Surface *pDstSurface, CGraphicsUIPosItem *pDstPosItem, bool bCenter);
	
private:
	SDL_Surface *m_pImageBase;	
	SDL_Surface *m_pScreen;	
	
	SDL_Surface *m_pPlayListText[100]; // TODO: Make this dynamic
	int			m_nPlayListTextCount;
	
	SDL_Surface *m_pPlayListItemText[100]; // TODO: Make this dynamic
	int			m_nPlayListItemTextCount;
		
	int m_nDepth;
 	int m_nFlags;	
	
	CGraphicsUITheme m_theme;	
	char m_szThemeImagePath[100];
	
	/** Image and Button Items */
	CGraphicsUIThemeItem m_themeItemBackground;
	CGraphicsUIThemeItem m_themeItemPlay;
	CGraphicsUIThemeItem m_themeItemPause;
	CGraphicsUIThemeItem m_themeItemStop;
	CGraphicsUIThemeItem m_themeItemLoad;
	CGraphicsUIThemeItem m_themeItemSound;
	CGraphicsUIThemeItem m_themeItemVolume;
	CGraphicsUIThemeItem m_themeItemABC123;
	
	/** Output Locations */
	CGraphicsUIPosItem m_posItemPlayListArea;
	CGraphicsUIPosItem m_posItemPlayListAreaSel;
	CGraphicsUIPosItem m_posItemPlayListItemArea;
	CGraphicsUIPosItem m_posItemPlayListItemAreaSel;
		
	/** String Locations */
	CGraphicsUIPosItem m_posItemFileNameString;
	CGraphicsUIPosItem m_posItemFileTitleString;
	CGraphicsUIPosItem m_posItemURLString;
	CGraphicsUIPosItem m_posItemSongTitleString;
	CGraphicsUIPosItem m_posItemSongArtistString;
	CGraphicsUIPosItem m_posItemLengthString;
	CGraphicsUIPosItem m_posItemSampleRateString;
	CGraphicsUIPosItem m_posItemBitRateString;
	CGraphicsUIPosItem m_posItemMPEGLayerString;
	CGraphicsUIPosItem m_posItemErrorString;
	CGraphicsUIPosItem m_posItemStreamString;
	CGraphicsUIPosItem m_posItemNetworkString;
	CGraphicsUIPosItem m_posItemBufferString;
};

#endif
