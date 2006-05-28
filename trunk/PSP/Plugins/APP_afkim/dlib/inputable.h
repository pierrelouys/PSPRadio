#ifndef INCLUDED_DLIB_INPUTABLE
#define INCLUDED_DLIB_INPUTABLE

#include "guibit.h"


class inputable : public guiBit
{
public:
	//Returns the index of the now ative inputable (returns its own on no change)
	virtual int takeInput(SDL_Joystick* joystick) = 0;
	
	//set the objects index value
	virtual void setIndexVal(const int inputVal);
	//set this object to be the currently selected/not currently selected inputable object
	//the object could remove/add itself to the guilist (this is the default behaviour)
	virtual void inputableActivate();
	virtual void inputableDeactivate();
	
//private:
	static bool holdingOne; //user is holding a key
	
	int inputableIndexVal;
	bool gui_active; //is the selected component;
};

//HACK
extern vector<inputable*> inputs;

//Special return val for takeInput, quits the program.
#define SWITCH_QUIT -1

#endif //INCLUDED_DLIB_INPUTABLE
