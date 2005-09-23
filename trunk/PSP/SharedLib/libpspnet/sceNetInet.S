	.set noreorder

#include "common.s"

  STUB_START "sceNetInet",0x90000,0x001c0005
    STUB_FUNC 0x17943399,sceNetInetInit 
    STUB_FUNC 0xa9ed66b9,sceNetInetTerm 
    STUB_FUNC 0xdb094e1b,sceNetInetAccept
    STUB_FUNC 0x1a33f9ae,sceNetInetBind  
    STUB_FUNC 0x8d7284ea,sceNetInetClose 
    STUB_FUNC 0x805502dd,sceNetInetCloseWithRST
    STUB_FUNC 0x410b34aa,sceNetInetConnect 
    STUB_FUNC 0xe247b6d6,sceNetInetGetpeername
    STUB_FUNC 0x162e6fd5,sceNetInetGetsockname
    STUB_FUNC 0x4a114c7c,sceNetInetGetsockopt 
    STUB_FUNC 0xd10a1a7a,sceNetInetListen     
    STUB_FUNC 0xfaabb1dd,sceNetInetPoll       
    STUB_FUNC 0xcda85c99,sceNetInetRecv       
    STUB_FUNC 0xc91142e4,sceNetInetRecvfrom   
    STUB_FUNC 0xeece61d2,sceNetInetRecvmsg    
    STUB_FUNC 0x5be8d595,sceNetInetSelect     
    STUB_FUNC 0x7aa671bc,sceNetInetSend       
    STUB_FUNC 0x05038fc7,sceNetInetSendto     
    STUB_FUNC 0x774e36f4,sceNetInetSendmsg    
    STUB_FUNC 0x2fe71fe7,sceNetInetSetsockopt 
    STUB_FUNC 0x4cfe4e56,sceNetInetShutdown   
    STUB_FUNC 0x8b7b220f,sceNetInetSocket
    STUB_FUNC 0x80a21abd,sceNetInetSocketAbort
    STUB_FUNC 0xfbabe411,sceNetInetGetErrno
    STUB_FUNC 0xb75d5b0a,sceNetInetInetAddr
    STUB_FUNC 0x1bdf5d13,sceNetInetInetAton
    STUB_FUNC 0xd0792666,sceNetInetInetNtop
    STUB_FUNC 0xe30b8c19,sceNetInetInetPton
  STUB_END

