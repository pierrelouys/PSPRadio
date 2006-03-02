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
#include <Tools.h>
#include <iniparser.h>
#include <PSPRadio_Exports.h>

PSP_MODULE_INFO("FSS_FTPD", 0, 1, 1);
PSP_NO_CREATE_MAIN_THREAD();
PSP_HEAP_SIZE_KB(2048);

//Configuration file: 
#define CFG_FILENAME "ftpd.cfg"

int exitCalled=0;
int exitSema;
CIniParser *g_ConfDict = NULL;

extern "C" 
{
	int module_stop(int args, void *argp);
}

int ftpdLoop(SceSize args, void *argp);

int main(int argc, char **argv)
{
	char strCfgFile[256];
	char strDir[256];
	
	printf ("argv[0]='%s' \n", argv[0]);
	
	strcpy(strDir, argv[0]);
	dirname(strDir); /** Retrieve the directory name */
	
	sprintf(strCfgFile, "%s/%s", strDir, CFG_FILENAME);
	
	printf ("Opening '%s'\n", strCfgFile);
	
	g_ConfDict = new CIniParser(strCfgFile);
	
	ModuleLog(LOG_INFO, "username='%s' password='%s'\n", 
		g_ConfDict->GetStr("USER:USER"),
		g_ConfDict->GetStr("USER:PASS"));
	
	///
	exitSema=sceKernelCreateSema("SEMA_FTPD_EXIT", 0, 0, 1, 0);
		char url[256];
		sprintf(url, "ftp://%s:%s@%s/", 
			g_ConfDict->GetStr("USER:USER"),
			g_ConfDict->GetStr("USER:PASS"),
			PSPRadioExport_GetMyIP()
			);

	///
int threadFtpLoop=0;
	threadFtpLoop=sceKernelCreateThread("THREAD_FTPD_SERVERLOOP", &ftpdLoop, 0x18, 0x10000, 0, NULL);
	if(threadFtpLoop >= 0) 
	{
		sceKernelStartThread(threadFtpLoop, 0, 0);
	} else 
	{
		ModuleLog(LOG_ERROR, "ERROR - main() : Impossible to create server loop thread.\n", threadFtpLoop);
	}
///
	ModuleLog(LOG_INFO, "INFO  - main() : Waiting for exit signal.\n");
	/* waiting for exit */
	sceKernelWaitSema(exitSema, 1, 0);
//
	delete(g_ConfDict), g_ConfDict = NULL;
	
	
	return 0;
}

void* getModuleInfo(void)
{
	return (void *) &module_info;
}

int module_stop(int args, void *argp)
{
	exitCalled=1;
	return 0;
}
