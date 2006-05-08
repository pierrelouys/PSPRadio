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

#include "PSPKeyHandler.h"
#include <time.h>
#include <psprtc.h>

CPSPKeyHandler::~CPSPKeyHandler()
{
}

CPSPKeyHandler::CPSPKeyHandler()
{
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

	m_stored_keystate	= 0;
	m_tick				= 0;
	m_tick_trigger		= 0;
	m_long_press		= 0;
	m_repeat_delay		= (250 * 1000);
}

bool CPSPKeyHandler::KeyHandler(CPSPKeyHandler::KeyEvent &event)
{
	bool	ret_value;

	// Read current state of the key pad
	//memset(&m_latch, 0, sizeof(m_latch));
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
		/** Ignore the following buttons for EVENT_UPDATE */
		m_latch.uiPress = m_latch.uiPress & ~PSP_CTRL_HOLD;
		m_latch.uiPress = m_latch.uiPress & ~PSP_CTRL_HOME; /* This happens by itself when the screen turns off */

		if (0 != m_latch.uiPress)
		{
			ret_value = KeyHandlerSTM(EVENT_UPDATE, event);
		}
		else
		{
			ret_value = false;
		}
	}

	return ret_value;
}

void CPSPKeyHandler::KeyHandler_Repeat(u32 delay)
{
	m_repeat_delay		= (delay * 1000);
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
					event.event		= KEY_EVENT_PRESSED;
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
					event.event		= KEY_EVENT_PRESSED;
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
						event.event	= KEY_EVENT_LONG_PRESS;
						}
					else
						{
						event.event	= KEY_EVENT_RELEASED;
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
						sceRtcTickAddMicroseconds(&m_tick_trigger, &m_tick, m_repeat_delay);
						event.event		= KEY_EVENT_REPEAT;
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
	sceRtcTickAddMicroseconds(&m_tick_trigger, &m_tick, m_repeat_delay);
}
