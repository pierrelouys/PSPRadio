#include <stdlib.h>
#include <string.h>
#include <pspthreadman.h>
#include <pspiofilemgr.h>
#include <pspiofilemgr_fcntl.h>
#include <pspiofilemgr_stat.h>
#include <pspiofilemgr_dirent.h>
#include "sutils.h"
#include "pspnet.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "ftp.h"
#include "_itoa.h"
#include "log.h"
#include "iniparser.h"

extern dictionary *g_ConfDict; /** RC: Configuration file dictionary */

int mftpRestrictedCommand(MftpConnection* con, char* command);

void sendResponse(MftpConnection* con, char* s) {
	strcat(con->comBuffer, s);
	if (endsWith(con->comBuffer, "\n")) {
		sceNetInetSend(con->comSocket, con->comBuffer, strlen(con->comBuffer) , 0);
		strcpy(con->comBuffer, "");
	}
}

void sendResponseLn(MftpConnection* con, char* s) {
	strcat(con->comBuffer, s);
	strcat(con->comBuffer, "\r\n");
	sceNetInetSend(con->comSocket, con->comBuffer, strlen(con->comBuffer) , 0);
	strcpy(con->comBuffer, "");
}

void sendData(MftpConnection* con, char* s) {
	strcat(con->dataBuffer, s);
	if (endsWith(con->dataBuffer, "\n")) {
		sceNetInetSend(con->dataSocket, con->dataBuffer, strlen(con->dataBuffer) , 0);
		strcpy(con->dataBuffer, "");
	}
}

void sendDataLn(MftpConnection* con, char* s) {
	strcat(con->dataBuffer, s);
	strcat(con->dataBuffer, "\r\n");
	sceNetInetSend(con->dataSocket, con->dataBuffer, strlen(con->dataBuffer) , 0);
	strcpy(con->dataBuffer, "");
}


unsigned short pasvPort=59735;
int openDataConnectionPASV(MftpConnection* con) {
	con->usePASV=1;

	int err;

	struct sockaddr_in addrPort;
	memset(&addrPort, 0, sizeof(struct sockaddr_in));

	addrPort.sin_len = sizeof(struct sockaddr_in);
	
	addrPort.sin_family = AF_INET;
	addrPort.sin_port = htons(pasvPort);
//	addrPort.sin_addr[0] = 0;
//	addrPort.sin_addr[1] = 0;
//	addrPort.sin_addr[2] = 0;
//	addrPort.sin_addr[3] = 0;

	con->pasvSocket = sceNetInetSocket(AF_INET, SOCK_STREAM, 0);
	if (con->pasvSocket & 0x80000000) return 0;

	err = sceNetInetBind(con->pasvSocket, (struct sockaddr *)&addrPort, sizeof(addrPort));
	if (err) return 0;

	err = sceNetInetListen(con->pasvSocket, 1);
	if (err) return 0;

	pasvPort++;
	return 0;
}


int openDataConnection(MftpConnection* con) {
	int err;

	if (con->usePASV) {
		struct sockaddr_in addrAccept;
		u32 cbAddrAccept;

		cbAddrAccept = sizeof(addrAccept);
		con->dataSocket = sceNetInetAccept(con->pasvSocket, (struct sockaddr *)&addrAccept, &cbAddrAccept);
		if (con->dataSocket & 0x80000000) return 0;
	} else {
		struct sockaddr_in addrPort;
		memset(&addrPort, 0, sizeof(struct sockaddr_in));

		addrPort.sin_len = sizeof(struct sockaddr_in);
		addrPort.sin_family = AF_INET;
		addrPort.sin_port = htons(con->port_port);
		addrPort.sin_addr = con->port_addr;
//		addrPort.sin_addr[0] = con->port_addr[0];
//		addrPort.sin_addr[1] = con->port_addr[1];
//		addrPort.sin_addr[2] = con->port_addr[2];
//		addrPort.sin_addr[3] = con->port_addr[3];

		con->dataSocket = sceNetInetSocket(AF_INET, SOCK_STREAM, 0);
		if (con->dataSocket & 0x80000000) return 0;

		err = sceNetInetConnect(con->dataSocket, (struct sockaddr *)&addrPort, sizeof(struct sockaddr_in));

		if (err) return 0;
	}

	return 1;
}

