#ifndef psp_curses_h
	#define psp_curses_h

	#include <stdio.h>
	#include <pspiofilemgr_dirent.h>
	#include <dirent.h>
	

	#include "stuff.h"

	#include "cursesbi.h"

	int pipe_open(int *fdpair);
	int pipe_close(int *fdpair);
	int pipe_read(int fd, void *buf, size_t len);
	int pipe_write(int fd, void *buf, size_t len);
	int env_termsize(int *x, int *y);



#if 0
	#define TRUE 1
	#define FALSE 0
	#define ERR -1
	#define COLOR_PAIRS 64

	#define A_BOLD 0x1
	#define A_REVERSE 0x2
	#define A_UNDERLINE 0x4

	enum keys
	{
		KEY_DOWN,
		KEY_UP,
		KEY_LEFT,
		KEY_RIGHT,
		KEY_END,
		KEY_ENTER,
		KEY_HOME,
		KEY_IC,
		KEY_NPAGE,
		KEY_DC,
		KEY_PPAGE,
		KEY_CANCEL,
		KEY_BACKSPACE,
		KEY_REVERSE,


	};

	#define LINES 40
	#define COLS  80


	//int stdscr();
	typedef struct STDSCR
	{
		int _cury;
		int _curx;
	} std_scr;

	extern std_scr *stdscr;


	#define attr_t int
#endif
	///thread:
	#include <pspkernel.h>
	#include <pspkerneltypes.h>
	
#endif
