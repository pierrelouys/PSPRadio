#include "accountCreator.h"
#include <iostream>

guiBit* accountCreator::background = NULL;
accountCreator::accountCreator(bitlbeeAccountType nType, wstring message, string nReturnTo)
{
	pixels = NULL;
	
	if (background == NULL)
	{
		//#warning accountCreator create background and never de-creates it... not really a worry though
		//we should free this some how...
		//reference counting?
		background = new guiBit("./pics/accc.png", false);
		background->moveTo(2,148);
	}
	currentSelected = "username";
	returnTo = nReturnTo;
	type = nType;
	infoText = textBlock(43,3);
	infoText.addText(message, TEXT_NORMAL_COLOR);
	
	text = new textArea(43, 3, &infoText);
	text->moveTo(11,168);
	username = new accountCreateText(61, 203, "username", "password");
	password = new accountCreateText(61, 223, "password", "done");
	
	dirty = true;
	gui_active = false;
}

accountCreator::~accountCreator()
{
	delete text;
	delete username;
	delete password;
}

string accountCreator::getInputKey() const
{
	return "accountCreator";
}

string accountCreator::takeInput(SDL_Joystick* joystick)
{
	//lock to prevent the previous X leaking in
	if (justActive)
	{
		if (PRESSING_X(joystick))
			return getInputKey();
		else
			justActive = false;
	}
	
	//Cancel adding an account, Can't cancel bitlbee.
	if (type!=BAT_BITLBEE && !holdingOne && (PRESSING_SELECT(joystick)))
	{
		holdingOne = true;
		return returnTo;
	}	
	dirty = false;
	string result;
	if (currentSelected == "username")
	{
		result = username->takeInput(joystick);
		if (username->dirty) dirty = true;
	}
	else //password
	{
		result = password->takeInput(joystick);
		if (password->dirty) dirty = true;
	}
	if (currentSelected != result)
	{
		if (result == "username")
		{
			username->inputableActivate();
			password->inputableDeactivate();
		}
		else if (result == "password")
		{
			password->inputableActivate();
			username->inputableDeactivate();
		}
		else if (result == "done")
		{
			if (type != BAT_BITLBEE)
			{
				bitlbeeCallback::getBee()->addAccount(type, username->getText(), password->getText());
			}
			return returnTo;
		}
		else if (result == "menuPopup" && type != BAT_BITLBEE) //quit back to menu unless its a bitlbee account
		{
			return returnTo;
		}
		
		currentSelected = result;
	}
	return getInputKey();
}

bool accountCreator::needsRedraw() const
{
	if (username->needsRedraw())
		return true;
	if (password->needsRedraw())
		return true;
	
	return dirty;
}

void accountCreator::draw()
{
	if (!gui_active)
		return;
	
	background->draw();
	text->draw();
	username->draw();
	password->draw();
}

void accountCreator::inputableActivate()
{
	justActive = true;
	inputable::inputableActivate();
	username->inputableActivate();
}

void accountCreator::inputableDeactivate()
{
	inputable::inputableDeactivate();
	username->inputableDeactivate();
	password->inputableDeactivate();
}