int closeDataConnection(MftpConnection* con) {
	int err=0;

	err |= sceNetInetClose(con->dataSocket);
	if (con->usePASV) {
		err |= sceNetInetClose(con->pasvSocket);
	}

	if (err) return 0; else return 1;
}


int mftpServerHello(MftpConnection* con) {
	sendResponseLn(con, "220 FTP Server Ready");

	return 0;
}

int mftpCommandPWD(MftpConnection* con, char* command) {
	if (mftpRestrictedCommand(con, command)) {
		sendResponse(con, "257 \"");
		sendResponse(con, con->curDir);
		sendResponseLn(con, "\" is current directory.");
	}

	return 0;
}

int mftpCommandCWD(MftpConnection* con, char* command) {
	if (mftpRestrictedCommand(con, command)) {
		char* newDir=skipWS(&command[3]);
		trimEndingWS(newDir);

		char parsedDir[MAX_PATH_LENGTH+1];

		char* pParsedDir=parsedDir;
		parsedDir[0]=0;


		char* parser=newDir;
		if ((*newDir)=='/') {
			strcpy(con->curDir,"/");
			parser++;
		}

		do {
			if ((*parser)==0 || (*parser)=='/') {
				*pParsedDir=0;
				if (strcmp(parsedDir,".")==0) {

				} else if (strcmp(parsedDir,"..")==0) {
					char* pUp=con->curDir+strlen(con->curDir)-2;
					while ( pUp>=con->curDir && (*pUp)!='/' ) {
						pUp--;
					}
					if ((++pUp)>=con->curDir) {
						*pUp=0;
					}
					if (con->curDir[0]==0) {
						break;
					}

				} else {
					strcat(con->curDir, parsedDir);
					strcat(con->curDir,"/");
				}

				pParsedDir=parsedDir;
			} else {
				(*pParsedDir++)=(*parser);
			}

		} while (*(parser++)!=0);
		
		trimEndingChar(con->curDir, '/');
		strcat(con->curDir, "/");

		sendResponseLn(con, "250 CWD command successful.");
	}

	return 0;
}

int mftpCommandLIST(MftpConnection* con,char* command) {
	if (mftpRestrictedCommand(con, command)) {
		if (openDataConnection(con)==0) {
			sendResponseLn(con, "425 impossible to open data connection.");
		} else {
			sendResponseLn(con, "150 Opening ASCII mode data connection for file list");

			char path[MAX_PATH_LENGTH+1];
			strcpy(path, con->root);
			strcat(path, con->curDir);

			int ret,fd;
			SceIoDirent curFile;

			fd = sceIoDopen(path);
			if (fd>0) {
				do {
					memset(&curFile, 0, sizeof(SceIoDirent));

					ret = sceIoDread(fd, &curFile);
					
					char sInt[16]; strcpy(sInt,"");

					if (ret>0) {
						if (FIO_SO_ISDIR(curFile.d_stat.st_attr)) {
							sendData(con, "drwxrwxrwx   2 root     root     ");
							_itoa((int) curFile.d_stat.st_size, sInt, 10);
							sendData(con, sInt);
							sendData(con, " Jan 01  1970 ");
							sendDataLn(con, curFile.d_name);
						} else if (FIO_SO_ISLNK(curFile.d_stat.st_attr)) {
							sendData(con, "lrwxrwxrwx   1 root     root     ");
							_itoa((int) curFile.d_stat.st_size, sInt, 10);
							sendData(con, sInt);
							sendData(con, " Jan 01  1970 ");
							sendData(con, curFile.d_name);
							sendData(con, " -> ");
							sendDataLn(con, "???");
						} else {
							sendData(con, "-rwxrwxrwx   1 root     root     ");
							_itoa((int) curFile.d_stat.st_size, sInt, 10);
							sendData(con, sInt);
							sendData(con, " Jan 01  1970 ");
							sendDataLn(con, curFile.d_name);
						}
					}
				} while (ret>0);

				sceIoDclose(fd);
			}

			sendResponseLn(con, "226 Transfer complete.");
			closeDataConnection(con);
		}
	}

	return 0;
}

