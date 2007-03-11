#include "textArea.h"
#include "../support.h"
#include "../render.h"

#define LOCKPIXELS(); 	if (SDL_MUSTLOCK(pixels)) SDL_LockSurface(pixels);
#define UNLOCKPIXELS();	if (SDL_MUSTLOCK(pixels)) SDL_UnlockSurface(pixels);

void textArea::clearArea()
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

textArea::textArea() { pixels=NULL; } //DONT USE
textArea::~textArea()
{
	if (pixels != NULL)
		SDL_FreeSurface(pixels);
	pixels = NULL;
}

//width/height are amount of chars
textArea::textArea(const int &newWidth, const int &newHeight, textBlock* nText)
{
	linesToSkip = 0;
	alignment = ALIGN_BOTTOM;
	width = newWidth;
	height = newHeight;
	pixels = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCALPHA, 
		FONT_WIDTH*width, FONT_HEIGHT*height+FONT_EXTRA_HEIGHT, 32, 
		//RGBA
		0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	
	//Now transparencyize the texture
	clearArea();
	text = nText;
	if (text != NULL) dirty = true;
	
	rect.x = 0;
	rect.y = 0;
	rect.w = pixels->w;
	rect.h = pixels->h;
	offset = screen_rect;
}

//Erase all current text and replace it with this text
void textArea::setText(textBlock* newText)
{
	clearArea();
	
	text = newText;
}

//Add this text to the end
void textArea::addText(const wstring &newText, const unsigned int &color)
{
	if (text != NULL)
	{
		text->addText(newText, color);
		dirty = true;
	}
}

//Get the text that is currently stored
textBlock* textArea::getText()
{
	return text;
}

void textArea::draw()
{
//	printf("textArea::draw()\n");
	if (dirty && text != NULL)
	{
		printf("Redrawing TA\n");
		clearArea();
		printTextToSurface(*text, pixels, alignment, linesToSkip);
	}
	dirty = false;
	guiBit::draw();
}

bool textArea::needsRedraw() const
{
	return dirty;
}

void textArea::setAlign(bool align)
{
	alignment = align;
	dirty = true;
}

void textArea::setLinesToSkip(int newSkip)
{
	if (linesToSkip != newSkip) dirty = true;
	linesToSkip = newSkip;
}


//list <wstring> text;
