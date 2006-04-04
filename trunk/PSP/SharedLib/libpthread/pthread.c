#include <stdio.h>
#include <pspkernel.h>
#include <pspkerneltypes.h>
#include "pthread.h"

typedef struct
{
	thread_function tf;
	void *arguments;
} __pthread_lib_user_thread_and_args;

int _pthread_lib_thread_handler(SceSize args, void *argp);

/* Create a thread */
int pthread_create(pthread_t *thread_id, const pthread_attr_t *attributes, thread_function tf, void *arguments)
{
	__pthread_lib_user_thread_and_args tdata;
	int thid;
	static int ctr = 0;
	char strName[32];
	int  initPriority = 0x20;
	int  stackSize = 0xFA0;
	SceUInt attr = PSP_THREAD_ATTR_USER;
	SceKernelThreadOptParam *option = NULL;

	tdata.tf = tf;
	tdata.arguments = arguments;
	ctr++;

	sprintf(strName, "pthread%d", ctr);

	thid = sceKernelCreateThread(strName, _pthread_lib_thread_handler, initPriority, stackSize, attr, option);  

	if (thid >= 0)
	{
		sceKernelStartThread(thid, sizeof(__pthread_lib_user_thread_and_args), &tdata);
	}

	return 0;
}

/* pthreads terminate when they return, or if they call: */
int pthread_exit(void *status)
{
	sceKernelExitThread(0);
	return 0;
}

///typedef void *(*thread_function)(void *)
	
int _pthread_lib_thread_handler(SceSize args, void *argp)
{	
	__pthread_lib_user_thread_and_args *ptdata = (__pthread_lib_user_thread_and_args *)argp;
	int iRet = 0;
	
	/** Call user thread */
	(ptdata->tf)(ptdata->arguments);

	sceKernelExitThread(0);

	return iRet;
}


#if 0
	
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