int mftpCommandNLST(MftpConnection* con, char* command) {
	if (mftpRestrictedCommand(con, command)) {
		if (openDataConnection(con)==0) {
			sendResponseLn(con, "425 impossible to open data connection.");
		} else {
			sendResponseLn(con, "150 Opening ASCII mode data connection for file list");

			char path[MAX_PATH_LENGTH+1];
			strcpy(path, con->root);
			strcat(path, con->curDir);

			int ret,fd;
			SceIoDirent curFile;

			fd = sceIoDopen(path);
			if (fd>0) {
				do {
					memset(&curFile, 0, sizeof(SceIoDirent));

					ret = sceIoDread(fd, &curFile);
					
					sendDataLn(con, curFile.d_name);
				} while (ret>0);

				sceIoDclose(fd);
			}

			sendResponseLn(con, "226 Transfer complete.");
			closeDataConnection(con);
		}
	}

	return 0;
}

int mftpCommandRETR(MftpConnection* con, char* command) {
	if (mftpRestrictedCommand(con, command)) {
		if (openDataConnection(con)==0) {
			sendResponseLn(con, "425 impossible to open data connection.");
		} else {
			char* fileName=skipWS(&command[5]);
			trimEndingWS(fileName);

			if (strlen(fileName)>0) {
				sendResponse(con, "150 Opening ASCII mode data connection for ");
				//TODO.txt (1805 bytes).
				sendResponse(con, fileName);
				sendResponseLn(con, ".");

				char filePath[MAX_PATH_LENGTH];
				if (strStartsWith(fileName, "/")) {
					strcpy(filePath, con->root);
					strcat(filePath, fileName);
				} else {
					strcpy(filePath, con->root);
					strcat(filePath, con->curDir);
					strcat(filePath, fileName);
				}

				SceIoStat fileStats;
				sceIoGetstat(filePath, &fileStats);

				if (FIO_SO_ISREG(fileStats.st_attr)) {
					int fdFile = sceIoOpen(filePath, PSP_O_RDONLY, 0777);

					char* buf[TRANSFER_BUFFER_SIZE];
					int c=0;
					while ((c=sceIoRead(fdFile, buf, TRANSFER_BUFFER_SIZE))>0) {
						sceNetInetSend(con->dataSocket, buf, c , 0);
					}

					sceIoClose(fdFile);
					sendResponseLn(con, "226 Transfer complete.");
				} else {
					sendResponse(con, "550 ");
					sendResponse(con, fileName);
					sendResponseLn(con, ": not a regular file.");
				}
			} else {
				sendResponseLn(con, "500 'RETR': command requires a parameter.");
			}

			
			closeDataConnection(con);
		}
	}

	return 0;
}

