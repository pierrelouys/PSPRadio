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
		m_RcvBlocker = new CBlocker(strNameForResources);
		
		sprintf(strNameForResources, "%sRcvack", m_strName);
		m_RcvOKBlocker = new CBlocker(strNameForResources);
		
		
		free(strNameForResources), strNameForResources = NULL;
	}
	else
	{
		Log (LOG_ERROR, "CPSPEventQ(%s):Memory allocation error!", strName);
	}
}

CPSPEventQ::~CPSPEventQ()
{
	Log (LOG_VERYLOW, "~CPSPEventQ(): Start");
	Clear();
	if (m_RcvBlocker)
	{
		delete m_RcvBlocker; m_RcvBlocker = NULL;
	}
	if (m_RcvOKBlocker)
	{
		delete m_RcvOKBlocker; m_RcvOKBlocker = NULL;
	}
	if(m_lock)
	{
		delete m_lock; m_lock = NULL;
	}
	free(m_strName), m_strName = NULL;
	Log (LOG_VERYLOW, "~CPSPEventQ(): Done.");
}

int CPSPEventQ::Send(QEvent &Event)
{
	//Log(LOG_VERYLOW, "Send(): Calling Lock(mid=%x)", Event.EventId);
	m_lock->Lock();

	if (Size() > 512)
	{
		char *pData = (char*)Event.pData;
		Log(LOG_ERROR,"Send.. queue filling up... SenderId=0x%x, EventId=0x%x", Event.SenderId, Event.EventId);
		if (pData != NULL)
		{
			Log(LOG_ERROR,"Send.. queue filling up... Data=0x%x '%c%c%c%c'...", 
				pData, pData[0], pData[1], pData[2], pData[3]);
		}
	}
	
	//Log(LOG_VERYLOW, "Send(): Calling Push_Back(mid=%x)", Event.EventId);
	m_EventList.push_back(Event);
	
	//notify receive by unblocking it
	m_RcvBlocker->UnBlock();
	
	m_lock->Unlock();

	
	return 0;
}

int CPSPEventQ::SendAndWaitForOK(QEvent &Event)
{
	m_lock->Lock();
	
	m_EventList.push_back(Event);
	
	//notify receive
	m_RcvBlocker->UnBlock();
	
	//now, wait for OK.
	m_RcvOKBlocker->Block();
	
	m_lock->Unlock();
	
	return 0;
}

int CPSPEventQ::Receive(QEvent &Event)
{
	//block until notified
	m_RcvBlocker->Block();

	if (Size() > 0)
	{
		Event = m_EventList.front();
		m_EventList.pop_front();
	}
	else
	{
		Log(LOG_ERROR, "Receive('%s') Error. Receieve unblocked, but no messages in the queue?! size=%d.",
			m_strName, Size());
	}
	
	/** The blocker is ready to block again as soon as it unblocks, so if there's more messages waiting, we need to 
	tell it to unblock now...*/	
	if (Size() > 0) /** Clear only if last message */
	{
		m_RcvBlocker->UnBlock();
	}
	return 0;
}

int CPSPEventQ::SendReceiveOK()
{
	m_RcvOKBlocker->UnBlock();
	return 0;
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
