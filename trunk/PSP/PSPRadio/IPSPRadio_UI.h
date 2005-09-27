#ifndef _UI_I_
#define _UI_I_

/** UI class interface */ 
class IPSPRadio_UI
{
public:
	//virtual ~IPSPRadio_UI(){};
	

	virtual int Initialize() = 0;
	virtual int SetTitle(char *strTitle);
	virtual int DisplayMessage_EnablingNetwork();
	virtual int DisplayMessage_DisablingNetwork();
	virtual int DisplayMessage_NetworkReady(char *strIP);
	virtual int DisplayMainCommands();
	virtual int DisplayActiveCommand(CPSPSound::pspsound_state playingstate);
	virtual int DisplayErrorMessage(char *strMsg);
	virtual int DisplayPlayBuffer(int a, int b);
};

#endif
	
