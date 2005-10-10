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

class CGraphicsUI : public virtual IPSPRadio_UI
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
	int OnNewSongData(CPlayList::songmetadata *pData);	
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
	
	bool InitializeTheme(char *szFilename, char *szThemePath);
	bool InitializeSDL();
	bool InitializeImages();
	
	void DisplayWordInfoArea(char *szWord, int nLineNumber, bool bCenter=true);
	void ClearLineInfoArea(int nLineNumber);

	void DisplayWordPlaylistArea(char *szWord, int nLineNumber, bool bCenter=true);
	void ClearLinePlaylistArea(int nLineNumber);
	
	void DisplayWordPlaylistItemArea(char *szWord, int nLineNumber, bool bCenter=true);
	void ClearLinePlaylistItemArea(int nLineNumber);
	
	void DisplayWord(CGraphicsUIThemeItem themeItemArea, char *szWord,  int nLineNumber, bool bCenter=true);
	void ClearLine(CGraphicsUIThemeItem themeItemArea, int nLineNumber);
	
	
private:
	const SDL_VideoInfo *m_pVideoInfo;
	SDL_Surface *m_pImageBase;	
	SDL_Surface *m_pScreen;
	int m_nDepth;
 	int m_nFlags;	
	
	CGraphicsUITheme m_theme;	
	char m_szThemeImagePath[100];
	CGraphicsUIThemeItem m_themeItemBackground;
	CGraphicsUIThemeItem m_themeItemPlay;
	CGraphicsUIThemeItem m_themeItemPause;
	CGraphicsUIThemeItem m_themeItemStop;
	CGraphicsUIThemeItem m_themeItemLoad;
	CGraphicsUIThemeItem m_themeItemSound;
	CGraphicsUIThemeItem m_themeItemVolume;
	CGraphicsUIThemeItem m_themeItemABC123;
	CGraphicsUIThemeItem m_themeItemInfoArea;
	CGraphicsUIThemeItem m_themeItemPlaylistArea;
	CGraphicsUIThemeItem m_themeItemPlaylistItemArea;
};



#endif
