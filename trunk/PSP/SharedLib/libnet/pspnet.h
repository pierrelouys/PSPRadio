#ifndef __PSPNET_H
#define __PSPNET_H

	#include <sys/types.h>
	#include <psputility.h>
	#include <reent.h>
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
	
	//defined in sdk (psputility.h/psputility_netconf.h)//int sceUtilityCheckNetParam(int iConfig);
	//same as above//int sceUtilityGetNetParam(int iConfig, u32 r5_gettype, void* r6return);
	
	
	// socket layer - patterned after Berkeley/WinSock sockets 
	// functions in this header can be used directly after init
	// if you prefer, you can #define them to the regular "socket()", "send()", "recv()" names
	//typedef int size_t;
	typedef size_t socklen_t;
	#define SOCKET int						/* for 'socket()' */
	#define AF_INET         2               /* internetwork: UDP, TCP, etc. */
	#define SOCK_STREAM     1               /* stream socket */
	#define SOCK_DGRAM      2               /* datagram socket */
	#define TCP_NODELAY     0x0001
	#define IPPROTO_TCP     6
	#define IPPROTO_UDP     17
	#define IPPROTO_IP      0
	
	#define SOL_SOCKET	0xffff
	#define SO_NOBLOCK	0x1009
	
	struct sockaddr {
		unsigned char sa_len;
		unsigned char sa_family;              /* address family */
		// REVIEW: is this correct?
		char    sa_data[14];            /* up to 14 bytes of direct address */
	};
	
	// Socket address, internet style.
	struct sockaddr_in {
		unsigned char sin_len;
		unsigned char sin_family; // REVIEW: is this correct ?
		unsigned short sin_port; // use htons()
		unsigned char sin_addr[4];
		char    sin_zero[8];
	};
	
	/* From glibc's bits/socket.h: Structure describing messages sent by
	`sendmsg' and received by `recvmsg'.  */
	struct msghdr
	{
		void *msg_name;		/* Address to send to/receive from.  */
		socklen_t msg_namelen;	/* Length of address data.  */
	
		struct iovec *msg_iov;	/* Vector of data to send/receive into.  */
		size_t msg_iovlen;		/* Number of elements in the vector.  */
		
		void *msg_control;		/* Ancillary data (eg BSD filedesc passing). */
		size_t msg_controllen;	/* Ancillary data buffer length.  */
		
		int msg_flags;		/* Flags on received message.  */
	};
	
	SOCKET sceNetInetSocket(int af, int type, int protocol);
	int sceNetInetSend(SOCKET s, const void* buf, int len, int flags);
	int sceNetInetSendto(SOCKET s, const void* buf, int len, int flags,
				const void* sockaddr_to, int tolen);
	int sceNetInetConnect(SOCKET s, const void *name, int namelen);
	int sceNetInetBind(SOCKET s, void *addr, int namelen);
	int sceNetInetClose(SOCKET s);
	int sceNetInetGetErrno();
	int sceNetInetRecv(SOCKET s, void* buf, int len, int flags);
	int sceNetInetSelect(SOCKET s, fd_set *r, fd_set *w, fd_set *o, struct timeval *timeout);
	int sceNetInetSetsockopt (SOCKET s, int level, int optname, const void *val, int len);
	int sceNetInetListen(SOCKET s, int backlog);
	/** accept - accept a new connection on a socket */
	int sceNetInetAccept(SOCKET s, struct sockaddr_in *address, socklen_t *address_len);
	/** getpeername - get the name of the peer socket */
	int sceNetInetGetpeername(SOCKET s, struct sockaddr *address, socklen_t *address_len);
	/** recvmsg - receive a message from a socket */
	int sceNetInetRecvmsg(SOCKET s, struct msghdr *message, int flags);
	/** select - synchronous I/O multiplexing */
	int sceNetInetSelect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timeval *timeout);
	
	
	#if 0
	//REVIEW: flesh these in
	int sceNetInetCloseWithRST(SOCKET s);
	int sceNetInetGetsockname(
	int sceNetInetGetsockopt(
	int sceNetInetPoll(
	int sceNetInetRecvfrom(
	int sceNetInetSendmsg(
	int sceNetInetSetsockopt(
	int sceNetInetShutdown(
	int sceNetInetSocketAbort(
	int sceNetInetInetAddr(
	int sceNetInetInetAton(
	int sceNetInetInetNtop(
	int sceNetInetInetPton(
	#endif
	
	// Other utilities
	unsigned short htons(unsigned short wIn);
	unsigned long htonl(unsigned long dwIn);
	
	///////////////////////////////////////////////
#endif
