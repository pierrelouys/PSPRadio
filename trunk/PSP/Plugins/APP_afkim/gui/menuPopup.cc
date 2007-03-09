#include "menuPopup.h"
#include "../dlib/util.h"
#include "../bitlbee.h"
#include "accountCreator.h"

menuPopup::menuPopup()
{
	init(20, 172, 12, 0, "./pics/selected.png"); 
	bgimage = new guiBit("./pics/menu.png");
	bgimage->moveTo(1,146);
}

menuPopup::~menuPopup()
{
	delete bgimage;
}

void menuPopup::draw()
{
	dirty = false;
	if (!gui_active)
		return;
	bgimage->draw();
	selector::draw();
}

string menuPopup::getInputKey() const
{
	return "menuPopup";
}

//functions called when things happen in takeInput,
//the value returned here is the value returned to takeInput
string menuPopup::fixSelected()
{
	if (selected < 0) selected = 5;
	if (selected > 5) selected = 0;
	return getInputKey();
}

string menuPopup::pressCross()
{
	if (selected == 5)
		return SWITCH_QUIT;

	accountCreator* newAC = NULL;
#warning menuPopup "menuPopup" in accountCreator = correct behaviour?
#warning menuPopup accountCreators get leaked when closed, likely that the bitlbee one gets leaked too!
	switch(selected)
	{
	case 0:
		newAC = new accountCreator(BAT_AOL, unicodeClean("Enter details for your AIM Account"), "menuPopup");
		break;
	case 1:
		newAC = new accountCreator(BAT_ICQ, unicodeClean("Enter details for your ICQ Account\nYour username is your ICQ number"), "menuPopup");
		break;
	case 2:
		newAC = new accountCreator(BAT_MSN, unicodeClean("Enter details for your MSN Account\nFull email address, like: someone@msn.com"), "menuPopup");
		break;
	case 3:
		newAC = new accountCreator(BAT_GMAIL, unicodeClean("Enter details for your GTalk Account\nYou do not need to add '@gmail.com'.\nIt is done automatically."), "menuPopup");
		break;
	case 4:
		newAC = new accountCreator(BAT_YAHOO, unicodeClean("Enter details for your Yahoo Account"), "menuPopup");
		break;
	}
	inputs["accountCreator"] = newAC;
	return "accountCreator";
}

string menuPopup::pressSelect()
{
	holdingOne = true;
	return changeReturnVal;
//	return 1; //HACK ??
}
