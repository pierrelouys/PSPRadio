#include "newmsgBit.h"
#include <SDL/SDL_image.h>
#include "../dlib/render.h"

newmsgBit::newmsgBit(string filename)
{
	pixels = IMG_Load(filename.c_str());
	if (pixels == NULL) {
		printf("Unable to load image!\n");
		return;
	}
	
	rect.x = 0;
	rect.y = 0;
	rect.w = pixels->w;
	rect.h = pixels->h;
	offset = screen_rect;
	
	enabled = false;
}

void newmsgBit::enable()
{
	if (!enabled) dirty = true;

	enabled = true;
}

void newmsgBit::disable()
{
	if (enabled) dirty = true;
	
	enabled = false;
}

bool newmsgBit::needsRedraw() const //return true if this gui element has changed since last redraw
{
	return dirty;
}

void newmsgBit::draw()
{
	dirty = false;
	if (!enabled) return;
	
	SDL_BlitSurface(pixels, &rect, screen, &offset);
}

