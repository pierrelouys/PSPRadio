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
	if (selected < 0) selected = 4;
	if (selected > 4) selected = 0;
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
			if (bitlbeeCallback::getBee()->isTalkingToSomeone()) //Can only rename if ur talking to someone
				return "buddyRenamer";
		case 2: //Block Buddy
			bitlbeeCallback::getBee()->blockCurrentBuddy();
			return ((menuMain*)inputs["menuMain"])->changeReturnVal; //HACK
		case 3: //Unblock Buddy
			bitlbeeCallback::getBee()->allowCurrentBuddy();
			return ((menuMain*)inputs["menuMain"])->changeReturnVal; //HACK
		case 4: //Delete Buddy
			bitlbeeCallback::getBee()->removeCurrentBuddy();
			return ((menuMain*)inputs["menuMain"])->changeReturnVal; //HACK
		case 5: //BUDDY?
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
