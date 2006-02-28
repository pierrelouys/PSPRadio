#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <malloc.h>
#include <pspsdk.h>
#include "PRXLoader.h"

#define MAX_ARGS 2048

CPRXLoader::CPRXLoader()
{
	m_ModId = -1;
	m_FileName = NULL;
	m_error = 0;
	m_IsStarted = false;
}

CPRXLoader::~CPRXLoader()
{
	if (m_FileName)
	{
		free(m_FileName), m_FileName = NULL;
	}

}


int CPRXLoader::Load(char *filename)
{
	SceKernelLMOption option;
	SceUID mpid = PSP_MEMORY_PARTITION_USER;

	memset(&option, 0, sizeof(option));
	
	option.size = sizeof(option);
	option.mpidtext = mpid;
	option.mpiddata = mpid;
	option.position = 0;
	option.access = 1;

	m_ModId = sceKernelLoadModule(filename, 0, &option);
	
	if (m_ModId > 0)
	{
		if (m_FileName)
		{
			free(m_FileName), m_FileName = NULL;
		}
		m_FileName = strdup(filename);
		m_error = 0;
	}
	else
	{
		m_error = m_ModId;
	}
	
	return m_ModId;
}

int CPRXLoader::Unload()
{
	int ret = 0;
	int status = 0;
	
	if (IsLoaded() == true)
	{
		if (IsStarted() == true)
		{
			// Stop
			ret = sceKernelStopModule(m_ModId, 0, NULL, &status, NULL);
			m_IsStarted = false;
		}
		
		// Unload
		if(ret >= 0)
		{
			ret = sceKernelUnloadModule(m_ModId);
		}
	
		m_ModId = 0;
	}
	
	return 0;
}

int CPRXLoader::Start(int argc, char * const argv[])
{
	return StartModuleWithArgs(m_FileName, m_ModId, argc, argv);
}

SceUID CPRXLoader::StartModuleWithArgs(char *filename, int modid, int argc, char * const argv[])
{
	int retVal = 0, mresult;
	char args[MAX_ARGS];
	int  argpos = 0;
	int  i;
	
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


