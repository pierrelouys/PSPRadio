#include <SDL/SDL_image.h>
#include "../render.h"

//danzeff
#include <danzeff.h>

//p_sprint
#include "p_sprint.h"
#include "kbwrap.h"


danzeffWrapper::danzeffWrapper()
{
	timedOut = false;
	lastChangeTime = SDL_GetTicks();
	danzeff_load_lite();
	danzeff_set_screen(screen);
	extraDirty = false;
}

danzeffWrapper::~danzeffWrapper()
{
	danzeff_free(); // - there better not ever be more than one of these objects around...
}

unsigned int danzeffWrapper::readInput(SDL_Joystick* joystick)
{
	unsigned int result =  danzeff_readInput(joystick);
	if (result != 0)
	{
		timedOut = false;
		lastChangeTime = SDL_GetTicks();
		extraDirty = true;
	}
	return result;
}

void danzeffWrapper::moveTo(const int &newX, const int &newY)
{
	danzeff_moveTo(newX, newY);
}

bool danzeffWrapper::dirty()
{
	bool isdirty = danzeff_dirty();
	if (isdirty)
	{
		timedOut = false;
		lastChangeTime  = SDL_GetTicks();
		extraDirty = true;
	}
	//update the timeout timer in here ;)
	if (lastChangeTime + 5000 < SDL_GetTicks())
	{
		if (!timedOut) extraDirty = true;
		timedOut = true;
	}
	
	return isdirty | extraDirty;
}

void danzeffWrapper::render()
{
	extraDirty = false;
	if (timedOut) return;
	
	danzeff_render();
}
void danzeffWrapper::lock() //locks the X key to not do anything untill released
{
	timedOut = false;
	lastChangeTime = SDL_GetTicks();
	extraDirty = true;
	//do nothing
}


//P_Sprintf Wrapper below:::


bool psprintWrapper::showKeyboard = false;
SDL_Surface* psprintWrapper::keyboardPic = NULL;
SDL_Rect psprintWrapper::keyboardRect;
	
SDL_Surface* psprintWrapper::lettersPic = NULL;
SDL_Rect psprintWrapper::lettersRect;

SDL_Surface* psprintWrapper::numbersPic = NULL;
SDL_Rect psprintWrapper::numbersRect;
bool psprintWrapper::holdingSpecial = true;
bool psprintWrapper::holdingX = true;

int psprintWrapper::currentPSMODE = P_SP_KEYGROUP_DEFAULT;

psprintWrapper::psprintWrapper()
{
	if (keyboardPic == NULL)
	{
		keyboardPic = IMG_Load("./pics/keyboard.png");
		keyboardRect.x = 0;
		keyboardRect.y = 0;
		keyboardRect.w = keyboardPic->w;
		keyboardRect.h = keyboardPic->h;
	
		lettersPic = IMG_Load("./pics/letters.png");
		lettersRect.x = 0;
		lettersRect.y = 0;
		lettersRect.w = lettersPic->w;
		lettersRect.h = lettersPic->h;

		numbersPic = IMG_Load("./pics/numbers.png");
		numbersRect.x = 0;
		numbersRect.y = 0;
		numbersRect.w = numbersPic->w;
		numbersRect.h = numbersPic->h;
	}
}

psprintWrapper::~psprintWrapper()
{
	//TODO
}

#include "pspctrl_emu.h"

unsigned int psprintWrapper::readInput(SDL_Joystick* joystick)
{
	if (holdingX && PRESSING_X(joystick))
		return 0;
	else
		holdingX = false;
	
	if (!holdingSpecial)
	{
		if (PRESSING_SELECT(joystick))
		{
			holdingSpecial = true;
			return DANZEFF_SELECT;
		}
		else if (PRESSING_LTRIGGER(joystick))
		{
			holdingSpecial = true;
			return DANZEFF_START;
		}
		else if (PRESSING_RTRIGGER(joystick))
		{
			holdingSpecial = true;
			isDirty = true;
			showKeyboard = !showKeyboard;
			return 0;
		}
	}
	if (PRESSING_LTRIGGER(joystick) || PRESSING_RTRIGGER(joystick) || PRESSING_SELECT(joystick))
	{
		holdingSpecial = true;
		return 0;
	}
	else
		holdingSpecial = false;
		
	SceCtrlData pspctrl = getCtrlFromJoystick(joystick);
	
	pspctrl.Buttons &= ~(PSP_CTRL_RTRIGGER | PSP_CTRL_LTRIGGER | PSP_CTRL_SELECT); //disable these keys

	struct p_sp_Key myKey;
	if (p_spReadKeyEx(&myKey, pspctrl.Buttons))
	{
		//ToDo: L, left, right
		printf("pressed: %i %i,%i\n", myKey.keychar, '\r', '\n');
		return myKey.keychar;
	}
	else
		return 0;
}

void psprintWrapper::moveTo(const int &newX, const int &newY)
{
//nothing
}

bool psprintWrapper::dirty()
{
	if (currentPSMODE != p_sp_GetActiveGroup())
		isDirty = true;
	
	return isDirty;
}

void psprintWrapper::render()
{
	isDirty = false;

	if (showKeyboard)
	{
		SDL_Rect keyOffset = screen_rect; //should always be 0,0
		SDL_BlitSurface(keyboardPic, &keyboardRect, screen, &keyOffset);
		currentPSMODE = p_sp_GetActiveGroup();
		
		if (p_sp_GetActiveGroup() == P_SP_KEYGROUP_DEFAULT)
		{
			SDL_Rect drawOffset = screen_rect; //should always be 0,0
			drawOffset.x = 430;
			drawOffset.y = 158;
			
			SDL_BlitSurface(lettersPic, &lettersRect, screen, &drawOffset);
		}
		else if (p_sp_GetActiveGroup() == P_SP_KEYGROUP_NUMFN)
		{
			SDL_Rect drawOffset = screen_rect; //should always be 0,0
			drawOffset.x = 430;
			drawOffset.y = 158;
			
			SDL_BlitSurface(numbersPic, &numbersRect, screen, &drawOffset);
		}
	}
}

void psprintWrapper::lock() //locks the X key to not do anything untill released
{
	holdingX = true;
	//do nothing
}
