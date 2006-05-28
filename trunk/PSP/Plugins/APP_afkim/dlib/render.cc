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

/*#ifdef PSP

SceUID rendererSemaphore;
SceUInt semTimeout = 1000*1000*60; // a minute
#define INIT_MUTEX(); rendererSemaphore = sceKernelCreateSema("rendererSemaphore", 0, 1, 1, 0);
#define LOCK_MUTEX(); sceKernelWaitSema(rendererSemaphore, 1, &semTimeout);
#define FREE_MUTEX(); sceKernelSignalSema(rendererSemaphore, 1);

#else //not psp, don't do any mutex stuff*/
#define INIT_MUTEX(); 
#define LOCK_MUTEX(); 
#define FREE_MUTEX(); 
//#endif

list<guiBit*> guiBitStack;

int renderInit()
{
	/* Initialize SDL?s video system and check for errors. */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		return 1;
	};
	
	atexit(SDL_Quit);
	
	screen = SDL_SetVideoMode(480, 272, 32, 0);
	if (screen == NULL) {
		printf("Unable to set video mode: %s\n", SDL_GetError());
		return 1;
	};
	screen_rect.x = 0;
	screen_rect.y = 0;
	screen_rect.w = 480;
	screen_rect.h = 272;
	
	support_init();
	
	//setup the mutex
	INIT_MUTEX();
	return 0;
}

void addGuiBit(guiBit* newBit)
{
	LOCK_MUTEX();
//	printf("GUI Adding %x\n",newBit);
	guiBitStack.push_back(newBit);
	
	FREE_MUTEX();
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
	LOCK_MUTEX();
	
	bool needToRun = false;
	list<guiBit*>::iterator iter;
	for (iter = guiBitStack.begin(); iter != guiBitStack.end(); iter++)
	{
		if ((*iter)->needsRedraw())
		{	needToRun = true; break; }
	}
	if (!needToRun) return false;
	printf("<GUI>\n"); //TODO REMOVE THIS LINE
	for (iter = guiBitStack.begin(); iter != guiBitStack.end(); iter++)
	{
//		printf("Drawing %x\n", (*iter));
		(*iter)->draw();
	}
	
	SDL_UpdateRect(screen, 0, 0, 0, 0);
	printf("</GUI>\n");
	FREE_MUTEX();
	return true;
}
