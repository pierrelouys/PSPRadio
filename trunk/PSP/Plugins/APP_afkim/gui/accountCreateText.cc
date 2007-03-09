#include "accountCreateText.h"
#include "../dlib/keyboards/p_sprint.h"
#include <iostream>

accountCreateText::accountCreateText(const unsigned int &nX, const unsigned int &nY, const string &inputKey, const string &nextInput)
{
	init(40, nX, nY, TEXT_NORMAL_COLOR);
	myInputKey = inputKey;
	nextInputKey = nextInput;
}

string accountCreateText::getInputKey() const
{
	return myInputKey;
}

wstring accountCreateText::getText()
{
	return realText;
}

string accountCreateText::handleEnter()
{
	if (realText.size() == 0)
		return getInputKey();
	
	dirty = true;
	return nextInputKey;
}

string accountCreateText::handleStart()
{
	return handleEnter();
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
