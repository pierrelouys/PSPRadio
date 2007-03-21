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

PSP_MODULE_INFO("PSPRADIO_MAIN", 0x0/**USER MODULE */, 0, 1);
PSP_MAIN_THREAD_PARAMS(/*Prio*/80, /*Stack KB*/512, PSP_THREAD_ATTR_USER);
PSP_MAIN_THREAD_NAME("PSPRadioMain");
PSP_HEAP_SIZE_KB(5*1024);

CPSPRadio *gPSPRadio = NULL;

/** main */
int main(int argc, char **argv)
{
	rootScreen = new CScreen(false, 2); /* two buffers */

	rootScreen->Clear(0);
	rootScreen->SetFrameBuffer(0);
	rootScreen->LoadBuffer(1, "Init.png");
	rootScreen->SetFrameBuffer(1);

	gPSPRadio = new CPSPRadio();
	if (gPSPRadio)
	{
		gPSPRadio->Main(argc, argv);

		delete(gPSPRadio); gPSPRadio = NULL;
	}

	return 0;

}
