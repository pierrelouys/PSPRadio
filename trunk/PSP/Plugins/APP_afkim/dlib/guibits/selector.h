#ifndef INCLUDED_DLIB_GUIBITS_SELECTOR
#define INCLUDED_DLIB_GUIBITS_SELECTOR

#include "../inputable.h"

class selector : public inputable
{
public:
//	selector(...);
	//Returns the index of the now active inputable (returns its own on no change)
	virtual string takeInput(SDL_Joystick* joystick);
	
	virtual void draw();
	virtual bool needsRedraw() const;

	virtual void inputableActivate();
	virtual void inputableDeactivate();
	bool dirty;
protected:
	//functions called when things happen in takeInput,
	//the value returned here is the value returned to takeInput
	virtual string fixSelected() = 0;
	virtual string pressCross() = 0;
	virtual string pressSelect() = 0;
	void init(const unsigned int &targetX, const unsigned int &targetY, const unsigned int &nItemSize, const int &nSelected, const string &image);
	
	int selected;
	SDL_Rect targetOffset;
	unsigned int itemSize;
};

#endif
