#ifndef INCLUDED_DBLIB_GUIBITS_TEXTLINEINPUT_H
#define INCLUDED_DBLIB_GUIBITS_TEXTLINEINPUT_H

#include "../inputable.h"
#include "../textBits.h"
#include "../support.h"
#include "../render.h"
#include "../keyboards/kbwrap.h"

class textLineInput : public inputable
{
public:
	virtual void clearArea();
	//Returns the index of the now active inputable (returns its own on no change)
	virtual string takeInput(SDL_Joystick* joystick);
	
	virtual void draw();
	virtual bool needsRedraw() const;


//should be private
	bool dirty;       //needs redrawing
protected:
	wstring realText; //store the message in an easy way
	textBlock text;   //used for rendering
	unsigned int color; //color of text
	unsigned int maxLength; //max length of area
	
	virtual void init(const unsigned int &nLength, const unsigned int &nX, const unsigned int &nY, const unsigned int &nColor);
	virtual string handleEnter() = 0;
	
	static keyboardWrapper* keyboard;
	
	static SDL_Surface* blinkerPic;
	static SDL_Rect blinkerRect;
	static unsigned int lastBlink;
	static bool blinkVisible;
	int cursorpos;
};

//Returned when the start key is pressed from in a textLineInput, this signifies that we want to switch areas
#warning TEXTLINEINPUT_SWITCH IS BAD, IT SHOULD BE SET ON ALL CHILD CLASSES INSTEAD
#warning IT CURRENTLY CAUSES CRASHES EVERYWHERE WHEN START IS PRESSED.
#define TEXTLINEINPUT_SWITCH "TEXTLINEINPUT_SWITCH"
#endif //INCLUDED_DBLIB_GUIBITS_TEXTLINEINPUT_H
