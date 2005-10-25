/* 
	PSPApp C++ OO Application Framework. (Initial Release: Sept. 2005)
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
#include <new>
#include <pspdisplay.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include <pspnet.h>
#include <psphprm.h>
#include <psppower.h>
#include "PSPApp.h"

#undef ReportError

PSP_MODULE_INFO("PSPAPP", 0x1000, 0, 1);
/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);
//extern SceModuleInfo module_info;

class CPSPApp *pPSPApp = NULL; /** Do not access / Internal Use. */

CPSPApp::CPSPApp(char *strProgramName, char *strVersionNumber)
{
	m_Exit = false;
	m_NetworkEnabled = false;
	m_USBEnabled = false;
	m_ExitSema = NULL;
	m_EventToPSPApp = NULL;
	m_thCallbackSetup = NULL; /** Callback thread */
	m_thRun = NULL; /** Run Thread */
	memset(&m_pad, 0, sizeof (SceCtrlData));
	strcpy(m_strMyIP, "0.0.0.0");
	m_ResolverId = 0;
	m_strProgramName = strdup(strProgramName);
	m_strVersionNumber = strdup(strVersionNumber);
	pPSPApp = this;
	m_Polling = false;
	
	m_thCallbackSetup = new CPSPThread("update_thread", callbacksetupThread, 100, 1024, THREAD_ATTR_USER);
	m_ExitSema = new CSema("PSPApp_Exit_Sema");

	if (m_thCallbackSetup)
	{
		m_thCallbackSetup->Start();
		
		sceCtrlSetSamplingCycle(0);
		sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
		
		m_Exit = false;
	}
	else /** Oops, error, let's exit the app */
	{
		m_Exit = true;
	}
	
	m_EventToPSPApp = new CPSPEventQ("msg_to_pspapp_q");

	
}

int CPSPApp::StartPolling()
{
	if (NULL == m_thRun)
	{
		m_thRun = new CPSPThread("run_thread", runThread, 80, 80000);
		
		/** Start Polling for Vblank and buttons */
		m_thRun->Start();
	}
	m_Polling = true;
	
	return 0;
}

int CPSPApp::StopPolling()
{
	m_Polling = false;
	sceKernelDelayThread(1000*50); /* Wait 50ms */
	return 0;
}

CPSPApp::~CPSPApp()
{
	Log(LOG_VERYLOW, "Destructor Called.");
	
	if (true == IsUSBEnabled())
	{
		Log(LOG_VERYLOW, "Disabling USB.");
		DisableUSB();
	}
	
	if (true == IsNetworkEnabled())
	{
		Log(LOG_VERYLOW, "Disabling Network.");
		DisableNetwork();
	}
	
	if (m_ExitSema)
	{
		Log(LOG_VERYLOW, "deleting exitsema");
		delete m_ExitSema;
	}

	if (m_EventToPSPApp)
	{
		Log(LOG_VERYLOW, "deleting eventtopspapp");
		delete(m_EventToPSPApp), m_EventToPSPApp = NULL;
	}

	Log(LOG_VERYLOW, "freeing program name.");
	free(m_strProgramName);
	Log(LOG_VERYLOW, "freeing version number.");
	free(m_strVersionNumber);
	
	Log(LOG_INFO, "Bye!.");
	
	sceKernelExitGame();

	return;
}

/** This is a thread */
int CPSPApp::Run()
{
	short oldAnalogue = 0;
	int   oldButtonMask = 0;
	SceCtrlLatch latch; 
	u32 hprmlatch = 0;
	
	Log(LOG_INFO, "Run(): Going into main loop.");
	
	sceCtrlSetSamplingCycle(10); 
	while (false == m_Exit)
	{
		sceDisplayWaitVblankStart();
		sceKernelDelayThread(10);
		//sceCtrlReadBufferPositive(&m_pad, 1); 
		
		if (false == m_Polling)
			continue;
		
		OnVBlank();
		//SendEvent(MID_ONVBLANK);
 
		sceCtrlReadLatch(&latch);
		//printf("latch: uiMake=%d; uiBreak=%d; uiPress=%d; uiRelease=%d;\n",
		//	latch.uiMake, latch.uiBreak, latch.uiPress, latch.uiRelease);
		
		if (latch.uiMake)
		{
			/** Button Pressed */
			oldButtonMask = latch.uiPress;
			//OnButtonPressed(oldButtonMask);
			SendEvent(MID_ONBUTTON_PRESSED, &oldButtonMask);
		}
		else if (latch.uiBreak)
		{
			/** Button Released */
			//OnButtonReleased(oldButtonMask);
			SendEvent(MID_ONBUTTON_RELEASED, &oldButtonMask);
		}
		
		if (oldAnalogue != (short)m_pad.Lx)
		{
			//pspDebugScreenSetXY(0,10);
			//printf ("Analog Lx=%03d Ly=%03d     ", m_pad.Lx, m_pad.Ly);
			//pspDebugScreenSetXY(0,5);
			oldAnalogue = (short)m_pad.Lx;
			OnAnalogueStickChange(m_pad.Lx, m_pad.Ly);
		}

		if (sceHprmIsRemoteExist())
		{
			sceHprmReadLatch(&hprmlatch);
			if (hprmlatch != 0x00)
				{
				SendEvent(MID_ONHPRM_RELEASED, &hprmlatch);
				Log(LOG_VERYLOW, "HPRM latch = %04x\n", hprmlatch);
				}
		}
	}
	
	Log(LOG_VERYLOW, "Run:: Wait()");
	m_ExitSema->Wait();
	Log(LOG_VERYLOW, "Run:: Calling OnExit().");
	OnExit();
	SendEvent(MID_PSPAPP_EXITING);

	Log(LOG_VERYLOW, "Run:: Right before calling sceKernelExitThread.");
	sceKernelExitThread(0);
	return 0;
}

int CPSPApp::CallbackSetupThread(SceSize args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", CPSPApp::exitCallback, NULL);
	sceKernelRegisterExitCallback(cbid);
    cbid = sceKernelCreateCallback("Power Callback", CPSPApp::powerCallback, NULL);
    scePowerRegisterCallback(0, cbid);
	sceKernelSleepThreadCB();

	return 0;
}

/** Note: OnAppExit is executed from the exit callback thread, which is running in USER MODE */
int CPSPApp::OnAppExit(int arg1, int arg2, void *common)
{
	m_Exit = true;
	return 0;
}

int CPSPApp::ReportError(char *format, ...)
{
	char *strMessage;
	va_list args;

	strMessage   = (char *)malloc(4096);

	va_start (args, format);         /* Initialize the argument list. */
	
	vsprintf(strMessage, format, args);
	
	va_end (args);
	
	return SendEvent(MID_ERROR, strMessage);
}

/* ---statics--- */
/* System Callbacks */
int CPSPApp::exitCallback(int arg1, int arg2, void *common) 
{
	return pPSPApp->OnAppExit(arg1, arg2, common);
}

int CPSPApp::powerCallback(int arg1, int pwrflags, void *common) 
{
	return pPSPApp->OnPowerEvent(pwrflags);
}

/* Callback thread */
int CPSPApp::callbacksetupThread(SceSize args, void *argp) 
{
	return pPSPApp->CallbackSetupThread(args, argp);
}

int CPSPApp::runThread(SceSize args, void *argp) 
{
	return pPSPApp->Run();
}


