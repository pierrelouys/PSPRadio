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
#include <stdarg.h>
#include <PSPApp.h>
#include "ScreenHandler.h"
#include "PSPRadio_Exports.h"
#include "Main.h"

int module_stop(int args, void *argp)
{
	return 0;
}

int PSPRadioExport_Log(char *file, int line, loglevel_enum LogLevel, char *strFormat, ...)
{
	va_list args;
	char logmessage[1024];

	if (pLogging && LogLevel >= pLogging->GetLevel()) /** Log only if Set() was called and loglevel is correct */
	{
		va_start (args, strFormat);         /* Initialize the argument list. */

		vsprintf(logmessage, strFormat, args);
		
		pLogging->Log_(file, line, LogLevel, logmessage);

		va_end (args);                  /* Clean up. */
	}
	
	return 0;
}

char *PSPRadioExport_GetProgramVersion()
{
	return gPSPRadio->GetProgramVersion();
}

bool PSPRadioExport_IsUSBEnabled()
{
	return gPSPRadio->IsUSBEnabled();
}

char *PSPRadioExport_GetMyIP()
{
	return gPSPRadio->GetMyIP();
}

void PSPRadioExport_RequestExclusiveAccess()
{
	gPSPRadio->SetPluginExclisiveAccess(true);
	if (gPSPRadio->GetUI())
	{
		gPSPRadio->GetUI()->OnScreenshot(CScreenHandler::PSPRADIO_SCREENSHOT_ACTIVE);
	}
}

void PSPRadioExport_GiveUpExclusiveAccess()
{
	if (gPSPRadio->GetUI())
	{
		gPSPRadio->GetUI()->OnScreenshot(CScreenHandler::PSPRADIO_SCREENSHOT_NOT_ACTIVE);
		/** Re-draw the current screen */
		gPSPRadio->GetScreenHandler()->GetCurrentScreen()->Activate(gPSPRadio->GetScreenHandler()->GetCurrentUIPtr());
	}
	gPSPRadio->SetPluginExclisiveAccess(false);
}

char *PSPRadioExport_GetVersion()
{
	return PSPRADIO_VERSION;
}
