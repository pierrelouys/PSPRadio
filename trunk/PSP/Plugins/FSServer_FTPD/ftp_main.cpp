#include <stdio.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <string.h>
#include <pspthreadman.h>
#include <PSPApp.h>
#include <pspnet.h>
#include <Logging.h>
#include <PSPRadio_Exports.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <machine/types.h>
#include "ftp.h"
#include <PSPThread.h>
#include <iniparser.h>
#include <Common.h>

#define SOCKET int
volatile MftpConnection *g_con = NULL; 

int ftpdLoop(SceSize args, void *argp) 
{
    u32 err = 0, cbAddrAccept = 0;
	struct sockaddr_in addrListen, addrAccept;
	
	memset(&addrListen, 0, sizeof(struct sockaddr_in));
	memset(&addrAccept, 0, sizeof(struct sockaddr_in));
	
	ModuleLog(LOG_INFO, "ftpdLoop : Server loop init...\n");

	SOCKET sockListen = socket(AF_INET, SOCK_STREAM, 0);//IPPROTO_TCP);///0);
	if (sockListen < 0) 
	{
		ModuleLog(LOG_ERROR, "ftpdLoop : socket returned '%d'.\n", sockListen);
		goto done;
	}

	addrListen.sin_family = AF_INET;
	addrListen.sin_port = htons(21);
	//addrListen.sin_len = sizeof(struct sockaddr_in);
	//addrListen.sin_addr.s_addr = inet_addr(PSPRadioExport_GetMyIP());

	err = bind(sockListen, (sockaddr *)&addrListen, sizeof(addrListen));
	if (err != 0) 
	{
		ModuleLog(LOG_ERROR, "ftpdLoop : socket returned '%d'.\n", err);
        goto done;
    }

	err = listen(sockListen, 1);
	if (err != 0) 
	{
		ModuleLog(LOG_ERROR, "ftpdLoop : listen returned '%d'.\n", err);
        goto done;
    }

	ModuleLog(LOG_INFO, "ftpdLoop : Entering server loop.\n");
	while (1) 
	{
		// blocking accept (wait for one connection)
		cbAddrAccept = sizeof(addrAccept);
		///addrAccept.sin_family = AF_INET;
		///addrAccept.sin_len = sizeof(struct sockaddr_in);
		ModuleLog(LOG_INFO, "ftpdLoop : calling accept.");
		SOCKET sockClient = accept(sockListen, (struct sockaddr *)&addrAccept, &cbAddrAccept);
		ModuleLog(LOG_INFO, "ftpdLoop : accept returns %d.", sockClient);
		if (sockClient < 0) 
		{
			goto done;
		}

		ModuleLog(LOG_INFO, "ftpdLoop : Handling new client connection.\n");

		MftpConnection* con=(MftpConnection*) malloc(sizeof(MftpConnection));
		/*
		if (sceNetApctlGetInfo(8, con->serverIp) != 0) {
			ModuleLog(LOG_INFO, "ERROR - main.ftpdLoop : Impossible to get IP address of the PSP.\n");
            goto done;
		}
		*/
		if (con)
		{
			con->comSocket=sockClient;
	
			mftpClientHandler(4, &con);
			#if 0
			CPSPThread *thClient = new CPSPThread("FSS_FTPD_CLIENT_TH", mftpClientHandler, 0x20);//80);
	
			if(thClient != NULL) 
			{
				//ModuleLog(LOG_INFO, ")
				g_con = con;
				thClient->Start();
			} 
			else 
			{
				ModuleLog(LOG_ERROR, "ftpdLoop : Impossible to create client handling thread. err=0x%x", err);
			}
			#endif
		}
		else
		{
			ModuleLog(LOG_ERROR, "ftpdLoop : Memory alloc error for connection");
		}
	}

done:
	ModuleLog(LOG_INFO, "ftpdLoop : Quitting server loop.\n");

	err = close(sockListen);
	if (err != 0) 
	{
		ModuleLog(LOG_INFO, "ftpdLoop : sceNetInetClose returned '%d'.\n", err);
    }
	
	PSPRadioExport_PluginExits(PLUGIN_FSS); /** Notify PSPRadio, so it can unload the plugin */

	return 0;
}
