#include <pspkernel.h>
#include <pspdebug.h>
#include <pspsdk.h>
#include <sys/select.h>
#include <errno.h>
#include <curl/curl.h>
#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <pspnet_resolver.h>

#include "wifi_user.h"

int pspUserLoadInetModules() {
	int res = sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
	if (res == 0 ) res = sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
	return res;
}

int pspUserInetInit()
{
	u32 retVal;

	retVal = sceNetInit(0x20000, 0x20, 0x1000, 0x20, 0x1000);
	if (retVal != 0)
		return retVal;

	retVal = sceNetInetInit();
	if (retVal != 0)
		return retVal;

	retVal = sceNetResolverInit();
	if (retVal != 0)
		return retVal;
	
	retVal = sceNetApctlInit(0x1400, 0x42); // increased stack size
	if (retVal != 0)
		return retVal;

	return 0;
}
