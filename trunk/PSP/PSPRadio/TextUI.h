#ifndef _PSPRADIOTEXTUI_
#define _PSPRADIOTEXTUI_

#include "IPSPRadio_UI.h"

class CTextUI : public IPSPRadio_UI
{
public:
	virtual ~CTextUI();
	
public:
	virtual int Initialize();
	virtual int SetTitle(char *strTitle);
	virtual int DisplayMessage_EnablingNetwork();
	virtual int DisplayMessage_DisablingNetwork();
	virtual int DisplayMessage_NetworkReady(char *strIP);
	virtual int DisplayMainCommands();
	virtual int DisplayActiveCommand(CPSPSound::pspsound_state playingstate);
	virtual int DisplayErrorMessage(char *strMsg);
	virtual int DisplayPlayBuffer(int a, int b);
	
private:
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
	int uiprint(char *strText, int x = 0, int y = 0, uicolors color = COLOR_WHITE);
	int uiprint(char *strFmt, char *strArg, int x = 0, int y = 0, uicolors color = COLOR_WHITE);
};



#endif
