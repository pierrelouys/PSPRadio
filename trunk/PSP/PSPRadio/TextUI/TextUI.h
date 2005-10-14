#ifndef _PSPRADIOTEXTUI_
#define _PSPRADIOTEXTUI_

#include "IPSPRadio_UI.h"

class CTextUI : public IPSPRadio_UI
{
public:
	CTextUI();
	~CTextUI();
	
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
	
	/** Screen Handling */
	void Initialize_Screen(CScreenHandler::Screen screen);
	void UpdateOptionsScreen(list<CScreenHandler::Options> &OptionsList, 
							list<CScreenHandler::Options>::iterator &CurrentOptionIterator);

	
private:
	CLock *m_lockprint;
	CLock *m_lockclear;
	CIniParser *m_Config;
	char  *m_strTitle;
	//helpers
	enum uicolors
	{
		COLOR_BLACK = 0x00000000,
		COLOR_WHITE = 0x00FFFFFF,
		COLOR_RED   = 0x000000FF,
		COLOR_GREEN = 0x0000FF00,
		COLOR_BLUE  = 0x00FF0000,
		COLOR_CYAN  = 0x00AABB00,
		COLOR_YELLOW= 0x00559999
	};
	void uiPrintf(int x, int y, int color, char *strFormat, ...);
	void ClearRows(int iRowStart, int iRowEnd = -1);

	int ClearErrorMessage();
	int GetConfigColor(char *strKey);
	void GetConfigPos(char *strKey, int *x, int *y);
	
	void PrintOption(int x, int y, int c, char *strName, char *strStates[], int iNumberOfStates, int iSelectedState);

};



#endif
