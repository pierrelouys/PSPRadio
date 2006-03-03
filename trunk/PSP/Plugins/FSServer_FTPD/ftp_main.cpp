#include <stdio.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <string.h>
#include <pspthreadman.h>
#include <Tools.h>
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
#include <iniparser.h>

#define SOCKET int

int ftpdLoop(SceSize args, void *argp) 
{
    u32 err, cbAddrAccept;
	struct sockaddr_in addrListen, addrAccept;
	
	memset(&addrListen, 0, sizeof(struct sockaddr_in));
	memset(&addrAccept, 0, sizeof(struct sockaddr_in));
	
	ModuleLog(LOG_INFO, "INFO  - main.ftpdLoop : Server loop init...\n");

	SOCKET sockListen = socket(AF_INET, SOCK_STREAM, 0);
	if (sockListen & 0x80000000) 
	{
		ModuleLog(LOG_ERROR, "ERROR - main.ftpdLoop : socket returned '%d'.\n", sockListen);
		goto done;
	}

	addrListen.sin_family = AF_INET;
	addrListen.sin_port = htons(21);

	err = bind(sockListen, (struct sockaddr *)&addrListen, sizeof(addrListen));
	if (err != 0) 
	{
		ModuleLog(LOG_ERROR, "ERROR - main.ftpdLoop : socket returned '%d'.\n", err);
        goto done;
    }

	err = listen(sockListen, 1);
	if (err != 0) 
	{
		ModuleLog(LOG_ERROR, "ERROR - main.ftpdLoop : listen returned '%d'.\n", err);
        goto done;
    }

	ModuleLog(LOG_INFO, "INFO  - main.ftpdLoop : Entering server loop.\n");
	while (1) 
	{
		// blocking accept (wait for one connection)
		cbAddrAccept = sizeof(addrAccept);
		SOCKET sockClient = accept(sockListen, (struct sockaddr *)&addrAccept, &cbAddrAccept);
		if (sockClient < 0) 
		{
			goto done;
		}

		ModuleLog(LOG_INFO, "INFO  - main.ftpdLoop : Handling new client connection.\n");

		MftpConnection* con=(MftpConnection*) malloc(sizeof(MftpConnection));
		/*
		if (sceNetApctlGetInfo(8, con->serverIp) != 0) {
			ModuleLog(LOG_INFO, "ERROR - main.ftpdLoop : Impossible to get IP address of the PSP.\n");
            goto done;
		}
		*/
		con->comSocket=sockClient;

		int tmp=sceKernelCreateThread("THREAD_FTPD_CLIENTLOOP", &mftpClientHandler, 0x18, 0x10000, 0, NULL);
		if(tmp >= 0) 
		{
			sceKernelStartThread(tmp, 4, &con);
		} else 
		{
			ModuleLog(LOG_INFO, "ERROR - main.ftpdLoop : Impossible to create client handling thread.\n", err);
		}

	}

done:
	ModuleLog(LOG_INFO, "INFO  - main.ftpdLoop : Quitting server loop.\n");

	err = close(sockListen);
	if (err != 0) 
	{
		ModuleLog(LOG_INFO, "ERROR - main.ftpdLoop : sceNetInetClose returned '%d'.\n", err);
    }
	
	return 0;
}
