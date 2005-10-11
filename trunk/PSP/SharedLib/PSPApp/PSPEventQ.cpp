/* 
	PSPEventQ Event Queue implementation for the PSP. (Initial Release: Sept. 2005)
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
#include "PSPEventQ.h"
#include <list>
using namespace std;

extern "C" {
int sceKernelClearEventFlag(u32, int);
int sceKernelWaitEventFlag(int evid, u32 bits, u32 wait, u32 *outBits, void *arg);
};

CPSPEventQ::CPSPEventQ(char *strName)
{
	char *strNameForResources = (char*)malloc(strlen(strName)+30);
	
	m_strName = strdup(strName);
	
	if (strNameForResources && m_strName)
	{
		
		sprintf(strNameForResources, "%sLock", m_strName);
		m_lock = new CLock(strNameForResources);
		Log(LOG_VERYLOW, "CPSPEventQ(%s): Created lock '%s' = %d", 
			strName, strNameForResources, m_lock->GetMutex());
		
		sprintf(strNameForResources, "%sRcvblk", m_strName);
		m_RcvBlockerEventId = sceKernelCreateEventFlag(strNameForResources, 0, 0x0, 0);
		
		Log (LOG_VERYLOW, "CPSPEventQ(%s): sceKernelCreateEventFlag(%s) returns 0x%x", strName, strNameForResources, m_RcvBlockerEventId);
		
		sprintf(strNameForResources, "%sRcvack", m_strName);
		m_RcvOKEventId = sceKernelCreateEventFlag(strNameForResources, 0, 0x0, 0);
		
		Log (LOG_VERYLOW, "CPSPEventQ(%s): sceKernelCreateEventFlag(%s) returns 0x%x", strName, strNameForResources, m_RcvOKEventId);
		
		free(strNameForResources), strNameForResources = NULL;
	}
	else
	{
		Log (LOG_ERROR, "CPSPEventQ(%s):Memory allocation error!", strName);
	}
}

CPSPEventQ::~CPSPEventQ()
{
	Log (LOG_VERYLOW, "~CPSPEventQ(): Calling sceKernelDeleteEventFlag()");
	int iRet = sceKernelDeleteEventFlag(m_RcvBlockerEventId);
	iRet = sceKernelDeleteEventFlag(m_RcvOKEventId);
	if(m_lock)
	{
		delete m_lock;
	}
	free(m_strName), m_strName = NULL;
	Log (LOG_VERYLOW, "~CPSPEventQ(): Done.");
}

int CPSPEventQ::Send(QEvent &Event)
{
	//Log(LOG_VERYLOW, "Send(): Calling Lock(mid=%x)", Event.EventId);
	m_lock->Lock();

	//Log(LOG_VERYLOW, "Send(): Calling Push_Back(mid=%x)", Event.EventId);
	m_EventList.push_back(Event);
	//notify receive
	//Log(LOG_VERYLOW, "Send(): Calling SetEvFlag(%x)", m_RcvBlockerEventId);
	int iRet = sceKernelSetEventFlag(m_RcvBlockerEventId, 0x1);
	
	if (iRet < 0)
	{
		Log(LOG_ERROR, "Send('%s'):Error On Send (msgid=0x%x) Error=0x%x", m_strName, Event.EventId, iRet);
	}
	
	m_lock->Unlock();

	
	return 0;
}

int CPSPEventQ::SendAndWaitForOK(QEvent &Event)
{
	m_lock->Lock();
	
	m_EventList.push_back(Event);
	//notify receive
	sceKernelSetEventFlag(m_RcvBlockerEventId, 0x1);
	
	//now, wait for OK.
	u32 flag = 0;
	sceKernelWaitEventFlag(m_RcvOKEventId, 0x1, 0, &flag, NULL);
	sceKernelClearEventFlag(m_RcvOKEventId, 10);
	
	m_lock->Unlock();
	
	return 0;
}

int CPSPEventQ::Receive(QEvent &Event)
{
	//block until notified
	u32 flag = 0;
	int iRet = sceKernelWaitEventFlag(m_RcvBlockerEventId, 0x1, 0/*wait0-and,1-OR*/, &flag, NULL);

	if (Size() > 0)
	{
		Event = m_EventList.front();
		m_EventList.pop_front();
	}
	else
	{
		Log(LOG_ERROR, "Receive('%s') Error. Receieve unblocked, but no messages in the queue?! size=%d. flag=0x%x iRet=0x%x",
			m_strName, Size(), flag, iRet);
	}
	
	/** Clear event flag, so Receive blocks when until the next message arrives */
	if (Size() == 0) /** Clear only if last message */
	{
		sceKernelClearEventFlag(m_RcvBlockerEventId, 10);//0x1);
		//Log(LOG_ERROR, "Receive() Size() is 0, clearing event flag.");
	}
	return 0;
}

int CPSPEventQ::SendReceiveOK()
{
	return sceKernelSetEventFlag(m_RcvOKEventId, 0x1);
}

int CPSPEventQ::Size()
{
	return m_EventList.size();
}

void CPSPEventQ::Clear()
{
	m_lock->Lock();

	while(m_EventList.size())
	{
		m_EventList.pop_front();
	}
	
	m_lock->Unlock();
}
