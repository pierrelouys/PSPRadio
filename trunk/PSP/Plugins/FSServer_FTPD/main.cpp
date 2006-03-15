/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * main.c - Simple PRX example.
 *
 * Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
 *
 * $Id: main.c 1531 2005-12-07 18:27:12Z tyranid $
 */
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <PSPApp.h>
#include <PSPThread.h>
#include <Tools.h>
#include <iniparser.h>
#include <PSPRadio_Exports.h>
#include <FSS_Exports.h>

int ftpdLoop(SceSize args, void *argp);

PSP_MODULE_INFO("FSS_FTPD", 0, 1, 1);
PSP_HEAP_SIZE_KB(1024);

//Configuration file: 
#define CFG_FILENAME "fss_ftpd/ftpd.cfg"

CIniParser *g_ConfDict = NULL;

volatile int g_blocker = 0;
#define BLOCKER_CREATE_AND_BLOCK(x,name) {x=sceKernelCreateSema(name,0,0,1,0);sceKernelWaitSema(x,1,NULL);}
#define BLOCKER_UNBLOCK_AND_DESTROY(x)   {sceKernelDeleteSema(x);sleep(1);}

extern "C" 
{

int main(int argc, char **argv)
{
	SceSize am = sceKernelTotalFreeMemSize();
	ModuleLog(LOG_INFO, "FTPD: main(): Available memory: %dbytes (%dKB or %dMB)", am, am/1024, am/1024/1024);

	BLOCKER_CREATE_AND_BLOCK(g_blocker, "fss_ftpd_blocker");

	ModuleLog(LOG_INFO, "FTPD: main(): Unblocked; exiting.");
	return 0;
}

int ModuleStartFSS()
{
	//return 0;
	char strCfgFile[256];
	char strDir[256];
	
	sleep(1);
	
	SceSize am = sceKernelTotalFreeMemSize();
	ModuleLog(LOG_INFO, "ModuleStartFSS(): Available memory: %dbytes (%dKB or %dMB)", am, am/1024, am/1024/1024);


	sprintf(strCfgFile, "%s/%s", getcwd(strDir, 255), CFG_FILENAME);
	
	ModuleLog(LOG_INFO, "Opening '%s'\n", strCfgFile);
	
	g_ConfDict = new CIniParser(strCfgFile);
	
	if (g_ConfDict != NULL)
	{
		
		ModuleLog(LOG_INFO, "ftp://%s:%s@%s/", 
				g_ConfDict->GetStr("USER:USER"),
				g_ConfDict->GetStr("USER:PASS"),
				PSPRadioExport_GetMyIP());
	
		CPSPThread *thServerLoop = new CPSPThread("FSS_FTPD_SERVERLOOP_TH", ftpdLoop, 80);
	
		if(thServerLoop != NULL) 
		{
			thServerLoop->Start();
		} 
		else 
		{
			ModuleLog(LOG_ERROR, "main(): Impossible to create server loop thread.\n");
		}
	}
	else
	{
		ModuleLog(LOG_ERROR, "Insufficient memory.");
	}
	
	return 0;
}

void* getModuleInfo(void)
{
	return (void *) &module_info;
}

int module_stop(int args, void *argp)
{
	delete(g_ConfDict), g_ConfDict = NULL;
	
	BLOCKER_UNBLOCK_AND_DESTROY(g_blocker);

	return 0;
}

}

