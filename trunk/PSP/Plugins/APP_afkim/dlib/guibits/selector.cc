#include "selector.h"
#include "../render.h"
#include <SDL/SDL_image.h>

void selector::init(const unsigned int &targetX, const unsigned int &targetY, const unsigned int &nItemSize, const int &nSelected, const string &image)
{
	selected = nSelected;
	
	pixels = IMG_Load(image.c_str());
	
	targetOffset = screen_rect;
	targetOffset.x = targetX - 7;
	targetOffset.y = targetY - 4;
	
	itemSize = nItemSize;
	
	rect.x = 0;
	rect.y = 0;
	rect.w = pixels->w;
	rect.h = pixels->h;
	
//	SDL_Rect offset;
	dirty = true;
	gui_active = false;
}

int selector::takeInput(SDL_Joystick* joystick)
{
	if (!holdingOne)
	{
		if (PRESSING_UP(joystick) || PRESSING_DOWN(joystick) || PRESSING_X(joystick) || PRESSING_SELECT(joystick) || PRESSING_START(joystick)) //pressed any
		{
			dirty = true;
			holdingOne = true;
		}
		
		if (PRESSING_UP(joystick))
		{
			selected--;
			holdingOne = true;
			return fixSelected();
		}
		else if (PRESSING_DOWN(joystick))
		{
			selected++;
			holdingOne = true;
			return fixSelected();
		}
		else if (PRESSING_X(joystick) || PRESSING_START(joystick))
		{
			holdingOne = true;
			return pressCross();
		}
		else if (PRESSING_SELECT(joystick))
		{
			holdingOne = true;
			return pressSelect();
		}
	}
	else if (!(PRESSING_UP(joystick) || PRESSING_DOWN(joystick) || PRESSING_X(joystick) || PRESSING_SELECT(joystick) || PRESSING_START(joystick))) //aren't pressing any
		holdingOne = false;
	return inputableIndexVal;
}
	
void selector::inputableActivate()
{
	holdingOne = true;
	gui_active = true;
}

void selector::inputableDeactivate()
{
	gui_active = false;
}


bool selector::needsRedraw() const
{
	return dirty;
}

void selector::draw()
{
	dirty = false;
	if (selected < 0 || !gui_active)
		return;
	
	//update offset
	offset = targetOffset;
	offset.y += selected * itemSize;
	//draw
	SDL_BlitSurface(pixels, &rect, screen, &offset);
}
