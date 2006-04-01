/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * main.c - Simple PRX example.
 *
 * Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
 *
 * $Id: main.c 1531 2005-12-07 18:27:12Z tyranid $
 */
#include <pspctrl.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <Tools.h>
#include <PSPRadio_Exports.h>
#include <APP_Exports.h>
#include <Common.h>

PSP_MODULE_INFO("APP_NetScan", 0, 1, 1);
PSP_HEAP_SIZE_KB(64);

#define printf pspDebugScreenPrintf

int wlanscan_main();
int wlanscan();
void do_resolver();
void app_plugin_main();

int ModuleStartAPP()
{
	sleep(1);
	
	SceSize am = sceKernelTotalFreeMemSize();
	ModuleLog(LOG_INFO, "ModuleStartApp(): Available memory: %dbytes (%dKB or %dMB)", am, am/1024, am/1024/1024);

	int thid = 0;

	thid = sceKernelCreateThread("app_thread", (void*) app_plugin_main, 0x50, 0xFA0*2, PSP_THREAD_ATTR_USER, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}


	//wait_for_button();
	
	return 0;
}

int ModuleContinueApp()
{
	return 0;
}

void wait_for_triangle()
{
	printf("** Press TRIANGLE **");
	SceCtrlData pad;
	for(;;) 
	{
		sceDisplayWaitVblankStart();
		sceCtrlReadBufferPositive(&pad, 1);
		if (pad.Buttons & PSP_CTRL_TRIANGLE)
		{
			break;
		}
	}
}

void app_plugin_main()
{
	int run = 1;

	PSPRadioExport_RequestExclusiveAccess(PLUGIN_APP);

	while (run == 1)
	{
		pspDebugScreenInit();
		printf(" NetScan Plugin for PSPRadio\n");
		printf("-----------------------------\n");
		printf("* CIRCLE: Perform WiFi Scan\n");
		printf("* SQUARE: Perform Resolver Test\n");
		printf("* CROSS:  Exit *\n");
		SceCtrlData pad;
		for(;;) 
		{
			sceDisplayWaitVblankStart();
			sceCtrlReadBufferPositive(&pad, 1);
			if (pad.Buttons & PSP_CTRL_CROSS)
			{
				run = 0;
				break;
			}
			else if (pad.Buttons & PSP_CTRL_CIRCLE)
			{
				pspDebugScreenInit();
				wlanscan();
				wait_for_triangle();
				break;
			}
			else if (pad.Buttons & PSP_CTRL_SQUARE)
			{
				pspDebugScreenInit();
				do_resolver();
				wait_for_triangle();
				break;
			}
		}
	}

	pspDebugScreenInit();
	PSPRadioExport_GiveUpExclusiveAccess();
}

/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * main.c - Example of using the WLAN scanner
 * The info here was reversed from scan.prx which Sony/Namco so 
 * helpfully left in plain sight on the japanese ridge racer UMD
 * Thx boys :)
 *
 * Copyright (c) 2006 James F
 *
 * $Id: main.c 1699 2006-01-15 23:46:00Z tyranid $
 * $HeadURL: svn://svn.pspdev.org/psp/trunk/pspsdk/src/samples/net/wlanscan/main.c $
 */
#include <pspkernel.h>
#include <stdio.h>
#include <pspnet.h>
#include <pspwlan.h>
#include <string.h>

/* Init the scan */
int sceNet_lib_5216CBF5(const char *name);
/* Do the scan */
int sceNet_lib_7BA3ED91(const char *name, void *type, u32 *size, void *buf, u32 *unk);
/* Terminate the scan */
int sceNet_lib_D2422E4D(const char *name);

#define InitScan sceNet_lib_5216CBF5
#define ScanAPs  sceNet_lib_7BA3ED91
#define TermScan sceNet_lib_D2422E4D

/* Global buffer to store the scan data */
unsigned char scan_data[0xA80];

/* Returned data */
struct ScanData
{
	struct ScanHead *pNext; 
	unsigned char bssid[6]; 
	char channel; 
	unsigned char namesize; 
	char name[32]; 
	unsigned int bsstype; 
	unsigned int beaconperiod; 
	unsigned int dtimperiod; 
	unsigned int timestamp; 
	unsigned int localtime; 
	unsigned short atim; 
	unsigned short capabilities; 
	unsigned char  rate[8]; 
	unsigned short rssi; 
	unsigned char  sizepad[6]; 
} __attribute__((packed));

/* Capability flags */
const char *caps[8] = {
	"ESS, ",
	"IBSS, ",
	"CF Pollable, ",
	"CF Pollreq,  ",
	"Privacy, ",
	"Short Preamble, ",
	"PBCC, ",
	"Channel Agility, "
};

