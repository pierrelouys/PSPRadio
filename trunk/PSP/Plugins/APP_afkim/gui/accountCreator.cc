#include "accountCreator.h"
#include <iostream>

guiBit* accountCreator::background = NULL;
accountCreator::accountCreator(bitlbeeAccountType nType, wstring message, int nReturnTo)
{
	pixels = NULL;
	
	if (background == NULL)
	{
		//we should free this some how...
		background = new guiBit("./pics/accc.png");
		background->moveTo(2,148);
	}
	currentSelected = 0;
	returnTo = nReturnTo;
	type = nType;
	infoText = textBlock(43,3);
	infoText.addText(message, TEXT_NORMAL_COLOR);
	
	text = new textArea(43, 3, &infoText);
	text->moveTo(11,168);
	username = new accountCreateText(61, 203);
	username->setIndexVal(0);
	password = new accountCreateText(61, 223);
	password->setIndexVal(1);
	
	dirty = true;
	gui_active = false;
}

int accountCreator::takeInput(SDL_Joystick* joystick)
{
	//lock to prevent the previous X leaking in
	if (justActive)
	{
		if (PRESSING_X(joystick))
			return inputableIndexVal;
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
	int result;
	if (currentSelected == 0)
	{
// 		cout << "AAAA" << endl;
		result = username->takeInput(joystick);
		if (username->dirty) dirty = true;
	}
	else
	{
//		cout << "BBBB" << endl;
		result = password->takeInput(joystick);
		if (password->dirty) dirty = true;
	}
	if (currentSelected != result)
	{
		switch(result)
		{
			case 0:
				username->inputableActivate();
				password->inputableDeactivate();
				break;
			case 1:
				password->inputableActivate();
				username->inputableDeactivate();
				break;
			case 2:
				if (type != BAT_BITLBEE)
				{
					bitlbeeCallback::getBee()->addAccount(type, username->getText(), password->getText());
				}
				return returnTo;
				break;
			default: //this is when we are quitting the menu
				return returnTo;
				break;
		}
		currentSelected = result;
	}
	return inputableIndexVal;
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
	
	cout << "redrawing AC" << endl;
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
