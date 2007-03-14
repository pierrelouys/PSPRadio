#include "textLineInput.h"
#include "../keyboards/kbwrap.h"
#include <iostream>
#include <fstream>
#include <SDL/SDL_image.h>

SDL_Surface* textLineInput::blinkerPic = NULL;
SDL_Rect textLineInput::blinkerRect;
unsigned int textLineInput::lastBlink = 0;
bool textLineInput::blinkVisible = 0;
keyboardWrapper* textLineInput::keyboard = NULL;


#define LOCKPIXELS(); 	if (SDL_MUSTLOCK(pixels)) SDL_LockSurface(pixels);
#define UNLOCKPIXELS();	if (SDL_MUSTLOCK(pixels)) SDL_UnlockSurface(pixels);

void textLineInput::init(const unsigned int &nLength, const unsigned int &nX, const unsigned int &nY, const unsigned int &nColor)
{
	gui_active = false;
	
	if (blinkerPic == NULL)
	{
		ifstream infile("./afkim.cfg");
		string inputType;
		infile >> inputType;
		if (inputType == "psprint")
			keyboard = new psprintWrapper();
		else
		{
			keyboard = new danzeffWrapper();
			keyboard->moveTo(228,6);
		}
		blinkerPic = IMG_Load("./pics/blinker.png");
		blinkerRect.x = 0;
		blinkerRect.y = 0;
		blinkerRect.w = blinkerPic->w;
		blinkerRect.h = blinkerPic->h;
		lastBlink = 0;
		blinkVisible = false;
	}
	
//	alignment = ALIGN_BOTTOM;
	pixels = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCALPHA,
		FONT_WIDTH*nLength, FONT_HEIGHT+FONT_EXTRA_HEIGHT, 32,
		//RGBA
		0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
		
		//Now transparencyize the texture
	clearArea();
	
	text = textBlock(nLength, 1);
	
	color = nColor;
	maxLength = nLength;
	rect.x = 0;
	rect.y = 0;
	rect.w = pixels->w;
	rect.h = pixels->h;
	offset = screen_rect;
	offset.x = nX;
	offset.y = nY;
	
	cursorpos = 0;
}

void textLineInput::clearArea()
{
	LOCKPIXELS();
	unsigned int* thePixels = (unsigned int*)pixels->pixels;
	for (int a = 0;a < pixels->w * pixels->h; a++)
	{
		thePixels[a] = 0xFFFFFF00;
	}
	UNLOCKPIXELS();
	
	dirty = true;
}

void textLineInput::clearText()
{
	realText.clear();
	cursorpos = 0;
	dirty = true;
}


string textLineInput::takeInput(SDL_Joystick* joystick)
{
	if (isHoldingOne())
		return getInputKey();
	
	unsigned int pressed = keyboard->readInput(joystick);
	
	if (pressed == DANZEFF_START) // switch input area button (start on danzeff, L on p_sprint) 
	{
		return handleStart();
	}
	else if (pressed == DANZEFF_SELECT) //select
	{
		return "menuMain"; //HACK: This isn't really correct, although it is okay ATM
	}
	else if (pressed == DANZEFF_LEFT)	//left
	{
		if (cursorpos > 0)
		{
			dirty = true;
			cursorpos--;
		}
		return getInputKey();
	}
	else if (pressed == DANZEFF_RIGHT)	//right
	{
		if (realText.length()!=0 && cursorpos < realText.length())
		{
			dirty = true;
			cursorpos++;
		}
		return getInputKey();
	}
	
	switch(pressed)
	{
	case 0:
		break;
	case 8: //backspace
		if (cursorpos != 0)
		{
			realText.erase(cursorpos-1, 1);
			cursorpos--;
		}
		dirty = true;
		break;
	case '\r': //enter
	case '\n': //enter
		return handleEnter();
		break;
	default:
		if (realText.length() != maxLength)
		{
			
			realText.insert(cursorpos, 1, (wchar_t)pressed);
			cursorpos++;
		
			dirty = true;
		}
	}
	
	return getInputKey();
}

void textLineInput::draw()
{
//	cout << "TLI " << realText.size() << endl;
	if (dirty)
	{
		printf("Redrawing TLI\n");
		clearArea();
		//This is a dirty way to render multiple spaces in a row:
		text.reset();
		textLine t;
		t.addText(realText, color);
		text.lines.push_back(t);
		printTextToSurface(text, pixels, ALIGN_TOP, 0);
	}
	dirty = false;
	guiBit::draw();
	
	if (gui_active)
		keyboard->render();
	
	if (lastBlink+500 < SDL_GetTicks())
	{
		blinkVisible = !blinkVisible;
		lastBlink = SDL_GetTicks();
	}
	if (gui_active && blinkVisible)
	{
		SDL_Rect drawOffset = screen_rect; //should always be 0,0
		drawOffset.x = offset.x+1-(FONT_WIDTH/2) + (FONT_WIDTH*cursorpos);
		drawOffset.y = offset.y;
		SDL_BlitSurface(blinkerPic, &blinkerRect, screen, &drawOffset);
	}
}

bool textLineInput::needsRedraw() const
{
	if (gui_active && keyboard->dirty())
		return true;

	if (gui_active && lastBlink+500 < SDL_GetTicks())
		return true;
	
	return dirty;
}
