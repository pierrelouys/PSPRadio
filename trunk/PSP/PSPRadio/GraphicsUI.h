#ifndef _PSPRADIOGRAPHICSUI_
#define _PSPRADIOGRAPHICSUI_

#include "IPSPRadio_UI.h"

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

enum uibuttonstate_enum
{
	UIBUTTONSTATE_OFF = 0,
	UIBUTTONSTATE_ON = 1
};

class CGraphicsUI : public IPSPRadio_UI
{
public:
	CGraphicsUI();
	~CGraphicsUI();
	
public:
	int Initialize();
	void Terminate();

	int SetTitle(char *strTitle);
	int DisplayMessage_EnablingNetwork();
	int DisplayMessage_DisablingNetwork();
	int DisplayMessage_NetworkReady(char *strIP);
	int DisplayMessage_NetworkSelection(int iProfileID, char *strProfileName);
	int DisplayMainCommands();
	int DisplayActiveCommand(CPSPSound::pspsound_state playingstate);
	int DisplayErrorMessage(char *strMsg);
	int DisplayPlayBuffer(int a, int b);
	int DisplayDecodeBuffer(int a, int b);

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

	void SetPlayButton(uibuttonstate_enum state);
	void SetPauseButton(uibuttonstate_enum state);
	void SetStopButton(uibuttonstate_enum state);

	
private:
	bool m_bSDLInitialized;
	SDL_Surface *m_pImageBase;	
	SDL_Surface *m_pImageLoad[2];
	SDL_Surface *m_pImagePlay[2];
	SDL_Surface *m_pImagePause[2];
	SDL_Surface *m_pImageStop[2];	
	SDL_Surface *m_pImageSound[2];
	
	SDL_Surface *m_pScreen;
	int m_nDepth;
 	int m_nFlags;	
};



#endif
