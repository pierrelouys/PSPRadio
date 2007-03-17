#ifndef INCLUDED_GUI_BUDDYRENAMER
#define INCLUDED_GUI_BUDDYRENAMER

#include "../dlib/inputable.h"
#include "../bitlbee.h"
#include "accountCreateText.h"

//gui object for creating accounts, bitlbee or aim etc
class buddyRenamer : public inputable
{
public:
	buddyRenamer();
	~buddyRenamer();
	string takeInput(SDL_Joystick* joystick);
	
	//From guibit:
	void draw();
	bool needsRedraw() const;
	
	//From inputable:
	void inputableActivate();
	void inputableDeactivate();
	string getInputKey() const;
	
//private
	accountCreateText* nickname; //FIXME: Should probably rename accountCreateText class to something better since we use it here
protected:
	guiBit* background;
};

#endif //INCLUDED_GUI_ACCOUNTCREATE
