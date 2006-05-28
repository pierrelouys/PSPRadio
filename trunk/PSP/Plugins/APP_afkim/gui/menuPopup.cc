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

//functions called when things happen in takeInput,
//the value returned here is the value returned to takeInput
int menuPopup::fixSelected()
{
	if (selected < 0) selected = 5;
	if (selected > 5) selected = 0;
	return inputableIndexVal;
}

int menuPopup::pressCross()
{
//	#warning need to launch
	int newIndex = inputs.size();
	switch(selected)
	{
	case 0:
		inputs.push_back(new accountCreator(BAT_AOL, unicodeClean("Enter details for your AIM Account"), 2));
		break;
	case 1:
		inputs.push_back(new accountCreator(BAT_ICQ, unicodeClean("Enter details for your ICQ Account\nYour username is your ICQ number"), 2));
		break;
	case 2:
		inputs.push_back(new accountCreator(BAT_MSN, unicodeClean("Enter details for your MSN Account\nFull email address, like: someone@msn.com"), 2));
		break;
	case 3:
		inputs.push_back(new accountCreator(BAT_GMAIL, unicodeClean("Enter details for your GTalk Account\nYou do not need to add '@gmail.com'.\nIt is done automatically."), 2));
		break;
	case 4:
		inputs.push_back(new accountCreator(BAT_YAHOO, unicodeClean("Enter details for your Yahoo Account"), 2));
		break;
	case 5:
		return SWITCH_QUIT;
	}
	inputs.back()->setIndexVal(newIndex);
	return inputableIndexVal+1;
}

int menuPopup::pressSelect()
{
	holdingOne = true;
	return changeReturnVal;
//	return 1; //HACK ??
}
