	.set noreorder

#include "pspstub.s"

	STUB_START "Dummy",0x00090000,0x00020005
	STUB_FUNC  0x563FF2B2,getModuleInfo
	STUB_END
