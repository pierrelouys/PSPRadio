#ifndef INCLUDED_DLIB_KEYBOARDS_KEYBOARDWRAPPER_H
#define INCLUDED_DLIB_KEYBOARDS_KEYBOARDWRAPPER_H


//Copy from danzeff
#define DANZEFF_LEFT   1
#define DANZEFF_RIGHT  2
#define DANZEFF_SELECT 3
#define DANZEFF_START  4


//wrapper interface for keyboards
class keyboardWrapper
{
public:
	virtual unsigned int readInput(SDL_Joystick* joystick) = 0;
	virtual void moveTo(const int &newX, const int &newY) = 0;
	virtual bool dirty() = 0;
	virtual void render() = 0;
	virtual void lock() = 0;	//locks the X key to not do anything untill released
	
	virtual ~keyboardWrapper() {};
};

//implementations of the wrapper
class danzeffWrapper : public keyboardWrapper
{
public:
	danzeffWrapper();
	~danzeffWrapper();
	unsigned int readInput(SDL_Joystick* joystick);
	void moveTo(const int &newX, const int &newY);
	bool dirty();
	void render();
	void lock();	//locks the X key to not do anything untill released
private:
	unsigned int lastChangeTime; //time user change status last (0 = who cares) SDL_GetTicks milliseconds
	bool timedOut;
	bool extraDirty; //internal dirty variable for when we need redrawing
};

class psprintWrapper : public keyboardWrapper
{
public:
	psprintWrapper();
	~psprintWrapper();
	unsigned int readInput(SDL_Joystick* joystick);
	void moveTo(const int &newX, const int &newY);
	bool dirty();
	void render();
	void lock();	//locks the X key to not do anything untill released
private:
	bool isDirty;

	static int currentPSMODE;
	static bool showKeyboard;
	static SDL_Surface* keyboardPic;
	static SDL_Rect keyboardRect;
	
	static SDL_Surface* lettersPic;
	static SDL_Rect lettersRect;

	static SDL_Surface* numbersPic;
	static SDL_Rect numbersRect;
	
	static bool holdingSpecial;
	static bool holdingX;
};

#endif //INCLUDED_DLIB_KEYBOARDS_KEYBOARDWRAPPER_H
