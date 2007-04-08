#include "menuAddAccount.h"
#include "../dlib/util.h"
#include "../bitlbee.h"
#include "menuMain.h"
#include "accountCreator.h"

menuAddAccount::menuAddAccount()
{
	init(11, 171, 12, 0, "./pics/selected.png"); 
	bgimage = new guiBit("./pics/menu.png", false);
	bgimage->moveTo(1,146);
}

menuAddAccount::~menuAddAccount()
{
	delete bgimage;
}

void menuAddAccount::draw()
{
	dirty = false;
	if (!gui_active)
		return;
	bgimage->draw();
	selector::draw();
}

string menuAddAccount::getInputKey() const
{
	return "menuAddAccount";
}

//functions called when things happen in takeInput,
//the value returned here is the value returned to takeInput
string menuAddAccount::fixSelected()
{
	if (selected < 0) selected = 5;
	if (selected > 5) selected = 0;
	return getInputKey();
}

string menuAddAccount::pressCross()
{
	accountCreator* newAC = NULL;
#warning menuAddAccount "menuAddAccount" in accountCreator = correct behaviour?
	switch(selected)
	{
	case 0:
		newAC = new accountCreator(BAT_AOL, unicodeClean("Enter details for your AIM Account"), "menuAddAccount");
		break;
	case 1:
		newAC = new accountCreator(BAT_ICQ, unicodeClean("Enter details for your ICQ Account\nYour username is your ICQ number"), "menuAddAccount");
		break;
	case 2:
		newAC = new accountCreator(BAT_MSN, unicodeClean("Enter details for your MSN Account\nFull email address, like: someone@msn.com"), "menuAddAccount");
		break;
	case 3:
		newAC = new accountCreator(BAT_GMAIL, unicodeClean("Enter details for your GTalk Account\nYou do not need to add '@gmail.com'.\nIt is done automatically."), "menuAddAccount");
		break;
	case 4:
		newAC = new accountCreator(BAT_YAHOO, unicodeClean("Enter details for your Yahoo Account"), "menuAddAccount");
		break;
	case 5:
		newAC = new accountCreator(BAT_JABBER, unicodeClean("Enter details for your Jabber Account\nUsername in the form handle@server.tld"), "menuAddAccount");
		break;
	}
	inputs["accountCreator"] = newAC;
	return "accountCreator";
}

string menuAddAccount::pressCircle()
{
	return "menuMain";
}

string menuAddAccount::pressSelect()
{
	return ((menuMain*)inputs["menuMain"])->changeReturnVal; //HACK
}
