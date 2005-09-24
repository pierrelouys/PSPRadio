	.set noreorder

#include "common.s"

  STUB_START "sceNetResolver",0x90000,0x00070005
    STUB_FUNC 0xf3370e61,sceNetResolverInit     
    STUB_FUNC 0x6138194a,sceNetResolverTerm     
    STUB_FUNC 0x244172af,sceNetResolverCreate   
    STUB_FUNC 0x94523e09,sceNetResolverDelete   
    STUB_FUNC 0x224c5f44,sceNetResolverStartNtoA
    STUB_FUNC 0x629e2fb7,sceNetResolverStartAtoN
    STUB_FUNC 0x808f6063,sceNetResolverStop
  STUB_END
