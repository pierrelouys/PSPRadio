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
	int  stackSize = 8*1024; /* 8Kb stack */
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
	sceKernelExitDeleteThread(0);
	return 0;
}

int _pthread_lib_thread_handler(SceSize args, void *argp)
{	
	__pthread_lib_user_thread_and_args *ptdata = (__pthread_lib_user_thread_and_args *)argp;
	int iRet = 0;
	
	/** Call user thread */
	(ptdata->tf)(ptdata->arguments);

	pthread_exit(NULL);

	return iRet;
}
