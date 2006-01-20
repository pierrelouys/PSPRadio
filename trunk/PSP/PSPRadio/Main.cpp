/*
	PSPRadio / Music streaming client for the PSP. (Initial Release: Sept. 2005)
	Copyright (C) 2005  Rafael Cabezas a.k.a. Raf

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#ifdef DEBUG
	#include <pspdebug.h>
	#include <pspkernel.h>
	#include <pspdebug.h>
	#include <pspdisplay.h>
	extern volatile bool flagGdbStubReady;
	/** Driver Loader Thread handle */
	extern int handleDriverLoaderThread;
#endif
#include <PSPApp.h>
#include <PSPSound.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <malloc.h>
#include <iniparser.h>
#include <Tools.h>
#include <Logging.h>
#include <pspwlan.h>
#include <psphprm.h>
#include <psprtc.h>
#include "ScreenHandler.h"
#include "PlayListScreen.h"
#include "SHOUTcastScreen.h"
#include "TextUI.h"
#include "TextUI3D.h"
#include <ivorbisfile.h>
#include "Screen.h"
#include "PSPRadio.h"

PSP_MAIN_THREAD_PRIORITY(80);
//PSP_MAIN_THREAD_STACK_SIZE_KB(512);

/** main */
int main(int argc, char **argv)
{
	#ifdef DEBUG
		/** Wait for GdbStub to be ready -- Thanks to bengarney for gdb wifi! */
		while(!flagGdbStubReady)
		   sceKernelDelayThread(5000000);
		sceKernelWaitThreadEnd(handleDriverLoaderThread, NULL);
		pspDebugScreenPrintf("After sceKernelWaitThreadEnd. Generating breakpoint.\n");
		/* Generate a breakpoint to trap into GDB */
		pspDebugBreakpoint();
	#endif


	rootScreen.SetBackgroundImage("Init.png");
	rootScreen.Clear();

	CPSPRadio *PSPRadio = new CPSPRadio();
	if (PSPRadio)
	{
		PSPRadio->Setup(argc, argv);
		PSPRadio->ProcessEvents();

		delete(PSPRadio);
	}

	return 0;

}
