	.set noreorder

#include "common.s"

  STUB_START "sceNetAdhocctl",0x90000,0x00140005
    STUB_FUNC 0xe26f226e,sceNetAdhocctlInit     
    STUB_FUNC 0x9d689e13,sceNetAdhocctlTerm     
    STUB_FUNC 0x0ad043ed,sceNetAdhocctlConnect  
    STUB_FUNC 0xec0635c1,sceNetAdhocctlCreate   
    STUB_FUNC 0x5e7f79c9,sceNetAdhocctlJoin     
    STUB_FUNC 0x08fff7a0,sceNetAdhocctlScan     
    STUB_FUNC 0x34401d65,sceNetAdhocctlDisconnect
    STUB_FUNC 0x20b317a0,sceNetAdhocctlAddHandler
    STUB_FUNC 0x6402490b,sceNetAdhocctlDelHandler
    STUB_FUNC 0x75ecd386,sceNetAdhocctlGetState  
    STUB_FUNC 0x362cbe8f,sceNetAdhocctlGetAdhocId
    STUB_FUNC 0xe162cb14,sceNetAdhocctlGetPeerList
    STUB_FUNC 0x99560abe,sceNetAdhocctlGetAddrByName
    STUB_FUNC 0x8916c003,sceNetAdhocctlGetNameByAddr
    STUB_FUNC 0xded9d28e,sceNetAdhocctlGetParameter 
    STUB_FUNC 0x81aee1be,sceNetAdhocctlGetScanInfo  
    STUB_FUNC 0xa5c055ce,sceNetAdhocctlCreateEnterGameMode
    STUB_FUNC 0x1ff89745,sceNetAdhocctlJoinEnterGameMode  
    STUB_FUNC 0xcf8e084d,sceNetAdhocctlExitGameMode       
    STUB_FUNC 0x5a014ce0,sceNetAdhocctlGetGameModeInfo    
  STUB_END 
