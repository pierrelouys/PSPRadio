#include "menuMain.h"
#include "../dlib/util.h"
#include "../bitlbee.h"
#include "accountCreator.h"

menuMain::menuMain()
{
	init(11, 159, 12, 0, "./pics/selected.png"); 
	bgimage = new guiBit("./pics/mainmenu.png", false);
	bgimage->moveTo(1,146);
}

menuMain::~menuMain()
{
	delete bgimage;
}

void menuMain::draw()
{
	dirty = false;
	if (!gui_active)
		return;
	bgimage->draw();
	selector::draw();
}

string menuMain::getInputKey() const
{
	return "menuMain";
}

//functions called when things happen in takeInput,
//the value returned here is the value returned to takeInput
string menuMain::fixSelected()
{
	//7 possible selections
	if (selected < 0) selected = 6;
	if (selected > 6) selected = 0;
	return getInputKey();
}

string menuMain::pressCross()
{
	switch(selected)
	{
		case 0: //Buddy Options
			return "menuBuddyOptions";
		case 1: //Away
			bitlbeeCallback::getBee()->setAway();
			break;
		case 2: //Back
			bitlbeeCallback::getBee()->setBack();
			break;
		case 3: //Add Account
			return "menuAddAccount";
		case 4: //Remove Account
			return "accountDeleter";
		case 5: //Reconnect Wifi
			return "RESET_AFKIM";
		case 6: //Quit
			return SWITCH_QUIT;
	}
	return changeReturnVal;
}

string menuMain::pressSelect()
{
	return changeReturnVal;
}
 
