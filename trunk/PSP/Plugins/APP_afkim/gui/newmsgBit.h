#ifndef INCLUDED_GUI_NEWMSGBIT_H
#define INCLUDED_GUI_NEWMSGBIT_H

#include "../dlib/guibit.h"


class newmsgBit : public guiBit
{
public:
	newmsgBit(string filename);
	void enable();
	void disable();

	bool needsRedraw() const; //return true if this gui element has changed since last redraw
	
	void draw();
private:
	bool dirty;
	bool enabled;
};

#endif //INCLUDED_GUI_NEWMSGBIT_H
