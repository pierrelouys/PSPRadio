#ifndef INCLUDED_DLIB_INPUTABLE
#define INCLUDED_DLIB_INPUTABLE

#include "guibit.h"
#include <map>

class inputable : public guiBit
{
public:
	//Returns the index of the now ative inputable (returns its own on no change)
	virtual string takeInput(SDL_Joystick* joystick) = 0;
	
	virtual string getInputKey() const = 0; //gets the string used to identify this inputable in the inputable map
	//set this object to be the currently selected/not currently selected inputable object
	//the object could remove/add itself to the guilist (this is the default behaviour)
	virtual void inputableActivate();
	virtual void inputableDeactivate();
	
//private:
	static bool holdingOne; //user is holding a key
	
	bool gui_active; //is the selected component;
};

//HACK
extern map<string, inputable*> inputs;

//Special return val for takeInput, quits the program.
#define SWITCH_QUIT "QUIT_THE_APP_PLZ"

#endif //INCLUDED_DLIB_INPUTABLE
