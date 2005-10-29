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
#ifndef _PSPAPP_
	#define _PSPAPP_
	
	#include <list>
	#include <pspkernel.h>
	#include <pspkerneltypes.h>
	#include <psppower.h>
	#include <PSPNet.h>
	#include <pspctrl.h>
	#include <pspaudio.h>
	#include <Logging.h>
	#include "PSPEventQ.h"

	/** Sender IDs */
	#define SID_PSPAPP			0x10000000
	#define SID_PSPSOUND		0x11000000
	
	/** Message IDs */
	#define MID_ERROR						0x00000000
	#define MID_THPLAY_BEGIN				0x00000010
	#define MID_THPLAY_END					0x00000020
	#define MID_THPLAY_DONE					0x00000031
	#define MID_THDECODE_DECODING			0x00000040
	#define MID_THDECODE_BEGIN				0x00000051
	#define MID_THDECODE_END				0x00000052
	#define MID_DECODE_STREAM_OPENING		0x00000060
	#define MID_DECODE_STREAM_OPEN_ERROR	0x00000070
	#define MID_DECODE_STREAM_OPEN			0x00000080
	#define MID_NEW_METADATA_AVAILABLE		0x000000A0
	#define MID_THDECODE_DECODING_DONE		0x000000B1
	#define MID_BUFF_PERCENT_UPDATE			0x00000090
	#define MID_TCP_CONNECTING_PROGRESS		0x000000C0
	#define MID_TCP_CONNECTING_FAILED		0x000000C1
	#define MID_TCP_CONNECTING_SUCCESS		0x000000C2
	#define MID_PSPAPP_EXITING				0x01000000
	#define MID_ONBUTTON_PRESSED			0x01000010
	#define MID_ONBUTTON_RELEASED			0x01000011
	#define MID_ONHPRM_RELEASED				0x01000012
	#define MID_ONVBLANK					0x01000020
	
	class CPSPThread;
	class CPSPApp;
	
	extern class CPSPApp *pPSPApp; /** Do not access / Internal Use. */
	
	class CPSPApp
	{
	public:
		CPSPApp(char *strProgramName, char *strVersionNumber);
		virtual ~CPSPApp();
		
		virtual int ProcessMessages(){return 0;};
		
		//void ExitApp() { m_Exit = true; };
		
		/** Accessors */
		bool IsExiting() { return m_Exit; };
		SceCtrlData GetPadData() { return m_pad; };
		char *GetProgramName() { return m_strProgramName; };
		char *GetProgramVersion() { return m_strVersionNumber; };
		
		/** Messaging */
		int SendEvent(int iEventId, void *pData = NULL, int iSenderId = SID_PSPAPP)
		{ 
			CPSPEventQ::QEvent event = { iSenderId, iEventId, pData };
			return m_EventToPSPApp?m_EventToPSPApp->Send(event):-1;
		};
		int ReportError(char *format, ...);
		
		/** Control the Run() Thread. In charge of polling vblank and buttons */
		int StartPolling(); /** Start polling buttons/vblank */
		int StopPolling();
		bool IsPolling(){ return m_Polling;}
		
		//void CantExit() { m_ExitSema->Up(); }
		//void CanExit() { m_ExitSema->Down(); }
		
		/** Networking */
		char *GetMyIP() { return m_strMyIP; };
		int  GetResolverId() { return m_ResolverId; };
		int  EnableNetwork(int profile);
		void DisableNetwork();
		bool IsNetworkEnabled() { return m_NetworkEnabled; };
		int  GetNumberOfNetworkProfiles();
		
		/** USB */
		int  EnableUSB();
		int  DisableUSB();
		bool IsUSBEnabled() { return m_USBEnabled; }
		
	protected:
		/** Helpers */
	
		int CallbackSetupThread(SceSize args, void *argp);
		//virtual void OnExit(){};
		
		/** Threads */
		int Run(); /** Thread */
	
		/** Event Handlers */
		virtual void OnVBlank(){};
		virtual void OnAnalogueStickChange(int Lx, int Ly){};

		/* System Callbacks */
		static int exitCallback(int arg1, int arg2, void *common);
		static int powerCallback(int arg1, int arg2, void *common);
		/* Callback thread */
		static int callbacksetupThread(SceSize args, void *argp);
		static int runThread(SceSize args, void *argp);
		
		/** Data */
		CPSPEventQ *m_EventToPSPApp;
		
	private:
		/** Data */
		CLogging m_Log;
		bool m_Exit;
		bool m_NetworkEnabled;
		bool m_USBEnabled;
		//CSema *m_ExitSema;
		CPSPThread *m_thCallbackSetup; /** Callback thread */
		CPSPThread *m_thRun; /** Run Thread */
		SceCtrlData m_pad; /** Buttons(Pad) data */
		char m_strMyIP[64];
		char m_ResolverBuffer[1024]; /** Could be smaller, no idea */
		int  m_ResolverId;
		char *m_strProgramName, *m_strVersionNumber;
		bool m_Polling;
		
		virtual int OnAppExit(int arg1, int arg2, void *common);
		virtual int OnPowerEvent(int pwrflags){return 0;};

		/** Networking */
		int WLANConnectionHandler(int profile);
		int NetApctlHandler();
		
	friend class CPSPSound;
	friend class CPSPSoundBuffer;
	friend class CPSPSoundStream;

	};
	
	#define ReportError pPSPApp->ReportError	
	
	/** Wrapper class around the kernel system calls for thread management */
	class CPSPThread
	{
		/** These macros can be called from inside the thread function */
		#define Sleep() sceKernelSleepThread()
		#define SleepAndServiceCallbacks() sceKernelSleepThreadCB()
	
	public:
		CPSPThread(const char *strName, SceKernelThreadEntry ThreadEntry, int initPriority = 0x11,
					int stackSize = 0xFA0, SceUInt attr = PSP_THREAD_ATTR_USER, SceKernelThreadOptParam *option = NULL)
				{ m_thid = sceKernelCreateThread(strName, ThreadEntry, initPriority, stackSize, attr, option);  };
		~CPSPThread()
				{ /*if (m_thid>=0) sceKernelWaitThreadEnd(m_thid, NULL),sceKernelTerminateDeleteThread(m_thid); */  };
				
		int Start()
				{ return m_thid>=0?sceKernelStartThread(m_thid, 0, NULL):-1; };
		int Suspend()
				{ return m_thid>=0?sceKernelSuspendThread(m_thid):-1; };
		int Resume()
				{ return m_thid>=0?sceKernelResumeThread(m_thid):-1; };
		int WakeUp() /** Wakeup a thread that put itself to sleep with ThreadSleep() */
				{ return m_thid>=0?sceKernelWakeupThread(m_thid):-1; };
		int Wait(SceUInt *timeoutInUs) /** Wait until thread exits or timeout */
				{ return m_thid>=0?sceKernelWaitThreadEnd(m_thid, timeoutInUs):-1; };
		int WaitAndServiceCallbacks(SceUInt *timeoutInUs) /** Wait until thread exits(servicing callbacks) or timeout */
				{ return m_thid>=0?sceKernelWaitThreadEndCB(m_thid, timeoutInUs):-1; };
		int SetPriority(int iNewPriority)
				{ return m_thid>=0?sceKernelChangeThreadPriority(m_thid, iNewPriority):-1; };    

	private:
		int m_thid;
	};

#endif
