#include "inputable.h"
#include "render.h"

bool inputable::holdingOne = false;

//set the objects index value
void inputable::setIndexVal(const int inputVal)
{
	inputableIndexVal = inputVal;
}

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

