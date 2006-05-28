#include "chatInput.h"
#include "../bitlbee.h"

chatInput::chatInput(const unsigned int &nLength, const unsigned int &nX, const unsigned int &nY, const unsigned int &nColor)
{
	init(nLength, nX, nY, nColor);
}

int chatInput::handleEnter()
{
	bitlbeeCallback::getBee()->messageCurrent(realText);
	realText = wstring();
	dirty = true;
	cursorpos = 0;
	return inputableIndexVal;
}

void chatInput::inputableActivate()
{
	keyboard->lock();
	gui_active = true;
}

void chatInput::inputableDeactivate()
{
	gui_active = false;
}
