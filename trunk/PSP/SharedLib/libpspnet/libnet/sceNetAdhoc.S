	.set noreorder

#include "common.s"

  STUB_START "sceNetAdhoc",0x90000,0x00190005
    STUB_FUNC 0xe1d621d7,sceNetAdhocInit
    STUB_FUNC 0xa62c6f57,sceNetAdhocTerm
    STUB_FUNC 0x7a662d6b,sceNetAdhocPollSocket
    STUB_FUNC 0x73bfd52d,sceNetAdhocSetSocketAlert
    STUB_FUNC 0x4d2ce199,sceNetAdhocGetSocketAlert
    STUB_FUNC 0x6f92741b,sceNetAdhocPdpCreate
    STUB_FUNC 0xabed3790,sceNetAdhocPdpSend       
    STUB_FUNC 0xdfe53e03,sceNetAdhocPdpRecv       
    STUB_FUNC 0x7f27bb5e,sceNetAdhocPdpDelete     
    STUB_FUNC 0xc7c1fc57,sceNetAdhocGetPdpStat    
    STUB_FUNC 0x877f6d66,sceNetAdhocPtpOpen       
    STUB_FUNC 0xfc6fc07b,sceNetAdhocPtpConnect    
    STUB_FUNC 0xe08bdac1,sceNetAdhocPtpListen     
    STUB_FUNC 0x9df81198,sceNetAdhocPtpAccept     
    STUB_FUNC 0x4da4c788,sceNetAdhocPtpSend       
    STUB_FUNC 0x8bea2b3e,sceNetAdhocPtpRecv       
    STUB_FUNC 0x9ac2eeac,sceNetAdhocPtpFlush      
    STUB_FUNC 0x157e6225,sceNetAdhocPtpClose      
    STUB_FUNC 0xb9685118,sceNetAdhocGetPtpStat    
    STUB_FUNC 0x7f75c338,sceNetAdhocGameModeCreateMaster
    STUB_FUNC 0x3278ab0c,sceNetAdhocGameModeCreateReplica
    STUB_FUNC 0x98c204c8,sceNetAdhocGameModeUpdateMaster 
    STUB_FUNC 0xfa324b4e,sceNetAdhocGameModeUpdateReplica
    STUB_FUNC 0xa0229362,sceNetAdhocGameModeDeleteMaster 
    STUB_FUNC 0x0b2228e9,sceNetAdhocGameModeDeleteReplica
  STUB_END 