int mftpCommandSTOR(MftpConnection* con, char* command) {
	if (mftpRestrictedCommand(con, command)) {
		if (openDataConnection(con)==0) {
			sendResponseLn(con, "425 impossible to open data connection.");
		} else {			
			char* fileName=skipWS(&command[5]);
			trimEndingWS(fileName);

			if (strlen(fileName)>0) {
				sendResponse(con, "150 Opening ASCII mode data connection for ");
				//TODO.txt (1805 bytes).
				sendResponse(con, fileName);
				sendResponseLn(con, ".");
				
				char filePath[MAX_PATH_LENGTH];
				if (strStartsWith(fileName, "/")) {
					strcpy(filePath, con->root);
					strcat(filePath, fileName);
				} else {
					strcpy(filePath, con->root);
					strcat(filePath, con->curDir);
					strcat(filePath, fileName);
				}

				int fdFile = sceIoOpen(filePath, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

				u8* buf[TRANSFER_BUFFER_SIZE];
				int c=0;
				while ((c=sceNetInetRecv(con->dataSocket, (u8*)buf, TRANSFER_BUFFER_SIZE, 0))>0) {
					 sceIoWrite(fdFile, buf, c); 
				}

				sceIoClose(fdFile);
				closeDataConnection(con);
				sendResponseLn(con, "226 Transfer complete.");
			} else {
				sendResponseLn(con, "500 'STOR': command requires a parameter.");
				closeDataConnection(con);
			}

		}
	}

	return 0;
}

int mftpCommandSIZE(MftpConnection* con, char* command) {
	if (mftpRestrictedCommand(con, command)) {
		char* fileName=skipWS(&command[5]);
		trimEndingWS(fileName);

		if (strlen(fileName)>0) {
			char filePath[MAX_PATH_LENGTH];
			if (strStartsWith(fileName, "/")) {
				strcpy(filePath, con->root);
				strcat(filePath, fileName);
			} else {
				strcpy(filePath, con->root);
				strcat(filePath, con->curDir);
				strcat(filePath, fileName);
			}

			SceIoStat fileStats;
			sceIoGetstat(filePath, &fileStats);

			if (FIO_SO_ISREG(fileStats.st_attr)) {
				char tmp[32];
				_itoa(fileStats.st_size, tmp, 10);
				sendResponse(con, "213 ");
				sendResponseLn(con, tmp);
			} else {
				sendResponse(con, "550 ");
				sendResponse(con, fileName);
				sendResponseLn(con, ": not a regular file.");
			}
			
		} else {
			sendResponseLn(con, "500 'SIZE': command requires a parameter.");
		}
	}

	return 0;
}

int ConvertPathToPSP(MftpConnection* con, char *strDest, char *strSrc)
{
	if (strStartsWith(strSrc, "/")) {
		strcpy(strDest, con->root);
		strcat(strDest, strSrc);
	} else {
		strcpy(strDest, con->root);
		strcat(strDest, con->curDir);
		strcat(strDest, strSrc);
	}

	return 0;
}


int mftpCommandRENAME(MftpConnection* con, char* strSource, char *strDest) 
{
	int iRet = 0;
	char strRealSourcePath[MAX_PATH_LENGTH];
	char strRealDestPath[MAX_PATH_LENGTH];
	
	/** Are source and Dest populated? */
	if (strlen(strSource) > 0 && strlen(strDest) > 0)
	{
		ConvertPathToPSP(con, strRealSourcePath, strSource);
		ConvertPathToPSP(con, strRealDestPath, strDest);
		
		sceIoRename(strRealSourcePath, strRealDestPath);

		sendResponseLn(con, "250 RNTO command successful.");
	} 
	else 
	{
		sendResponseLn(con, "500 'RENAME': command requires two parameters.");
	}
	
	return iRet;
}
int mftpCommandDELE(MftpConnection* con, char* command) {
	if (mftpRestrictedCommand(con, command)) {
		char* fileName=skipWS(&command[5]);
		trimEndingWS(fileName);

		if (strlen(fileName)>0) {

			char filePath[MAX_PATH_LENGTH];
			if (strStartsWith(fileName, "/")) {
				strcpy(filePath, con->root);
				strcat(filePath, fileName);
			} else {
				strcpy(filePath, con->root);
				strcat(filePath, con->curDir);
				strcat(filePath, fileName);
			}

			sceIoRemove(filePath);

			sendResponseLn(con, "250 DELE command successful.");
		} else {
			sendResponseLn(con, "500 'DELE': command requires a parameter.");
		}
	}

	return 0;
}

int mftpCommandRMD(MftpConnection* con, char* command) {
	if (mftpRestrictedCommand(con, command)) {
		char* fileName=skipWS(&command[4]);
		trimEndingWS(fileName);

		if (strlen(fileName)>0) {

			char filePath[MAX_PATH_LENGTH];
			if (strStartsWith(fileName, "/")) {
				strcpy(filePath, con->root);
				strcat(filePath, fileName);
			} else {
				strcpy(filePath, con->root);
				strcat(filePath, con->curDir);
				strcat(filePath, fileName);
			}

			trimEndingChar(filePath, '/');
			sceIoRmdir(filePath);

			sendResponseLn(con, "250 RMD command successful.");
		} else {
			sendResponseLn(con, "500 'RMD': command requires a parameter.");
		}
	}

	return 0;
}

int mftpCommandMKD(MftpConnection* con, char* command) {
	if (mftpRestrictedCommand(con, command)) {
		char* fileName=skipWS(&command[4]);
		trimEndingWS(fileName);

		if (strlen(fileName)>0) {

			char filePath[MAX_PATH_LENGTH];
			if (strStartsWith(fileName, "/")) {
				strcpy(filePath, con->root);
				strcat(filePath, fileName);
			} else {
				strcpy(filePath, con->root);
				strcat(filePath, con->curDir);
				strcat(filePath, fileName);
			}

			trimEndingChar(filePath, '/');
			
			sceIoMkdir(filePath, 0);

			sendResponse(con, "257 \"");
			sendResponse(con, fileName);
			sendResponseLn(con, "\" - Directory successfully created.");
		} else {
			sendResponseLn(con, "500 'MKD': command requires a parameter.");
		}
	}

	return 0;
}

int mftpCommandHELP(MftpConnection* con, char* command) {
	sendResponseLn(con, "214-The following commands are recognized (* =>'s unimplemented).");
	sendResponseLn(con, "214-USER    PASS    ACCT*   CWD     XCWD*    CDUP    XCUP*    SMNT*");
	sendResponseLn(con, "214-QUIT    REIN*   PORT    PASV    TYPE    STRU*    MODE*    RETR");
	sendResponseLn(con, "214-STOR    STOU*   APPE*    ALLO*   REST*    RNFR*    RNTO*    ABOR*");
	sendResponseLn(con, "214-DELE    MDTM*    RMD     XRMD*    MKD     XMKD*    PWD     XPWD*");
	sendResponseLn(con, "214-SIZE    LIST    NLST    SITE    SYST    STAT*    HELP    NOOP");
	sendResponseLn(con, "214 Direct comments about original version to psp@amoks.com.");
	sendResponseLn(con, "214 Direct comments about raf versions to http://rafpsp.blogspot.com/");

	return 0;
}

int mftpCommandSITE(MftpConnection* con, char * command) {
	char* param=skipWS(&command[5]);
	trimEndingWS(param);
	toUpperCase(param);
	if (strcmp(param, "HELP")==0) {
		sendResponseLn(con, "214-The following SITE commands are recognized (* =>'s unimplemented).");
		sendResponseLn(con, "214-HELP");
		sendResponseLn(con, "214 Direct comments about original version to psp@amoks.com.");
		sendResponseLn(con, "214 Direct comments about raf versions to http://rafpsp.blogspot.com/");
	} else if (strlen(param)==0) {
		sendResponseLn(con, "500 'SITE' requires argument.");
	} else {
		sendResponse(con, "500 '");
		sendResponse(con, command);
		sendResponseLn(con, "' not understood.");
	}

	return 0;
}

int mftpCommandPORT(MftpConnection* con, char* command) {
	int params[6];
	char decimByte[4];
	char* pDecimByte=decimByte;
	char* pParams=skipWS(&command[5]);

	int state=0; int err=0; int nbParams=0;
	do {
		if (state==0 && *pParams>='0' && *pParams<='9') {
			state=1;
			pParams--;
		} else if (state==1 && *pParams>='0' && *pParams<='9') {
			if (pDecimByte-decimByte<=2) {
				*(pDecimByte++)=*pParams;
			} else {
				err=1;
			}
		} else if (state==1 && (*pParams==',' || *pParams==0) && nbParams<6) {
			*pDecimByte=0;

			if (strlen(decimByte)==0) {
				err=1;
			} else {
				int param=0;
				char* tmp=decimByte+strlen(decimByte)-1;
				int pow=1;
				while (tmp>=decimByte && err==0) {
					
					if (*tmp>='0' && *tmp<='9') {
						param+= ((*tmp)-48)*pow;
						pow=pow*10;
					} else {
						err=1;
					}

					tmp--;
				}

				if (err==0) {
					params[nbParams++]=param;
					pDecimByte=decimByte;
				}
			}
		} else {
			err=1;
		}

	} while (*(pParams++)!=0 && err==0);

	if (err==0 && state==1 && nbParams==6) {
		con->usePASV=0;

		con->port_addr_c[0]=(unsigned char) params[0];
		con->port_addr_c[1]=(unsigned char) params[1];
		con->port_addr_c[2]=(unsigned char) params[2];
		con->port_addr_c[3]=(unsigned char) params[3];
		con->port_port=((unsigned char) params[4]<<8) | ((unsigned char) params[5]);
		sendResponseLn(con, "200 PORT command successful.");
	} else {
		con->port_addr.s_addr=0;
		//con->port_addr[1]=0;
		//con->port_addr[2]=0;
		//con->port_addr[3]=0;
		con->port_port=0;
		sendResponseLn(con, "500 illegal PORT command.");
	}

	return 0;
}

int mftpCommandUSER(MftpConnection* con, char* command) {
	if (con->userLoggedIn) {
		sendResponseLn(con, "503 You are already logged in!");
	} else {
		con->user[0]=0;
		con->pass[0]=0;
		char* pUser=skipWS(&command[5]);
		trimEndingWS(pUser);
		if (strlen(pUser)==0) {
			sendResponseLn(con, "500 'USER': command requires a parameter.");
		} else {
			strncpy(con->user, pUser, MAX_USER_LENGTH);
			sendResponse(con, "331 Password required for ");
			sendResponse(con, con->user);
			sendResponseLn(con, ".");

		}
	}

	return 0;
}

int mftpCommandPASS(MftpConnection* con, char* command) {
	if (con->userLoggedIn) {
		sendResponseLn(con, "503 You are already logged in!");
	} else {
		if (strlen(con->user)==0) {
			sendResponseLn(con, "503 Login with USER first.");
		} else {
			con->pass[0]=0;
			char* pPass=skipWS(&command[5]);
			trimEndingWS(pPass);
			if (strlen(pPass)==0) {
				sendResponseLn(con, "500 'PASS': command requires a parameter.");
			} else {
				strncpy(con->pass, pPass, MAX_PASS_LENGTH);

				if (strcmp(con->user, iniparser_getstr(g_ConfDict, "USER:USER"))==0 
					&& strcmp(con->pass, iniparser_getstr(g_ConfDict, "USER:PASS"))==0) {
					sendResponseLn(con, "230 You're logged in.");
					con->userLoggedIn=1;
				} else {
					con->user[0]=0; con->pass[0]=0;
					con->userLoggedIn=0;
					sendResponseLn(con, "530 You're not allowed to log in.");
				}
			}
		}
	}

	return 0;
}

int mftpCommandTYPE(MftpConnection* con, char* command) {
	if (mftpRestrictedCommand(con, command)) {
		char* pParam1=skipWS(&command[5]);
		trimEndingWS(pParam1);
		if (strlen(pParam1)==0) {
			sendResponseLn(con, "500 'TYPE': command requires a parameter.");
		} else if (strlen(pParam1)==1 && (*pParam1=='A' || *pParam1=='E' || *pParam1=='I' || *pParam1=='L')) {
			con->transferType=*pParam1;
			sendResponse(con, "200 Type set to ");
			sendResponse(con, pParam1);
			sendResponseLn(con, ".");
		} else {
			sendResponseLn(con, "500 'TYPE': 2 parameters -- extended version not understood.");
		}
	}

	return 0;
}

int mftpCommandSYST(MftpConnection* con, char* command) {
	if (mftpRestrictedCommand(con, command)) {
		sendResponseLn(con, "215 UNIX Type: L8");
	}

	return 0;
}

int mftpCommandPASV(MftpConnection* con, char* command) {
	if (mftpRestrictedCommand(con, command)) {
		
		char tmp[32];
		strncpy(tmp, con->serverIp, 31);
		strReplaceChar(tmp, '.', ',');

		sendResponse(con, "227 Entering Passive Mode (");
		sendResponse(con, tmp);
		sendResponse(con, ",");
		_itoa((pasvPort>>8) & 0xFF, tmp, 10);
		sendResponse(con, tmp);
		sendResponse(con, ",");
		_itoa(pasvPort & 0xFF, tmp, 10);
		sendResponse(con, tmp);
		sendResponseLn(con, ").");

		openDataConnectionPASV(con);
	}

	return 0;
}

int mftpCommandNOOP(MftpConnection* con, char* command) {
	if (mftpRestrictedCommand(con, command)) {
		sendResponseLn(con, "200 NOOP command successful.");
	}

	return 0;
}

int mftpCommandQUIT(MftpConnection* con, char* command) {
	closeDataConnection(con);

	return -1;
}

int mftpRestrictedCommand(MftpConnection* con, char* command) {
	if (!con->userLoggedIn) {
		sendResponseLn(con, "530 Please login with USER and PASS.");

		return 0;
	} else {
		return 1;
	}
}

int mftpDispatch(MftpConnection* con, char* command) {

	char uCommand[MAX_COMMAND_LENGTH+1];
	static char strRenameFrom[MAX_COMMAND_LENGTH+1];
	strncpy(uCommand, command, MAX_COMMAND_LENGTH);
	toUpperCase(uCommand);

	int ret=0;
	if (strlen(uCommand)>0) {
		if (strcmp(uCommand, "PWD")==0) {
			ret=mftpCommandPWD(con, command);
		} else if (strcmp(uCommand, "NLST")==0) {
			ret=mftpCommandNLST(con, command);
		} else if (strcmp(uCommand, "LIST")==0 || strStartsWith(uCommand, "LIST ")) {
			ret=mftpCommandLIST(con, command);
		} else if (strStartsWith(uCommand, "RETR ")) {
			ret=mftpCommandRETR(con, command);
		} else if (strStartsWith(uCommand, "STOR ")) {
			ret=mftpCommandSTOR(con, command);
		} else if (strStartsWith(uCommand, "SIZE ")) {
			ret=mftpCommandSIZE(con, command);
		} else if (strStartsWith(uCommand, "DELE ")) {
			ret=mftpCommandDELE(con, command);
		} else if (strStartsWith(uCommand, "RNFR ")) {
			strcpy(strRenameFrom, command+5);
			sendResponseLn(con, "350 RNFR command successful.");
		} else if (strStartsWith(uCommand, "RNTO ")) {
			ret=mftpCommandRENAME(con, strRenameFrom, command+5);
		} else if (strStartsWith(uCommand, "RMD ")) {
			ret=mftpCommandRMD(con, command);
		} else if (strStartsWith(uCommand, "MKD ")) {
			ret=mftpCommandMKD(con, command);
		} else if (strcmp(uCommand, "CDUP")==0) {
			ret=mftpCommandCWD(con, "CWD ..");
		} else if (strStartsWith(uCommand, "CWD ")) {
			ret=mftpCommandCWD(con, command);
		} else if (strcmp(uCommand, "HELP")==0 || strStartsWith(uCommand, "HELP ")) {
			ret=mftpCommandHELP(con, command);
		} else if (strcmp(uCommand, "SITE")==0 || strStartsWith(uCommand, "SITE ")) {
			ret=mftpCommandSITE(con, command);
		} else if (strStartsWith(uCommand, "PORT ")) {
			ret=mftpCommandPORT(con, command);
		} else if (strStartsWith(uCommand, "USER ")) {
			ret=mftpCommandUSER(con, command);
		} else if (strStartsWith(uCommand, "PASS ")) {
			ret=mftpCommandPASS(con, command);
		} else if (strStartsWith(uCommand, "TYPE ")) {
			ret=mftpCommandTYPE(con, command);
		} else if (strcmp(uCommand, "SYST")==0) {
			ret=mftpCommandSYST(con, command);
		} else if (strcmp(uCommand, "PASV")==0) {
			ret=mftpCommandPASV(con, command);
		} else if (strcmp(uCommand, "NOOP")==0) {
			ret=mftpCommandNOOP(con, command);
		} else if (strcmp(uCommand, "QUIT")==0) {
			ret=mftpCommandQUIT(con, command);
		} else {
			sendResponse(con, "500 ");
			sendResponse(con, command);
			sendResponseLn(con, " not understood.");
		}
	}

	return ret;
}

//typedef int (*SceKernelThreadEntry)(SceSize args, void *argp);
int mftpClientHandler(SceSize args, void *argp) {
	MftpConnection *con = *(MftpConnection **)argp;

	con->dataSocket=0;
	con->pasvSocket=0;
	memset(con->comBuffer, 0, 1024);
	memset(con->dataBuffer, 0, 1024);
	strcpy(con->root,"ms0:");
	strcpy(con->curDir,"/");
	con->transferType='A';
	memset(con->user, 0, MAX_USER_LENGTH);
	memset(con->pass, 0, MAX_PASS_LENGTH);
	con->usePASV=0;
	con->userLoggedIn=0;
	con->port_port=0;
	con->port_addr.s_addr = 0;

	int err;

	mftpServerHello(con);

	char readBuffer[1024];
	char lineBuffer[1024];
	int lineLen=0;
	int errLoop=0;
	while (errLoop>=0)
	{
		int nb = sceNetInetRecv(con->comSocket, (u8*)readBuffer, 1024, 0);

		if (nb <= 0) break;

		int i=0; 
		while (i<nb) {
			if (readBuffer[i]!='\r') {
				lineBuffer[lineLen++]=readBuffer[i];
				if (readBuffer[i]=='\n' || lineLen==1024) {
					lineBuffer[--lineLen]=0;

					char* command=skipWS(lineBuffer);
					trimEndingWS(command);
					if ((errLoop=mftpDispatch(con, command))<0) break;

					lineLen=0;
				}
			}
			i++;
		}
	}

	err = sceNetInetClose(con->comSocket);

	free(con);

	LogPrintf("INFO  - ftp.mftpClientHandler : Connection closed.\n");

	sceKernelExitDeleteThread(0);
	
	return 0;
}
