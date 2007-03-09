#include "chatInput.h"
#include "../bitlbee.h"

chatInput::chatInput(const unsigned int &nLength, const unsigned int &nX, const unsigned int &nY, const unsigned int &nColor)
{
	init(nLength, nX, nY, nColor);
}

string chatInput::getInputKey() const
{
	return "chatInput";
}

string chatInput::handleEnter()
{
	bitlbeeCallback::getBee()->messageCurrent(realText);
	realText = wstring(); //FIXME: resize(0); ?
	dirty = true;
	cursorpos = 0;
	return getInputKey();
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
