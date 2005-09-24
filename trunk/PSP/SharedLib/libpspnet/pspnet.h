#ifndef __PSPNET_H
#define __PSPNET_H
	
	#ifndef __PSP__
		#define __PSP__
	#endif
	
	#ifdef __cplusplus
		extern "C" {
	#endif
 

	#include <pspkerneltypes.h>
	#include <sys/types.h>
	#include <psputility.h>
	#include <reent.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>
	//#include <w3c-libwww/wwwsys.h> /** RC */
	
	#ifndef BOOLEAN
	#define BOOLEAN char
	#define TRUE (BOOLEAN)1
	#define FALSE (BOOLEAN)0
	#endif
	
	
	int nlhLoadDrivers();
	int nlhInit();
	int nlhTerm();
	
	int sceNetInit(SceSize poolsize, int callout_tpl, SceSize callout_stack, int netintr_tpl, SceSize netintr_stack); 
	int sceNetTerm();
	
	int sceNetGetLocalEtherAddr(u8* addr);
	
	int sceNetInetInit();
	int sceNetInetTerm();
	
	/** Resolver */
	#include <netinet/in.h>

	int sceNetResolverInit();
	int sceNetResolverTerm();
	int sceNetResolverCreate(int *rid, void *buf, SceSize buflen);
 	int sceNetResolverDelete(int rid);
	int sceNetResolverStartNtoA(int rid, const char *hostname, struct in_addr *addr, unsigned int timeout, int retry);
	int sceNetResolverStartAtoN(int rid, const struct in_addr *in_addr, char *hostname, SceSize hostname_len, unsigned int timeout, int retry);
	int sceNetResolverStop(int rid);
	/** End Resolver */
	
	int sceNetAdhocInit(); 
	int sceNetAdhocctlInit(u32 r4, u32 r5, void* r6);
	int sceNetAdhocTerm();
	int sceNetAdhocctlConnect(const char*);
	
	int sceNetApctlConnect(int profile);
	int sceNetApctlInit(u32 stacksize, u32 prio);
	int sceNetApctlTerm();
	int sceNetApctlGetState(u32* stateOut);
	
	
	
/* code */
	enum apctl_info
	{
		 SCE_NET_APCTL_INFO_CNF_NAME          =0,
		 SCE_NET_APCTL_INFO_BSSID             =1,
		 SCE_NET_APCTL_INFO_SSID              =2,
		 SCE_NET_APCTL_INFO_SSID_LEN          =3,
		 SCE_NET_APCTL_INFO_AUTH_PROTO        =4,
		 SCE_NET_APCTL_INFO_RSSI              =5,
		 SCE_NET_APCTL_INFO_CHANNEL           =6,
		 SCE_NET_APCTL_INFO_PWRSAVE           =7,
		 SCE_NET_APCTL_INFO_IP_ADDRESS        =8,
		 SCE_NET_APCTL_INFO_NETMASK           =9,
		 SCE_NET_APCTL_INFO_DEFAULT_ROUTE     =10,
		 SCE_NET_APCTL_INFO_PRIMARY_DNS       =11,
		 SCE_NET_APCTL_INFO_SECONDARY_DNS     =12,
		 SCE_NET_APCTL_INFO_HTTP_PROXY_FLAG   =13,
		 SCE_NET_APCTL_INFO_HTTP_PROXY_SERVER =14,
		 SCE_NET_APCTL_INFO_HTTP_PROXY_PORT   =15
	};
	int sceNetApctlGetInfo(int code, void *info); 
	int sceNetApctlDisconnect();
	
	
	/* states */
	enum apctl_states
	{
		apctl_state_disconnected	= 0,
		apctl_state_scanning		= 1,
		apctl_state_joining			= 2,
		apctl_state_IPObtaining		= 3,
		apctl_state_IPObtained		= 4
	};
	
	/* events */
	enum apctl_events
	{
		 apctl_event_CONNECT_REQ	= 0,
		 apctl_event_SCAN_REQ		= 1,
		 apctl_event_SCAN_COMPLETED	= 2,
		 apctl_event_ESTABLISH		= 3,
		 apctl_event_GET_IP			= 4,
		 apctl_event_DISCONNECT_REQ	= 5,
		 apctl_event_ERROR			= 6,
		 apctl_event_INFO			= 7
	};
	
	typedef void (*sceNetApctlHandler)(int prev_state, int new_state, int event, int error_code, void *arg);
	int sceNetApctlAddHandler(sceNetApctlHandler handler, void *arg); 	
	
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
	
	int sceNetInetGetErrno();
	
	//char *inet_ntoa(struct in_addr in);
	struct in_addr_b
	{
		unsigned char b1,b2,b3,b4;
	};
	#include <netinet/in.h>	
	char *sceNetInetInetNtoa (struct in_addr in);
	
	#ifdef __cplusplus
		}
	#endif

#endif
