#ifndef _UI_I_
#define _UI_I_

#include "PlayList.h"

/** UI class interface */ 
class IPSPRadio_UI
{
public:
	//virtual ~IPSPRadio_UI(){};//{};
	
	virtual int Initialize(char *strCWD);
	virtual void Terminate() = 0;

	virtual int SetTitle(char *strTitle);
	virtual int DisplayMessage_EnablingNetwork();
	virtual int DisplayMessage_DisablingNetwork();
	virtual int DisplayMessage_NetworkReady(char *strIP);
	virtual int DisplayMessage_NetworkSelection(int iProfileID, char *strProfileName);
	virtual int DisplayMainCommands();
	virtual int DisplayActiveCommand(CPSPSound::pspsound_state playingstate);
	virtual int DisplayErrorMessage(char *strMsg);
	virtual int DisplayBufferPercentage(int a);

	/** these are listed in sequential order */
	virtual int OnNewStreamStarted();
	virtual int OnStreamOpening();
	virtual int OnConnectionProgress();
	virtual int OnStreamOpeningError();
	virtual int OnStreamOpeningSuccess();
	virtual int OnVBlank();
	virtual int DisplaySampleRateAndKBPS(int samplerate, int bitrate);
	virtual int DisplayMPEGLayerType(char *strType);
	virtual int OnNewSongData(CPlayList::songmetadata *pData);

};

#endif
	
