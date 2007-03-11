#include "render.h"
#include "guibit.h"
#include "support.h"

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <vector>

#ifdef PSP
#include <pspkernel.h>
#endif

using namespace std;

SDL_Surface *screen;
SDL_Rect screen_rect;

list<guiBit*> guiBitStack;

int renderInit()
{
	/* Initialize SDL?s video system and check for errors. */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		return 1;
	};
	
	atexit(SDL_Quit);
	
	screen = SDL_SetVideoMode(480, 272, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
	if (screen == NULL) {
		printf("Unable to set video mode: %s\n", SDL_GetError());
		return 1;
	};
	screen_rect.x = 0;
	screen_rect.y = 0;
	screen_rect.w = 480;
	screen_rect.h = 272;
	
	support_init();
	
	return 0;
}

void addGuiBit(guiBit* newBit)
{
	guiBitStack.push_back(newBit);
}

//remove a guiBit from the stack
void removeGuiBit(guiBit* oldBit)
{
	list<guiBit*>::iterator iter;
	for (iter = guiBitStack.begin(); iter != guiBitStack.end(); iter++)
	{
		if ((*iter) == oldBit)
		{
			guiBitStack.erase(iter);
			break;
		}
	}
}

bool renderGui()
{
	bool needToRun = false;
	list<guiBit*>::iterator iter;
	for (iter = guiBitStack.begin(); iter != guiBitStack.end(); iter++)
	{
		if ((*iter)->needsRedraw())
		{	needToRun = true; break; }
	}
	if (!needToRun) return false;
	
	for (iter = guiBitStack.begin(); iter != guiBitStack.end(); iter++)
	{
		(*iter)->draw();
	}
	
	SDL_Flip(screen);
	
	return true;
}

list<guiBit*> renderGetBitStack()
{
	return guiBitStack;
}

void renderClearBitStack()
{
	guiBitStack.clear();
}

void renderSetBitStack(list<guiBit*> newBitStack)
{
	guiBitStack = newBitStack;
}