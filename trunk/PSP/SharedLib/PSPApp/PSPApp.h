#ifndef _PSPAPP_
	#define _PSPAPP_
	/* 
	 PSPApp
	*/
	
	#include <list>
	#include <pspkernel.h>
	#include <pspkerneltypes.h>
	#include <pspnet.h>
	#include <pspdebug.h>
	#include <pspdisplay.h>
	#include <pspctrl.h>
	#include <pspaudio.h>

	
	
	//enum true_or_false
	//{
	//	FALSE = 0,
	//	TRUE  = 1
	//};
		
	/* Define printf, just to make typing easier */
	#define printf	pspDebugScreenPrintf
	
	class CPSPThread;
	class CPSPApp;
	
	extern class CPSPApp *pPSPApp; /** Do not access / Internal Use. */
	
	class CPSPApp
	{
	public:
		CPSPApp();
		virtual ~CPSPApp();
		//virtual int Run() = 0;
		int Run();
		void ExitApp() { m_Exit = TRUE; };
		/** Accessors */
		SceCtrlData GetPadData() { return m_pad; };
		char *GetMyIP() { return m_strMyIP; };
		int GetResolverId() { return m_ResolverId; };
	
	protected:
		/** Helpers */
		int EnableNetwork(int profile);
		void DisableNetwork();
	
		virtual int CallbackSetupThread(SceSize args, void *argp);
		virtual void OnExit(){};
	
		/** Event Handlers */
		virtual void OnButtonPressed(int iButtonMask){};
		virtual void OnButtonReleased(int iButtonMask){};
		virtual void OnVBlank(){};
		virtual void OnAnalogueStickChange(int Lx, int Ly){};
		virtual void OnAudioBufferEmpty(void* buf, unsigned int length){};

		/* System Callbacks */
		static int  exitCallback(int arg1, int arg2, void *common);
		static void audioCallback(void* buf, unsigned int length);
		/* Callback thread */
		static int callbacksetupThread(SceSize args, void *argp);
		
		friend class CPSPSound;
		friend class CPSPSound_MP3;
		BOOLEAN m_Exit;
		
	
	private:
		/** Data */
		CPSPThread *m_thCallbackSetup; /** Callback thread */
		SceCtrlData m_pad; /** Buttons(Pad) data */
		char m_strMyIP[64];
		char m_ResolverBuffer[1024]; /** Could be smaller, no idea */
		int  m_ResolverId;
		
		virtual int OnAppExit(int arg1, int arg2, void *common); /** We call OnExit here */
		int WLANConnectionHandler(int profile);
		int NetApctlHandler();
	};
	
	/** Wrapper class around the kernel system calls for thread management */
	class CPSPThread
	{
	/** These macros can be called from inside the thread function */
	#define ThreadSleep() sceKernelSleepThread()
	#define ThreadSleepAndServiceCallbacks() sceKernelSleepThreadCB()
	
	public:
		CPSPThread(const char *strName, SceKernelThreadEntry ThreadEntry, int initPriority = 0x11,
					int stackSize = 0xFA0, SceUInt attr = 0/*PSP_THREAD_ATTR_USER*/, SceKernelThreadOptParam *option = NULL)
				{ m_thid = sceKernelCreateThread(strName, ThreadEntry, initPriority, stackSize, attr, option);  };
		~CPSPThread()
				{ if (m_thid>=0) sceKernelWaitThreadEnd(m_thid, NULL),sceKernelTerminateDeleteThread(m_thid);   };
				
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
	/** Attribute for threads. 
	enum PspThreadAttributes
	{
		// Enable VFPU access for the thread./
		PSP_THREAD_ATTR_VFPU = 0x00004000,
		// Start the thread in user mode (done automatically 
		//  if the thread creating it is in user mode). /
		PSP_THREAD_ATTR_USER = 0x80000000,
		// Thread is part of the USB/WLAN API./
		PSP_THREAD_ATTR_USBWLAN = 0xa0000000,
		// Thread is part of the VSH API./
		PSP_THREAD_ATTR_VSH = 0xc0000000,
	};
	*/
 
	
#endif