/* Print the scan data to stdout */
void print_scan(unsigned char *data, size_t size)
{
	int i = 1;

	while(size >= sizeof(struct ScanData))
	{
		char name[33];
		char bssid[30];
		int  loop;
		struct ScanData *pData;

		pData = (struct ScanData *) data;
		printf("==================\n");
		printf("****** BSS: %d\n", i);
		strncpy(name, pData->name, 32);
		name[32] = 0;
		sceNetEtherNtostr(pData->bssid, bssid);
		printf("BSSID: '%s' ", bssid);
		printf("SSID: '%s' ", name);
		printf("bsstype: '");
		if(pData->bsstype == 1)
		{
			printf("Infrastructure'\n");
		}
		else if(pData->bsstype == 2)
		{
			printf("Independent'\n");
		}
		else
		{
			printf("Unknown'\n");
		}
		printf("Beacon Period: %d ", pData->beaconperiod);
		printf("DTIM period: %d ", pData->dtimperiod);
		printf("Timestamp: %d ", pData->timestamp);
		printf("Local Time: %d\n", pData->localtime);
		printf("Channel: %d ", pData->channel);
		printf("ATIM: %d\n", pData->atim);
		printf("Capability Information: ");
		for(loop = 0; loop < 8; loop++)
		{
			if(pData->capabilities & (1 << loop))
			{
				printf("%s", caps[loop]);
			}
		}
		printf("\n");

		printf("Rate: ");
		for(loop = 0; loop < 8; loop++)
		{
			const char *type;

			if(pData->rate[loop] & 0x80)
			{
				type = "Basic";
			}
			else
			{
				type = "Operational";
			}

			if(pData->rate[loop] & 0x7F)
			{
				printf("%s %d kbps, ", type, (pData->rate[loop] & 0x7F) * 500);
			}
		}
		printf("\n");

		printf("RSSI: %d\n", pData->rssi);

		printf("\n");
		i++;
		data += sizeof(struct ScanData);
		size -= sizeof(struct ScanData);
	}
}

/* Do a scan */
void do_scan(void)
{
	unsigned char type[0x4C];
	u32 size, unk;
	int i;
	int ret;

	if(InitScan("wlan") >= 0)
	{
		/* No real idea what this is doing ;) */
		memset(type, 0, sizeof(type));
		for(i = 1; i < 0xF; i++)
		{
			type[0x9+i] = i;
		}
		type[0x3C] = 1;
		*((u32*) (type + 0x44)) = 6;    /* Minimum strength */
		*((u32*) (type + 0x48)) = 100;  /* Maximum strength */
		size = sizeof(scan_data);
		unk  = 0;
		memset(scan_data, 0, sizeof(scan_data));
		ret = ScanAPs("wlan", type, &size, scan_data, &unk);
		if(ret < 0)
		{
			printf("Error, could not perform scan err = %08X\n", ret);
		}
		else
		{
			print_scan(scan_data, size);
		}
	}
	else
	{
		printf("Error, cannot initialise scan\n");
	}

	TermScan("wlan");
}

int wlanscan()
{
	int ret;
	printf(" WLANScan (based on pspsdk sample by James F)\n");
	printf("----------------------------------------------\n");

	ret = -1;
	while(ret < 0)
	{
		ret = sceWlanDevAttach();

		/* If returns a error which indicates it isn't ready */
		if(ret == (int)0x80410D0E)
		{
			sceKernelDelayThread(1000000);
		}
		else if(ret < 0)
		{
			printf("Error attaching to wlan device %08X\n", ret);
			goto error;
		}
	}

	do_scan();
error:
	sceWlanDevDetach();

	return 0;
}

#define RESOLVE_NAME "pspradio.berlios.de"
#include <pspnet_resolver.h>

void do_resolver()
{
	int rid = -1;
	char buf[1024];
	struct in_addr addr;
	char name[1024];
	u32 err = 0;
	
	printf(" Resolver Test (based on pspsdk sample by James F)\n");
	printf("---------------------------------------------------\n");
	printf("\n* PSPRadio IP: '%s'\n", PSPRadioExport_GetMyIP());
	ModuleLog(LOG_INFO, "NetScan Plugin: Resolver Test. PSP IP = %s", PSPRadioExport_GetMyIP());

	do
	{
		/* Create a resolver */
		memset(buf, 0, sizeof(buf));
		err = sceNetResolverCreate(&rid, buf, 1024);
		if(err < 0)
		{
			printf(">> Error creating resolver <<\n");
			ModuleLog(LOG_ERROR, "NetScan Plugin: Resolver Test. Error creating Resolver. PSPIP='%s' Err=0x%x", PSPRadioExport_GetMyIP(), err);
			break;
		}

		printf("* Created resolver 0x%08x\n", rid);

		/* Resolve a name to an ip address */
		memset(&addr, 0, sizeof(addr));
		err = sceNetResolverStartNtoA(rid, RESOLVE_NAME, &addr, 2, 3);
		if(err < 0)
		{
			printf(">> Error resolving %s <<\n", RESOLVE_NAME);
			ModuleLog(LOG_ERROR, "NetScan Plugin: Resolver Test. Failure resolving hostname. PSPIP='%s' Err=0x%x", PSPRadioExport_GetMyIP(), err);
			break;
		}

		printf("* Resolved '%s' to '%s'\n", RESOLVE_NAME, inet_ntoa(addr));

		/* Resolve the ip address to a name */
		memset(name, 0, 1024);
		err = sceNetResolverStartAtoN(rid, &addr, name, 1024, 2, 3);
		if(err < 0)
		{
			printf("Error resolving ip to name\n");
			ModuleLog(LOG_ERROR, "NetScan Plugin: Resolver Test. Failure resolving ip back to name. PSPIP='%s' Err=0x%x", PSPRadioExport_GetMyIP(), err);
			break;
		}

		ModuleLog(LOG_INFO, "NetScan Plugin: Resolver Test. Success: Hostname='%s' resolved to IP='%s' resolved to '%s'",
				RESOLVE_NAME, inet_ntoa(addr), name);
		printf("* Resolved ip to '%s'\n", name);
	}
	while(0);

	if(rid >= 0)
	{
		sceNetResolverDelete(rid);
	}
	printf("\n");
}
