	.set noreorder

#include "common.s"

  STUB_START "sceNetApctl",0x90000,0x00080005
    STUB_FUNC 0xe2f91f9b,sceNetApctlInit     
    STUB_FUNC 0xb3edd0ec,sceNetApctlTerm      
    STUB_FUNC 0x2befdf23,sceNetApctlGetInfo   
    STUB_FUNC 0x8abadd51,sceNetApctlAddHandler
    STUB_FUNC 0x5963991b,sceNetApctlDelHandler
    STUB_FUNC 0xcfb957c6,sceNetApctlConnect   
    STUB_FUNC 0x24fe91a1,sceNetApctlDisconnect
    STUB_FUNC 0x5deac81b,sceNetApctlGetState
  STUB_END
