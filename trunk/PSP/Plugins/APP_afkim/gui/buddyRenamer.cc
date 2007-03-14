#include "buddyRenamer.h"
#include <iostream>
#include "menuMain.h"

buddyRenamer::buddyRenamer()
{
	pixels = NULL;
	
	background = new guiBit("./pics/rename.png", false);
	background->moveTo(2,178);
	
	nickname = new accountCreateText(61, 224, "", "done");
	
	dirty = true;
	gui_active = false;
}

buddyRenamer::~buddyRenamer()
{
	delete nickname;
	delete background;
}

string buddyRenamer::getInputKey() const
{
	return "buddyRenamer";
}

string buddyRenamer::takeInput(SDL_Joystick* joystick)
{
	//lock to prevent the previous X leaking in
	if (justActive)
	{
		if (PRESSING_X(joystick))
			return getInputKey();
		else
			justActive = false;
	}
	
	if (PRESSING_SELECT(joystick))
	{
		return "menuBuddyOptions";
	}
	else
	{
		string res = nickname->takeInput(joystick);
		if (nickname->dirty) dirty = true;
		
		if (res == "done") //Returning, rename em
		{
			bitlbeeCallback::getBee()->renameCurrentBuddy(nickname->getText());
			return ((menuMain*)inputs["menuMain"])->changeReturnVal;
		}
		return getInputKey();
	}
}

bool buddyRenamer::needsRedraw() const
{
	nickname->needsRedraw();
		return true;
	
	return dirty;
}

void buddyRenamer::draw()
{
	if (!gui_active)
		return;
	
	background->draw();
	nickname->draw();
}

void buddyRenamer::inputableActivate()
{
	justActive = true;
	inputable::inputableActivate();
	nickname->inputableActivate();
	nickname->clearText();
}

void buddyRenamer::inputableDeactivate()
{
	inputable::inputableDeactivate();
	nickname->inputableDeactivate();
}
