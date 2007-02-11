#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <malloc.h>
#include <pspsdk.h>
#include <Logging.h>
#include "PRXLoader.h"

#define MAX_ARGS 2048

CPRXLoader::CPRXLoader()
{
	m_ModId = -1;
	m_FileName = NULL;
	m_error = 0;
	m_IsStarted = false;
	m_IsLoaded = false;
	m_Name = strdup("Off");
}

CPRXLoader::~CPRXLoader()
{
	Unload();
	if (m_FileName)
	{
		free(m_FileName), m_FileName = NULL;
	}
	if (m_Name)
	{
		free(m_Name), m_Name = NULL;
	}

}

void CPRXLoader::SetName(char *strName)
{
	if (m_Name)
	{
		free(m_Name);
	}
	m_Name = strdup(strName);
}

char *CPRXLoader::GetName()
{
	if (m_Name)
	{
		return m_Name;
	}
	else
	{
		return "Off";
	}
}


int CPRXLoader::Load(const char *filename)
{
	SceKernelLMOption option;
	SceUID mpid = PSP_MEMORY_PARTITION_USER;

	memset(&option, 0, sizeof(option));
	
	option.size = sizeof(option);
	option.mpidtext = mpid;
	option.mpiddata = mpid;
	option.position = 0;
	option.access = 1;

	SceSize am = sceKernelTotalFreeMemSize();
	Log(LOG_LOWLEVEL, "Load('%s'): Available memory before loading: %dbytes (%dKB or %dMB)", filename, am, am/1024, am/1024/1024);
	am = sceKernelMaxFreeMemSize();
	Log(LOG_LOWLEVEL, "Load('%s'): Max(continguos) memory before loading: %dbytes (%dKB or %dMB)", filename, am, am/1024, am/1024/1024);


	m_ModId = sceKernelLoadModule(filename, 0, &option);
	
	Log(LOG_LOWLEVEL, "Load('%s') Module Id=%d", filename, m_ModId);
	sleep(1);
	
	if (m_ModId > 0)
	{
		if (m_FileName)
		{
			free(m_FileName), m_FileName = NULL;
		}
		m_FileName = strdup(filename);
		m_error = 0;
		m_IsLoaded = true;
	}
	else
	{
		m_error = m_ModId;
		m_IsLoaded = false;
	}
	
	return m_ModId;
}

int CPRXLoader::Unload()
{
	int ret = 0;
	int status = 0;
	
	if (IsLoaded() == true)
	{
		Log(LOG_LOWLEVEL, "Stopping Module %d", m_ModId);
		if (IsStarted() == true)
		{
			// Stop
			Log(LOG_LOWLEVEL, "Unload(): Module was started, so stopping first.");
			ret = sceKernelStopModule(m_ModId, 0, NULL, &status, NULL);
			m_IsStarted = false;
		}
		
		Log(LOG_LOWLEVEL, "StopModule returned 0x%x", ret);
		
		// Unload
		///if(ret >= 0)
		{
			Log(LOG_LOWLEVEL, "Unload(): Unloading Module");
			ret = sceKernelUnloadModule(m_ModId);
			Log(LOG_LOWLEVEL, "UnloadModule returned 0x%x", ret);
		}
		
		sleep(1);
	
		m_ModId = 0;
		m_IsLoaded = false;
	}

	return 0;
}

int CPRXLoader::Start(int argc, char * const argv[])
{
	return StartModuleWithArgs(m_FileName, m_ModId, argc, argv);
}

SceUID CPRXLoader::StartModuleWithArgs(char *filename, int modid, int argc, char * const argv[])
{
	int retVal = 0, mresult = 0;
	char args[MAX_ARGS];
	int  argpos = 0;
	int  i = 0;
	
	memset(args, 0, MAX_ARGS);
	strcpy(args, filename);
	argpos += strlen(args) + 1;
	
	for(i = 0; (i < argc) && (argpos < MAX_ARGS); i++)
	{
		int len;
		snprintf(&args[argpos], MAX_ARGS-argpos, "%s", argv[i]);
		len = strlen(&args[argpos]);
		argpos += len + 1;
	}
	
	retVal = sceKernelStartModule(modid, argpos, args, &mresult, NULL);
	
	if(retVal < 0)
	{
		m_IsStarted = false;
		return retVal;
	}
	
	m_IsStarted = true;
	return modid;
}


