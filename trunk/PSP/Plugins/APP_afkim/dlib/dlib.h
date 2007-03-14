#ifndef INCLUDED_DLIB_DLIB_H
#define INCLUDED_DLIB_DLIB_H

#include <string>
using namespace std;

//I'd love to typedef this but it don't fucking work, i think its gcc3 vs gcc4
#ifdef PSP
typedef basic_string<wchar_t> wstring;
#else //on pc
#define wstring basic_string<wchar_t>
#endif

#define ALIGN_TOP    true
#define ALIGN_BOTTOM false

#ifdef PSP
#include <netinet/tcp.h>
#include <sys/fd_set.h>
#include <sys/select.h>
#include <pspkernel.h>
#include <unistd.h>
#define usleep(a); sceKernelDelayThread(a);
#endif


///Setup some generic key handlers, will need these for each system that dlib is built on.
#ifdef PSP
	#define PRESSING_UP(joystick)       (SDL_JoystickGetButton(joystick, 8))
	#define PRESSING_DOWN(joystick)     (SDL_JoystickGetButton(joystick, 6))
	#define PRESSING_LEFT(joystick)     (SDL_JoystickGetButton(joystick, 7))
	#define PRESSING_RIGHT(joystick)    (SDL_JoystickGetButton(joystick, 9))
	
	#define PRESSING_X(joystick)        (SDL_JoystickGetButton(joystick, 2))
	#define PRESSING_O(joystick)        (SDL_JoystickGetButton(joystick, 1))
	#define PRESSING_SQUARE(joystick)   (SDL_JoystickGetButton(joystick, 3))
	#define PRESSING_TRIANGLE(joystick) (SDL_JoystickGetButton(joystick, 0))
	
	#define PRESSING_SELECT(joystick)   (SDL_JoystickGetButton(joystick, 10))
	#define PRESSING_START(joystick)    (SDL_JoystickGetButton(joystick, 11))
	
	#define PRESSING_LTRIGGER(joystick) (SDL_JoystickGetButton(joystick, 4))
	#define PRESSING_RTRIGGER(joystick) (SDL_JoystickGetButton(joystick, 5))
#else //Not PSP, PC?
	#define PRESSING_UP(joystick)       (SDL_JoystickGetAxis(joystick, 5)<0)
	#define PRESSING_DOWN(joystick)     (SDL_JoystickGetAxis(joystick, 5)>0)
	#define PRESSING_LEFT(joystick)     (SDL_JoystickGetAxis(joystick, 6)<0)
	#define PRESSING_RIGHT(joystick)    (SDL_JoystickGetAxis(joystick, 6)>0)
	
	#define PRESSING_X(joystick)        (SDL_JoystickGetButton(joystick, 2))
	#define PRESSING_O(joystick)        (SDL_JoystickGetButton(joystick, 1))
	#define PRESSING_SQUARE(joystick)   (SDL_JoystickGetButton(joystick, 3))
	#define PRESSING_TRIANGLE(joystick) (SDL_JoystickGetButton(joystick, 0))
	
	#define PRESSING_SELECT(joystick)   (SDL_JoystickGetButton(joystick, 9))
	#define PRESSING_START(joystick)    (SDL_JoystickGetButton(joystick, 8))
	
	#define PRESSING_LTRIGGER(joystick) (SDL_JoystickGetButton(joystick, 4) || SDL_JoystickGetButton(joystick, 6))
	#define PRESSING_RTRIGGER(joystick) (SDL_JoystickGetButton(joystick, 5) || SDL_JoystickGetButton(joystick, 7))

#endif //PSP


#endif
