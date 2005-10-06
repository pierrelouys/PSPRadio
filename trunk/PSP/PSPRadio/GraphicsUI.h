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
	int DisplaySampleRateAndKBPS(int samplerate, int bitrate);
	int DisplayMPEGLayerType(char *strType);
	//int DisplayMetadata(char *strTitle, char *strURL);
	int OnNewSongData(CPlayList::songmetadata *pData);
	
private:
	SDL_Surface *LoadImage(char *szImageName);
	void UnLoadImage(SDL_Surface **ppImage);

	void SetBaseImage(void);
	void SetPlayButton(uibuttonstate_enum state);
	void SetPauseButton(uibuttonstate_enum state);
	void SetStopButton(uibuttonstate_enum state);
	
	bool InitializeTheme(char *szFilename);
	bool InitializeSDL();
	bool InitializeImages();
	
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
};



#endif
