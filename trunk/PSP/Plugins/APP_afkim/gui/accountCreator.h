#ifndef INCLUDED_GUI_ACCOUNTCREATE
#define INCLUDED_GUI_ACCOUNTCREATE

#include "../dlib/inputable.h"
#include "../bitlbee.h"
#include "accountCreateText.h"

//gui object for creating accounts, bitlbee or aim etc
class accountCreator : public inputable
{
public:
	accountCreator(bitlbeeAccountType nType, wstring message, int nReturnTo);
	virtual int takeInput(SDL_Joystick* joystick);
	
	virtual void draw();

	virtual bool needsRedraw() const;
	
	virtual void inputableActivate();
	virtual void inputableDeactivate();

//private
	accountCreateText* username;
	accountCreateText* password;
protected:
	bool dirty;
	bitlbeeAccountType type;
	
	int currentSelected;
	int returnTo;
	
	static guiBit* background;
	textBlock infoText;

	textArea* text;
	
	bool justActive; //true if we were just activated, stops buttons leaking to the keyboard
};

#endif //INCLUDED_GUI_ACCOUNTCREATE
