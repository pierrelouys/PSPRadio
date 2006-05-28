#include "accountCreateText.h"
#include "../dlib/keyboards/p_sprint.h"
#include <iostream>

#warning THIS WONT WORK RIGHT THIS IS BROKE

accountCreateText::accountCreateText(const unsigned int &nX, const unsigned int &nY)
{
	init(40, nX, nY, TEXT_NORMAL_COLOR);
}

wstring accountCreateText::getText()
{
	return realText;
}

int accountCreateText::handleEnter()
{
	if (realText.size() == 0)
		return inputableIndexVal;
	
	dirty = true;
	return inputableIndexVal+1;			//HACK?
}

void accountCreateText::inputableActivate()
{
	keyboard->lock();
	gui_active = true;
}

void accountCreateText::inputableDeactivate()
{
	gui_active = false;
}
