#include "guibit.h"
#include "render.h"
#include <SDL/SDL_image.h>
#include <iostream>

guiBit::guiBit()
{
	pixels = NULL;
}


guiBit::guiBit(string filename, bool performDisplayFormat)
{
	pixels = IMG_Load(filename.c_str());
	if (pixels == NULL) {
		printf("Unable to load image!\n");
		return;
	}
	if (performDisplayFormat)
	{
		SDL_Surface* tmp = SDL_DisplayFormat(pixels);
		if (tmp != NULL)
		{
			printf("Made DF: %s\n", filename.c_str());
			SDL_FreeSurface(pixels);
			pixels = tmp;
			tmp = NULL;
		}
		else
		{
			printf("DF FAILED: %s\n", filename.c_str());
		}
	}
	
	rect.x = 0;
	rect.y = 0;
	rect.w = pixels->w;
	rect.h = pixels->h;
	offset = screen_rect;
}

guiBit::~guiBit()
{
	if (pixels != NULL)
		SDL_FreeSurface(pixels);
}

bool guiBit::needsRedraw() const
{
	//we are a static element, always return false
	return false;
}

void guiBit::draw()
{
//	printf("guiBit::draw - %x - (%i,%i) (%i,%i)\n", pixels, rect.x, rect.y, rect.w, rect.h);
	SDL_BlitSurface(pixels, &rect, screen, &offset);
}

void guiBit::setOffset(const int &newX, const int &newY, const int &newW, const int &newH)
{
	rect.x = newX;
	rect.y = newY;
	rect.w = newW;
	rect.h = newH;
}

void guiBit::moveTo(int x, int y) //move the top left render pos to x,y (pixels)
{
	offset.x = x;
	offset.y = y;
}
