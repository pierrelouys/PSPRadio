#ifndef psp_curses_h
	#define psp_curses_h

	#include <stdio.h>
	#include <pspctrl.h>
	#include <pspiofilemgr_dirent.h>
	#include <dirent.h>

	#ifndef tBoolean
	#define tBoolean int
	#define truE 1
	#define falsE 0
	#define KEY_BACKSPACE 8
	#endif
		

	void PSPPutch(char ch);

	int env_termsize(int *x, int *y);

	void PSPInputHandlerStart();
	void PSPInputHandlerEnd();
	int PSPInputHandler(SceCtrlData pad, char *key);

	extern tBoolean g_InputMethod;
	extern volatile tBoolean g_PSPEnableRendering;
	extern volatile tBoolean g_PSPEnableInput;

#endif
