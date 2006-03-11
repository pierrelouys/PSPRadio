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
#ifndef _PSPTHREAD_
	#define _PSPTHREAD_

	#include <pspkernel.h>
	#include <pspkerneltypes.h>
	
	/** Wrapper class around the kernel system calls for thread management */
	class CPSPThread
	{
		/** These macros can be called from inside the thread function */
		#define Sleep() sceKernelSleepThread()
		#define SleepAndServiceCallbacks() sceKernelSleepThreadCB()

	public:
		CPSPThread(const char *strName, SceKernelThreadEntry ThreadEntry, int initPriority = 0x11,
					int stackSize = 0xFA0, SceUInt attr = PSP_THREAD_ATTR_USER, SceKernelThreadOptParam *option = NULL);
		~CPSPThread();

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
