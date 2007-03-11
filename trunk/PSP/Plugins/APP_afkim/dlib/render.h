#ifndef DLIB_INCLUDED_RENDER_H
#define DLIB_INCLUDED_RENDER_H

#include "dlib.h"

#include <SDL/SDL.h>

#include <list>

#include "guibit.h"

#define COLOR_WHITE   0x0FFFFFFF
#define COLOR_BLACK   0x000000FF
#define COLOR_RED     0xFF0000FF

#define TEXT_NORMAL_COLOR COLOR_BLACK
#define TEXT_ERROR_COLOR  COLOR_RED

//Initialize the renderer, returns 0 on success
int renderInit();

//Add a new GuiBit to the stack
void addGuiBit(guiBit* newBit);

//remove a guiBit from the stack
void removeGuiBit(guiBit* oldBit);

//render the current state of the gui
//true if redrew
bool renderGui();

//Gets the current list of guiBits
list<guiBit*> renderGetBitStack();

//Clears the current list of guiBits
void renderClearBitStack();

//Sets the current list of guiBits
void renderSetBitStack(list<guiBit*> newBitStack);

extern SDL_Surface *screen;
extern SDL_Rect screen_rect;
#endif //DLIB_INCLUDED_RENDER_H
