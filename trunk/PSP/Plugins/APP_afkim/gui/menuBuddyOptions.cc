#include "menuBuddyOptions.h"
#include "menuMain.h"
#include "../dlib/util.h"
#include "../bitlbee.h"

menuBuddyOptions::menuBuddyOptions()
{
	init(11, 159, 12, 0, "./pics/selected.png"); 
	bgimage = new guiBit("./pics/buddymenu.png", false);
	bgimage->moveTo(1,146);
}

menuBuddyOptions::~menuBuddyOptions()
{
	delete bgimage;
}

void menuBuddyOptions::draw()
{
	dirty = false;
	if (!gui_active)
		return;
	bgimage->draw();
	selector::draw();
}

string menuBuddyOptions::getInputKey() const
{
	return "menuBuddyOptions";
}

//functions called when things happen in takeInput,
//the value returned here is the value returned to takeInput
string menuBuddyOptions::fixSelected()
{
	if (selected < 0) selected = 6;
	if (selected > 6) selected = 0;
	return getInputKey();
}

string menuBuddyOptions::pressCross()
{
	switch (selected)
	{
		case 0: //Buddy Details
			bitlbeeCallback::getBee()->getCurrentBuddyDetails();
			return ((menuMain*)inputs["menuMain"])->changeReturnVal; //HACK
		case 1: //Rename Buddy
			return "buddyRenamer";
		case 2: //Block Buddy
			break;
		case 3: //Unblock Buddy
			break;
		case 4: //Add Buddy
			break;
		case 5: //Delete Buddy
			break;
		case 6: //BUDDY?
			break;
	}
	return getInputKey();
}

string menuBuddyOptions::pressSelect()
{
	return "menuMain";
}
