#include "accountDeleter.h"
#include "menuMain.h"
#include "../dlib/util.h"
#include "../bitlbee.h"

accountDeleter::accountDeleter()
{
	init(11, 164, 10, 0, "./pics/selected.png"); 
	bgimage = new guiBit("./pics/delaccount.png", false);
	bgimage->moveTo(1,146);
	
	listText = textBlock(29,8); //8 should be more than we need
	text = new textArea(29, 8, &listText);
	text->moveTo(22,165);
	text->setAlign(ALIGN_TOP);
	
	dirty = true;
	
	bitlbee = bitlbeeCallback::getBee(); //record for performance, fairly pointless
}

accountDeleter::~accountDeleter()
{
	bitlbee = NULL;
	delete text;
	delete bgimage;
}

void accountDeleter::draw()
{
	dirty = false;
	if (!gui_active)
		return;
	bgimage->draw();
	text->draw();
	selector::draw();
}

void accountDeleter::inputableActivate()
{
	selector::inputableActivate();
	listText.reset();
	for (unsigned int a = 0; a < bitlbee->accounts.size(); a++)
	{
		listText.addText(unicodeClean(bitlbee->accounts[a].type + " - " + bitlbee->accounts[a].details + "\n"), 0x00000000); //black color, maybe change color depending on type?
	}
}

string accountDeleter::getInputKey() const
{
	return "accountDeleter";
}

//functions called when things happen in takeInput,
//the value returned here is the value returned to takeInput
string accountDeleter::fixSelected()
{
	if (selected < 0) selected = bitlbee->accounts.size()-1;
	if (selected >= bitlbee->accounts.size()) selected = 0;
	if (selected < 0) selected = 0;
	return getInputKey();
}

string accountDeleter::pressCross()
{
	if (bitlbee->accounts.size() > 0)
	{
		bitlbee->deleteAccount(bitlbee->accounts[selected].details);
	}
	
	return ((menuMain*)inputs["menuMain"])->changeReturnVal; //HACK
}

string accountDeleter::pressSelect()
{
	return "menuMain";
}
