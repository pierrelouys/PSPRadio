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
//PSP_NO_CREATE_MAIN_THREAD();
PSP_HEAP_SIZE_KB(2048);

//Configuration file: 
#define CFG_FILENAME "fss_ftpd/ftpd.cfg"

CIniParser *g_ConfDict = NULL;

volatile bool g_ExitModule = false;

extern "C" 
{

int main(int argc, char **argv)
{
	SceSize am = sceKernelTotalFreeMemSize();
	ModuleLog(LOG_INFO, "FTPD: main(): Available memory: %dbytes (%dKB or %dMB)", am, am/1024, am/1024/1024);
	while (g_ExitModule == false)
	{
		sleep(1);
	}
	ModuleLog(LOG_INFO, "FTPD: main(): Unblocked; exiting.");
	return 0;
}

int ModuleStartFSS()
{
	//return 0;
	char strCfgFile[256];
	char strDir[256];
	
	//while(PSPRadioExport_IsFSSMainBlocked() == false){sleep(1);};
	//sleep(1);

	ModuleLog(LOG_INFO, "g_ExitModule=%d", g_ExitModule);
	char *a = (char*)malloc(20);
	sprintf(a, "modified 1234567890");
		//  12345678901234567890
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
	
// 		int threadFtpLoop=sceKernelCreateThread("THREAD_FTPD_SERVERLOOP", ftpdLoop, 0x18, 0x10000, 0, NULL);
// 		if(threadFtpLoop >= 0) {
// 			sceKernelStartThread(threadFtpLoop, 0, 0);
// 		}

//		return 0;	
		//char *a = new char[10];
		//sprintf(a, "abcdefghi");
// 		SceSize am = sceKernelTotalFreeMemSize();
// 		ModuleLog(LOG_INFO, "Available memory before malloc(10): %dbytes (%dKB or %dMB)", am, am/1024, am/1024/1024);
// 		char *a = (char*)malloc(10);
// 		sprintf(a, "abcdefghi");
// 		am = sceKernelTotalFreeMemSize();
// 		ModuleLog(LOG_INFO, "Available memory after malloc(10): %dbytes (%dKB or %dMB)", am, am/1024, am/1024/1024);
// 		return 0;

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
	//PSPRadioExport_FSSUnBlockMain();
	
	g_ExitModule = true;

	return 0;
}

}

