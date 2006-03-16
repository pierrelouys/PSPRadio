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
#include <pspsdk.h>
#include "PSPRadio.h"

#ifdef DYNAMIC_BUILD
	PSP_MODULE_INFO("PSPRADIO_PRX", 0x0/**USER MODULE */, 0, 1);
	PSP_MAIN_THREAD_PARAMS(/*0x20*/80, 64, PSP_THREAD_ATTR_USER);
	PSP_MAIN_THREAD_NAME("PSPRadioPRX");
	PSP_HEAP_SIZE_KB(8192);
#else
	PSP_MODULE_INFO("PSPRADIO", 0x0, 0, 1); 	 
 	PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);
	PSP_MAIN_THREAD_PRIORITY(80);
#endif

CPSPRadio *gPSPRadio = NULL;

/** main */
int main(int argc, char **argv)
{
	rootScreen = new CScreen();
	
	rootScreen->SetBackgroundImage("Init.png");
	rootScreen->Clear();

	gPSPRadio = new CPSPRadio();
	if (gPSPRadio)
	{
		gPSPRadio->Setup(argc, argv);
		Log(LOG_INFO, "PSPRadio() Main, Calling ProcessEvents()");
		for (int i = 0; i < argc ; i++)
		{
			if (i == 3)
			{
				Log(LOG_ALWAYS, "TEXT_ADDR: PSPRadio.prx: Main(Arg %d)='%s'", i, argv[i]); /** Log text address for debugging */
			}
			else
			{
				Log(LOG_LOWLEVEL, "Main(Arg %d)='%s'", i, argv[i]);
			}
		}
		
		gPSPRadio->ProcessEvents();

		delete(gPSPRadio); gPSPRadio = NULL;
	}

	return 0;

}
