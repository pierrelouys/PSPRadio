#ifndef INCLUDED_GUIBIT_H
#define INCLUDED_GUIBIT_H

//#include <vector>
#include <string>
#include <SDL/SDL.h>

using namespace std;

class guiBit
{
public:
	guiBit();//NEVER USE
	/*	Create a guiBit from the given image.
		If performDisplayFormat is true, SDL_DisplayFormat will be called on the loaded image.
		This increases rendering speed, but disables transparency
	*/
	guiBit(string filename, bool performDisplayFormat);
	
	virtual ~guiBit();
	
	virtual bool needsRedraw() const; //return true if this gui element has changed since last redraw
	virtual void draw();
	
	//Set the internal draw dimensions of the surface (this doesn't makes sence on some guibits)
	void setOffset(const int &newX, const int &newY, const int &newW, const int &newH);
	void moveTo(int x, int y); //move the top left render pos to x,y (pixels)
protected:
	#warning Need to integrate dirty variable into guibit
	SDL_Rect rect;
	SDL_Surface* pixels;
	
	SDL_Rect offset;
};

#endif //INCLUDED_GUIBIT_H
