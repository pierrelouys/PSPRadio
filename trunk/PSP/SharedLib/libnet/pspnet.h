#ifndef __PSPNET_H
#define __PSPNET_H
	
	#ifndef __PSP__
		#define __PSP__
	#endif

	#include <sys/types.h>
	#include <psputility.h>
	#include <reent.h>
	#include <w3c-libwww/wwwsys.h> /** RC */
	
	// Net Lib Helper - work in progress
	
	// major functions - Access Point / NetInet
	int nlhLoadDrivers();
	int nlhInit();
	int nlhTerm();
	
	int sceNetInit(u32 r4, u32 r5, u32 r6, u32 r7, u32 r8);
	int sceNetTerm();
	
	int sceNetGetLocalEtherAddr(u8* addr);
	
	int sceNetInetInit();
	int sceNetInetTerm();
	
	int sceNetResolverInit();
	int sceNetResolverTerm();
	
	int sceNetAdhocInit(); 
	int sceNetAdhocctlInit(u32 r4, u32 r5, void* r6);
	int sceNetAdhocTerm();
	int sceNetAdhocctlConnect(const char*);
	
	int sceNetApctlConnect(int profile);
	int sceNetApctlInit(u32 r4, u32 r5);
	int sceNetApctlTerm();
	int sceNetApctlGetState(u32* stateOut);
	int sceNetApctlGetInfo(u32 r4, void* r5);
	int sceNetApctlDisconnect();
	
	
	#define SOCKET int
	
	int sceNetInetClose(SOCKET s);
	
	/** RC 09-11-2005: These defined from http://www.netrino.com/Publications/Glossary/Endianness.html */
	#define htons(A)  ((((u16)(A) & 0xff00) >> 8) | \
					   (((u16)(A) & 0x00ff) << 8))
	#define htonl(A) ((((u32)(A) & 0xff000000) >> 24) | \
	                  (((u32)(A) & 0x00ff0000) >> 8)  | \
	                  (((u32)(A) & 0x0000ff00) << 8)  | \
	                  (((u32)(A) & 0x000000ff) << 24))
	#define ntohs     htons
	#define ntohl     htohl
#endif
