#ifndef _PSPRADIOTEXTUI_
#define _PSPRADIOTEXTUI_

#include "IPSPRadio_UI.h"

class CTextUI : public IPSPRadio_UI
{
public:
	CTextUI();
	~CTextUI();
	
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
	int OnStreamOpening(char *StreamName);
	int OnConnectionProgress();
	int OnStreamOpeningError(char *StreamName);
	int OnStreamOpeningSuccess(char *StreamName);
	int DisplaySampleRateAndKBPS(int samplerate, int bitrate);
	int DisplayMPEGLayerType(char *strType);
	int DisplayMetadata(char *strTitle, char *strURL);

	
private:
	CLock *m_lockprint;
	CLock *m_lockclear;
	//helpers
	enum uicolors
	{
		COLOR_BLACK = 0x00000000,
		COLOR_WHITE = 0x00FFFFFF,
		COLOR_RED   = 0x000000FF,
		COLOR_GREEN = 0x0000FF00,
		COLOR_BLUE  = 0x00FF0000,
		COLOR_YELLOW= 0x00AABB00
	};
	void uiPrintf(int x, int y, uicolors color, char *strFormat, ...);
	void ClearRows(int iRowStart, int iRowEnd = -1);

	int ClearErrorMessage();

};



#endif
