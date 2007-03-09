#ifndef INCLUDED_GUI_ACCOUNTCREATE
#define INCLUDED_GUI_ACCOUNTCREATE

#include "../dlib/inputable.h"
#include "../bitlbee.h"
#include "accountCreateText.h"

//gui object for creating accounts, bitlbee or aim etc
class accountCreator : public inputable
{
public:
	accountCreator(bitlbeeAccountType nType, wstring message, string nReturnTo);
	~accountCreator();
	string takeInput(SDL_Joystick* joystick);
	
	//From guibit:
	void draw();
	bool needsRedraw() const;
	
	//From inputable:
	void inputableActivate();
	void inputableDeactivate();
	string getInputKey() const;
	
//private
	accountCreateText* username;
	accountCreateText* password;
protected:
	bool dirty;
	bitlbeeAccountType type;
	
	string currentSelected;
	string returnTo;
	
	static guiBit* background;
	textBlock infoText;

	textArea* text;
	
	bool justActive; //true if we were just activated, stops buttons leaking to the keyboard
};

#endif //INCLUDED_GUI_ACCOUNTCREATE
