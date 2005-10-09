/* 
	PSPMessageQ Message Queue implementation for the PSP. (Initial Release: Sept. 2005)
	Copyright (C) 2005  Rafael Cabezas a.k.a. Raf
	
	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <malloc.h>
#include <PSPApp.h>
#include <Logging.h>
#include "PSPMessageQ.h"
#include <list>
using namespace std;

extern "C" {
int sceKernelClearEventFlag(u32, int);
};

CPSPMessageQ::CPSPMessageQ(char *strName)
{
	char *strNameForResources = (char*)malloc(strlen(strName)+10);
	
	m_strName = strdup(strName);
	
	if (strNameForResources && m_strName)
	{
		
		sprintf(strNameForResources, "%s_lock", m_strName);
		m_lock = new CLock(strNameForResources);
		
		sprintf(strNameForResources, "%s_event", m_strName);
		m_EventId = sceKernelCreateEventFlag(strNameForResources, 0, 0x0, 0);
		
		Log (LOG_VERYLOW, "CPSPMessageQ(): sceKernelCreateEventFlag() returns 0x%x", m_EventId);
		
		free(strNameForResources), strNameForResources = NULL;
	}
	else
	{
		Log (LOG_ERROR, "CPSPMessageQ():Memory allocation error!");
	}
}

CPSPMessageQ::~CPSPMessageQ()
{
	Log (LOG_VERYLOW, "~CPSPMessageQ(): Calling sceKernelDeleteEventFlag()");
	int iRet = sceKernelDeleteEventFlag(m_EventId);
	Log (LOG_VERYLOW, "~CPSPMessageQ(): sceKernelDeleteEventFlag() returns 0x%x", iRet);
	if(m_lock)
	{
		delete m_lock;
	}
	free(m_strName), m_strName = NULL;
}

int CPSPMessageQ::Send(QMessage &Message)
{
	m_lock->Lock();

	m_msglist.push_back(Message);
	//notify receive
	/*int iRet = */sceKernelSetEventFlag(m_EventId, 0x1);
	
	m_lock->Unlock();

	
	return 0;
}

int CPSPMessageQ::Receive(QMessage &Message)
{
	//block until notified
	u32 flag = 0;
	/*int iRet = */sceKernelWaitEventFlag(m_EventId, 0x1, 0/*wait0-and,1-OR*/, &flag, NULL);

	if (Size() > 0)
	{
		Message = m_msglist.front();
		m_msglist.pop_front();
	}
	else
	{
		Log(LOG_ERROR, "Receive() Error. Receieve unblocked, but no messages in the queue?! size=%d. flag=0x%x", Size(), flag);
	}
	
	/** Clear event flag, so Receive blocks when until the next message arrives */
	if (Size() == 0) /** Clear only if last message */
	{
		sceKernelClearEventFlag(m_EventId, 10);//0x1);
		Log(LOG_ERROR, "Receive() Size() is 0, clearing event flag.");
	}
	return 0;
}
	
int CPSPMessageQ::Size()
{
	return m_msglist.size();
}

void CPSPMessageQ::Clear()
{
	m_lock->Lock();

	while(m_msglist.size())
	{
		m_msglist.pop_front();
	}
	
	m_lock->Unlock();
}
