#ifndef INCLUDED_SUPPORT_H
#define INCLUDED_SUPPORT_H

#include <list>
#include <string>
#include <SDL/SDL.h>
#include "dlib.h"
#include "textBits.h"

using namespace std;

//This file provides the text rendering stuffs.
//It still needs work TODO


#define FONT_HEIGHT 10
#define FONT_WIDTH 6

//extra amount of pixels to give so that the bottom of g,p type letters can be drawn on the bottom
#define FONT_EXTRA_HEIGHT 3

void support_init();

void printTextToSurface(textBlock &text, SDL_Surface* pixels, bool align, const int &linesToSkip);

#endif
