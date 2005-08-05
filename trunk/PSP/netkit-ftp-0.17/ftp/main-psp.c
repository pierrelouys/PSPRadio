#include <stdio.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <string.h>
#include <pspthreadman.h>
#include "Tools.h"
#include "pspnet.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <machine/types.h>
#include <netinet/ip.h>
#include "pg.h"
//#include "_itoa.h"
//#include "log.h"
int g_argc;
char **g_argv;

int main_ftp(int argc, char **argv); //Defined in main.c


asm(".global __lib_stub_top");
asm(".global __lib_stub_bottom");
char g_strBuf[256];

PSP_MODULE_INFO("FTP", 0x1000, 1, 1);
/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(0);

void Kprintf(char *, ...);
#define LogPrintf printf

int sceNetApctlAddHandler(void *handler, void *arg );

int exitCalled=0;
int exitSema;

/* Exit callback */
int exit_callback(int arg1, int arg2, void *arg)
{

	sceKernelSignalSema(exitSema, 1);
	exitCalled=1;

	return 0;
}

/* Callback thread */
//typedef int (*SceKernelThreadEntry)(SceSize args, void *argp);
int CallbackThread(SceSize args, void *argp)
{
	int cbid;
	exitSema=sceKernelCreateSema("SEMA_FTPD_EXIT", 0, 0, 1, 0);
	cbid = sceKernelCreateCallback("Exit Callback", &exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);

	sceKernelSleepThreadCB();
	
	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
	int thid = 0;

//SceUID sceKernelCreateThread(const char *name, SceKernelThreadEntry entry, int initPriority,
//                             int stackSize, SceUInt attr, SceKernelThreadOptParam *option);
	
	thid = sceKernelCreateThread("update_thread", &CallbackThread, 0x11, 0xFA0, THREAD_ATTR_USER, 0);
	if(thid >= 0) {
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}


void ApctlCallback(int old_state, int state, int type, int unknown, void* arg) 
{

	if (state==0) {
		// little pause
		sceKernelDelayThread(500000);
		// idle => connect
		sceNetApctlConnect(0);
	}
	
	
	pgFillvram(0);

	pgPrint(0, 10, 0xffff, g_strBuf);
	LogPrintf("RCG: %s", g_strBuf);
	char *msg = "";
	switch (state) {
		case 0:
		msg = "Idle.";
		break;
		case 1:
		msg = "Initialising...";
		break;
		case 2:
		msg = "Associating...";
		break;
		case 3:
		msg = "DHCP query (do not work).";
		break;
		case 4:
		msg = "Complete.";

		char strText[256];

		char ipaddress[32]; ipaddress[0]=0;
		if (sceNetApctlGetInfo(8, ipaddress) != 0) {
			LogPrintf("ERROR - main.ApctlCallback : Impossible to get IP address of the PSP.\n");
		}
		sprintf(strText, "PSPs IP Address = '%s'", ipaddress);
		pgPrint(0, 3, 0xff, strText);
		break;
	}

	pgPrint(0, 2, 0xffff, msg);

	/*strcpy(mess, "old_state: ");
	_itoa(old_state, itmp, 10);
	strcat(mess, itmp);
	pgPrint(0, 4, 0xffff, itmp);
	strcpy(mess, "state: ");
	_itoa(state, itmp, 10);
	strcat(mess, itmp);
	pgPrint(0, 5, 0xffff, itmp);
	strcpy(mess, "type: ");
	_itoa(type, itmp, 10);
	strcat(mess, itmp);
	pgPrint(0, 6, 0xffff, itmp);
	strcpy(mess, "unknown: ");
	_itoa(unknown, itmp, 10);
	strcat(mess, itmp);
	pgPrint(0, 7, 0xffff, itmp);*/

	pgScreenFlipV();
}


int ftpdLoop(SceSize args, void *argp) 
{
	return main_ftp(g_argc, g_argv);
}

int threadFtpLoop=0;
void WLANConnectionHandler()  {
    u32 err;

    err = nlhInit();
    if (err != 0) {
		LogPrintf("ERROR - main.WLANConnectionHandler : nlhInit returned '%d'.\n", err);
        goto close_net;
    }

	err = sceNetApctlAddHandler(ApctlCallback, NULL);
	if (err != 0) {
		LogPrintf("ERROR - main.WLANConnectionHandler : sceNetApctlAddHandler returned '%d'.\n", err);
        goto close_net;
    }

	err = sceNetApctlConnect(0);
    if (err != 0) {
		LogPrintf("ERROR - main.WLANConnectionHandler : sceNetApctlConnect returned '%d'.\n", err);
        goto close_net;
    }
  
	LogPrintf("INFO  - main.WLANConnectionHandler : Starting server loop thread.\n");
	threadFtpLoop=sceKernelCreateThread("THREAD_FTPD_SERVERLOOP", &ftpdLoop, 0x18, 0x10000, 0, NULL);
	if(threadFtpLoop >= 0) {
		sceKernelStartThread(threadFtpLoop, 0, 0);
	} else {
		LogPrintf("ERROR - main.WLANConnectionHandler : Impossible to create server loop thread. %d\n", err);
	}

	//ftpdLoop();

	LogPrintf("INFO  - main.WLANConnectionHandler : Waiting for exit signal.\n");
	/* waiting for exit */
	sceKernelWaitSema(exitSema, 1, 0);

	pgFillvram(0);
	pgPrint(0, 1, 0xffff, "Disconnecting...");
	pgScreenFlipV();

	err = sceNetApctlDisconnect();
	if (err != 0) {
		LogPrintf("ERROR - main.WLANConnectionHandler : sceNetApctlDisconnect returned '%d'.\n", err);
        goto close_net;
    }

close_net:
    err = nlhTerm();
	if (err != 0) {
		LogPrintf("ERROR - main.WLANConnectionHandler : nlhTerm returned '%d'.\n", err);
    }
}

char g_DirName[256];
int main(int argc, char **argv)
{
	//pspDebugInstallKprintfHandler(NULL);

	//LogOpen("ms0:/ftpd.log");
	g_argc = argc;
	g_argv = argv;
	
	SetupCallbacks();

	pgInit();
	pgScreenFrame(2, 0);
	pgFillvram(0);

	
	sprintf(g_strBuf, "Raf log: dirname='%s'", dirname(argv[0], g_DirName));
	pgPrint(0, 1, 0xffff, "ftpd");
	pgScreenFlipV();

	
	nlhLoadDrivers();
	WLANConnectionHandler();
	

	sceKernelDeleteSema(exitSema);
	//LogClose();

	sceKernelExitGame();
	//sceKernelExitDeleteThread(0);

	return 0;
}
