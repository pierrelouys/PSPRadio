/* 
	PSP Keypad handler.
	Copyright (C) 2005  Rafael Cabezas a.k.a. Raf & Jesper Sandberg
	
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

#include <time.h>
#include <pspkerneltypes.h>
#include <pspctrl.h>
#include <psprtc.h>
#include "PSPKeyHandler.h"
#include "PSPApp.h"

CPSPKeyHandler::~CPSPKeyHandler()
{
}

CPSPKeyHandler::CPSPKeyHandler()
{
	m_stored_keystate	= 0;
	m_tick				= 0;
	m_tick_trigger		= 0;
	m_long_press		= 0;
}

bool CPSPKeyHandler::KeyHandler(CPSPKeyHandler::KeyEvent &event)
{
	bool	ret_value;

	// Read current state of the key pad
	sceCtrlReadLatch(&m_latch);

	if (m_latch.uiMake)
	{
		ret_value = KeyHandlerSTM(EVENT_PRESSED, event);
	}
	else if (m_latch.uiBreak)
	{
		ret_value = KeyHandlerSTM(EVENT_RELEASED, event);
	}
	else
	{
		ret_value = KeyHandlerSTM(EVENT_UPDATE, event);
	}
	
	#if 0
	SceCtrlData	s_PadData;
	sceCtrlPeekBufferPositive( &s_PadData, 1 );
	if (s_PadData.Buttons & PSP_CTRL_HOME)
	{
		Log(LOG_ERROR, "HOME PRESSED!!!");
		m_latch.uiPress |= PSP_CTRL_HOME;
		ret_value = KeyHandlerSTM(EVENT_RELEASED, event);
	}
	#endif

	return ret_value;
}

bool CPSPKeyHandler::KeyHandlerSTM(STM_EVENT key_event, CPSPKeyHandler::KeyEvent &event)
{
	static	STM_STATE	stm_var = STATE_IDLE;
	bool				ret_value = false;

	// Simple STM to handle key events
	switch (stm_var)
	{
		// This state is when no key is pressed
		case	STATE_IDLE:
		{
			switch (key_event)
			{
				case	EVENT_PRESSED:
				{
					KeyHandlerStore();
					event.event		= MID_ONBUTTON_PRESSED;
					event.key_state	= m_stored_keystate;
					ret_value = true;

					// Transition to STATE_PRESSED state
					stm_var = STATE_PRESSED;
				}
				break;

				case	EVENT_RELEASED:
				case	EVENT_UPDATE:
				{
					// Nothing
				}
				break;
			}
		}
		break;

		// This state is when a key is pressed
		case	STATE_PRESSED:
		{

			switch (key_event)
			{
				case	EVENT_PRESSED:
				{
					KeyHandlerStore();
					event.event		= MID_ONBUTTON_PRESSED;
					event.key_state	= m_stored_keystate;
					ret_value = true;
				}
				break;

				case	EVENT_RELEASED:
				{
					m_tick			= 0;
					m_tick_trigger	= 0;

					if (m_long_press >= KEYHANDLER_LONG_PRESS)
						{
						event.event	= MID_ONBUTTON_LONG_PRESS;
						}
					else
						{
						event.event	= MID_ONBUTTON_RELEASED;
						}
					event.key_state	= m_stored_keystate;
					ret_value = true;

					// Transition to STATE_IDLE state
					stm_var = STATE_IDLE;
				}
				break;

				case	EVENT_UPDATE:
				{
					// Get current tick count
					sceRtcGetCurrentTick(&m_tick);

					// If we have passed the trigger value, reset the trigger and send event
					if (m_tick >= m_tick_trigger)
					{
						m_long_press++;
						// Find the time for the next "repeat event"
						sceRtcTickAddMicroseconds(&m_tick_trigger, &m_tick, KEYHANDLER_REPEAT_INTERVAL);
						event.event		= MID_ONBUTTON_REPEAT;
						event.key_state	= m_stored_keystate;
						ret_value = true;
					}
				}
				break;
			}
		}
		break;
	}

	return ret_value;
}

void CPSPKeyHandler::KeyHandlerStore()
{
	// Store latch with keypresses
	m_stored_keystate	= m_latch.uiPress;
	m_long_press		= 0x00;

	// Get Tick count when key was pressed
	sceRtcGetCurrentTick(&m_tick);
	// Find the time for the next "repeat event"
	sceRtcTickAddMicroseconds(&m_tick_trigger, &m_tick, KEYHANDLER_REPEAT_INTERVAL);
}
