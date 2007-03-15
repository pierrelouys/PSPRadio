#include "inputable.h"
#include "render.h"

bool inputable::holdingOne = false;
bool inputable::areRepeating = false;
unsigned int inputable::holdingOneTime = 0;

//set this object to be the currently selected/not currently selected inputable object
//the object should remove/add itself to the guilist
void inputable::inputableActivate()
{
	if (!gui_active)
	{
		gui_active = true;
		addGuiBit(this);
	}
}

void inputable::inputableDeactivate()
{
	if (gui_active)
	{
		gui_active = false;
		removeGuiBit(this);
	}
}

bool inputable::isHoldingOne()
{
	if (holdingOne) //currently holding one, check repeats
	{
		unsigned int timeDifference = SDL_GetTicks() - holdingOneTime;
		if (timeDifference >= 500 || (areRepeating && timeDifference >= 200))
		{
			areRepeating = true;
			holdingOneTime = SDL_GetTicks();
			return false;
		}
		else
			return true;
	}
	else
		return false;
}

void inputable::updateHoldingOne(SDL_Joystick* joystick)
{
	bool isCurrentlyHoldingOne = 
	PRESSING_UP(joystick) ||
	PRESSING_DOWN(joystick) ||
	PRESSING_LEFT(joystick) ||
	PRESSING_RIGHT(joystick) ||
	PRESSING_X(joystick) ||
	PRESSING_O(joystick) ||
	PRESSING_SQUARE(joystick) ||
	PRESSING_TRIANGLE(joystick) ||
	PRESSING_SELECT(joystick) ||
	PRESSING_START(joystick);
//	PRESSING_LTRIGGER(joystick) ||
//	PRESSING_RTRIGGER(joystick);
	
	if (isCurrentlyHoldingOne)
	{
		if (!holdingOne) //Just pressed it
		{
			holdingOneTime = SDL_GetTicks();
		}
		holdingOne = true;
	}
	else
	{
		holdingOne = false;
		areRepeating = false;
	}
}