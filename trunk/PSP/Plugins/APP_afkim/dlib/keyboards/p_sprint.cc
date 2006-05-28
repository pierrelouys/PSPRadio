/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * main.c - p-sprint
 *
 * Copyright (c) 2005 Arwin van Arum
 * special thanks to Shine for leading the way
 *
 * $Id$
 */
#include "pspctrl_emu.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* modifier state bits */
#define P_SP_MOD_NONE 0
#define P_SP_MOD_CONTROL 1
#define P_SP_MOD_SHIFT 2
#define P_SP_MOD_ALT 4

//reserved for future implementation:
//#define P_SP_MOD_SPECIAL 16

/* key group constants */
#define P_SP_KEYGROUP_DEFAULT 0
#define P_SP_KEYGROUP_NUMFN 1 
#define P_SP_KEYGROUP_CONTROL 2 

//reserved for future implementation:
//#define P_SP_KEYGROUP_CUSTOM1 3 
//#define P_SP_KEYGROUP_CUSTOM2 4 
//#define P_SP_KEYGROUP_CUSTOM3 5 


struct p_sp_Key
{
	int keyid;
	int keychar;
	int keycode;
	int modifiers;
	int keygroup;
};

int KeyCodes[64][5];
char KeyChars[255][4];

/* new globals for the non-blocking p_sp_readKey function */
unsigned int g_prev_btnstate = 0;
unsigned int g_prev_btn = 0;
//unsigned int g_prev_filt_btnstate = 0;
int g_shift_state = 0;
int g_active_group = 0;
struct p_sp_Key g_prev_Key;
int g_prevkey_set = 0;
int g_keyrep_rate = 2;
int g_keyrep_first = 16;
int g_keyrep_first_hold = 16;
int g_keyrep_zsf = 0;
int g_keyrep_counter = 0;
int g_init_done = 0;
int g_iButton = 0;

int setCustomKeyRepeat(int Rate, int Hold)
{
	g_keyrep_rate = Rate;
	g_keyrep_first = Hold;
	g_keyrep_first_hold = Hold;
	return 1;
}

int p_spSetupKeyCodes(void)
{
	/* links unique keys or key combinations that p-sprint recognises
	to PC compatible keycodes */

	int i = 0;
	int j = 0;
	while(i<5)
	{
		while(j<65)
		{
			KeyCodes[j][i]=0;
			j++;
		}
		i++;
	}

	/* default group = 0 */

	KeyCodes[1][0]=8;
	KeyCodes[2][0]=32;

	KeyCodes[3][0]=98-32;
	KeyCodes[4][0]=121-32;
	KeyCodes[5][0]=103-32;
	KeyCodes[6][0]=219;
	KeyCodes[7][0]=222;
	KeyCodes[8][0]=188;

	KeyCodes[9][0]=102-32;
	KeyCodes[10][0]=111-32;
	KeyCodes[11][0]=117-32;
	KeyCodes[12][0]=120-32;
	KeyCodes[13][0]=118-32;
	KeyCodes[14][0]=192;

	KeyCodes[15][0]=106-32;
	KeyCodes[16][0]=108-32;
	KeyCodes[17][0]=100-32;
	KeyCodes[18][0]=109-32;
	KeyCodes[19][0]=122-32;
	KeyCodes[20][0]=221;

	KeyCodes[21][0]=220;
	KeyCodes[22][0]=113-32;
	KeyCodes[23][0]=112-32;
	KeyCodes[24][0]=115-32;
	KeyCodes[25][0]=116-32;
	KeyCodes[26][0]=99-32;

	KeyCodes[27][0]=186;
	KeyCodes[28][0]=119-32;
	KeyCodes[29][0]=107-32;
	KeyCodes[30][0]=114-32;
	KeyCodes[31][0]=101-32;
	KeyCodes[32][0]=97-32;

	KeyCodes[33][0]=190;
	KeyCodes[34][0]=189;
	KeyCodes[35][0]=191;
	KeyCodes[36][0]=104-32;
	KeyCodes[37][0]=110-32;
	KeyCodes[38][0]=105-32;
	KeyCodes[39][0]=13;
	KeyCodes[40][0]=27;
	KeyCodes[41][0]=20;
	KeyCodes[42][0]=93;

	/* group = 1 */

	KeyCodes[1][1]=8;
	KeyCodes[2][1]=32;

	KeyCodes[3][1]=112;
	KeyCodes[4][1]=113;
	KeyCodes[5][1]=119;
	KeyCodes[6][1]=219;
	KeyCodes[7][1]=186;
	KeyCodes[8][1]=188;

	KeyCodes[9][1]=114;
	KeyCodes[10][1]=115;
	KeyCodes[11][1]=116;
	KeyCodes[12][1]=111;
	KeyCodes[13][1]=123;
	KeyCodes[14][1]=192;

	KeyCodes[15][1]=120;
	KeyCodes[16][1]=117;
	KeyCodes[17][1]=118;
	KeyCodes[18][1]=121;
	KeyCodes[19][1]=122;
	KeyCodes[20][1]=221;

	//KeyCodes[21][1]=55;
	KeyCodes[22][1]=106;
	KeyCodes[23][1]=48;
	KeyCodes[24][1]=49;
	KeyCodes[25][1]=50;
	KeyCodes[26][1]=57;

	KeyCodes[27][1]=186;
	KeyCodes[28][1]=107;
	KeyCodes[29][1]=109;
	KeyCodes[30][1]=51;
	KeyCodes[31][1]=52;
	KeyCodes[32][1]=53;

	KeyCodes[33][1]=190;
	KeyCodes[34][1]=189;
	//KeyCodes[35][1]=55;
	KeyCodes[36][1]=56;
	KeyCodes[37][1]=54;
	KeyCodes[38][1]=55;
	KeyCodes[39][1]=13;
	KeyCodes[40][1]=27;
	/* shortcuts, psp only */
	KeyCodes[41][1]=20;
	KeyCodes[42][1]=93;

	/* group = 2 */

	KeyCodes[1][2]=40;
	KeyCodes[2][2]=13;

	KeyCodes[3][2]=37;
	KeyCodes[4][2]=36;
	KeyCodes[5][2]=45;
	//KeyCodes[6][2]=38;
	//KeyCodes[7][2]=222;
	//KeyCodes[8][2]=188;

	KeyCodes[9][2]=35;
	KeyCodes[10][2]=38;
	KeyCodes[11][2]=33;
	KeyCodes[12][2]=19;
	KeyCodes[13][2]=16;
	//KeyCodes[14][2]=192;

	KeyCodes[15][2]=46;
	KeyCodes[16][2]=34;
	KeyCodes[17][2]=39;
	KeyCodes[18][2]=107;
	KeyCodes[19][2]=106;
	//KeyCodes[20][2]=221;

	//KeyCodes[21][2]=55;
	KeyCodes[22][2]=145;
	KeyCodes[23][2]=109;
	KeyCodes[24][2]=27;
	KeyCodes[25][2]=91;
	KeyCodes[26][2]=93;

	//KeyCodes[27][2]=186;
	KeyCodes[28][2]=144;
	//KeyCodes[29][2]=82;
	KeyCodes[30][2]=17;
	KeyCodes[31][2]=16;
	KeyCodes[32][2]=18;

	//KeyCodes[33][2]=190;
	//KeyCodes[34][2]=189;
	//KeyCodes[35][2]=80;
	KeyCodes[36][2]=9;
	KeyCodes[37][2]=92;
	KeyCodes[38][2]=20;
	KeyCodes[39][2]=13;
	KeyCodes[40][2]=27;
	/* shortcuts, psp only */
	KeyCodes[41][2]=20;
	KeyCodes[42][2]=8;

	return 0;
}

int p_spGetKeycodeFriendlyName(int keyCode, char keyName[20])
{
	/* returns a friendly name for non-character keycodes */

	switch(keyCode)
	{
	case 8:
		strcpy(keyName, "Backspace");
		break;
	case 32:
		strcpy(keyName, "Space");
		break;
	case 112:
		strcpy(keyName, "F1");
		break;
	case 113:
		strcpy(keyName, "F2");
		break;
	case 114:
		strcpy(keyName,"F3");
		break;
	case 115:
		strcpy(keyName,"F4");
		break;
	case 116:
		strcpy(keyName,"F5");
		break;
	case 117:
		strcpy(keyName,"F6");
		break;
	case 118:
		strcpy(keyName,"F7");
		break;
	case 119:
		strcpy(keyName,"F8");
		break;
	case 120:
		strcpy(keyName,"F9");
		break;
	case 121:
		strcpy(keyName,"F10");
		break;
	case 122:
		strcpy(keyName,"F11");
		break;
	case 123:
		strcpy(keyName,"F12");
		break;
	case 9:
		strcpy(keyName,"TAB");
		break;
	case 27:
		strcpy(keyName,"ESC");
		break;
	case 20:
		strcpy(keyName,"CAPS");
		break;
	case 16:
		strcpy(keyName,"SHIFT");
		break;
	case 17:
		strcpy(keyName,"CTRL");
		break;
	case 18:
		strcpy(keyName,"ALT");
		break;
	case 91:
		strcpy(keyName,"L_OS");
		break;
	case 92:
		strcpy(keyName,"R_OS");
		break;
	case 13:
		strcpy(keyName,"Enter");
		break;
	case 37:
		strcpy(keyName,"Left");
		break;
	case 38:
		strcpy(keyName,"Up");
		break;
	case 39:
		strcpy(keyName,"Right");
		break;
	case 40:
		strcpy(keyName,"Down");
		break;
	case 45:
		strcpy(keyName,"Insert");
		break;
	case 46:
		strcpy(keyName,"Delete");
		break;
	case 36:
		strcpy(keyName,"Home");
		break;
	case 35:
		strcpy(keyName,"End");
		break;
	case 33:
		strcpy(keyName,"Page Up");
		break;
	case 34:
		strcpy(keyName,"Page Down");
		break;
	case 44:
		strcpy(keyName,"SysRq");
		break;
	case 144:
		strcpy(keyName,"Num Lock");
		break;
	case 145:
		strcpy(keyName,"Scroll Lk");
		break;
	case 19:
		strcpy(keyName,"Break");
		break;
	case 106:
		strcpy(keyName,"* (NUM)");
		break;
	case 107:
		strcpy(keyName,"+ (NUM)");
		break;
	case 111:
		strcpy(keyName,"/ (NUM)");
		break;
	case 109:
		strcpy(keyName,"- (NUM)");
		break;
	//case else
	//{
	//	strcpy("");
	//	return 0;
	//}
	}
	//strcpy(friendlyName,keyName);
	//printf("%s %d", keyName,keyCode);
	return 1;
}


int p_spSetupKeyChars(void)
{
	/* links PC compatible keycodes to PC compatible character values */
	
	int i = 0;
	int j = 0;
	while(i<4)
	{
		while(j<256)
		{
			KeyChars[j][i]=0;
			j++;
		}
		i++;
	}

	/* default group = 0 */

	KeyChars[8][0]=8;
	KeyChars[13][0]=13;
	KeyChars[27][0]=27;
	KeyChars[32][0]=32;
	KeyChars[83][2]=83;
	KeyChars[83][0]=83+32;
	KeyChars[84][2]=84;
	KeyChars[84][0]=84+32;
	KeyChars[67][2]=67;
	KeyChars[67][0]=67+32;
	KeyChars[219][2]=123;
	KeyChars[219][0]=91;
	KeyChars[222][2]=34;
	KeyChars[222][0]=39;
	KeyChars[188][2]=60;
	KeyChars[188][0]=44;

	KeyChars[82][2]=82;
	KeyChars[82][0]=82+32;
	KeyChars[69][2]=69;
	KeyChars[69][0]=69+32;
	KeyChars[65][2]=65;
	KeyChars[65][0]=65+32;
	KeyChars[88][2]=88;
	KeyChars[88][0]=88+32;
	KeyChars[86][2]=86;
	KeyChars[86][0]=86+32;
	KeyChars[192][2]=126;
	KeyChars[192][0]=96;

	KeyChars[72][2]=72;
	KeyChars[72][0]=72+32;
	KeyChars[78][2]=78;
	KeyChars[78][0]=78+32;
	KeyChars[73][2]=73;
	KeyChars[73][0]=73+32;
	KeyChars[77][2]=77;
	KeyChars[77][0]=77+32;
	KeyChars[90][2]=90;
	KeyChars[90][1]=26;
	KeyChars[90][0]=90+32;
	KeyChars[221][2]=125;
	KeyChars[221][0]=93;

	KeyChars[220][2]=124;
	KeyChars[220][0]=92;
	KeyChars[81][2]=81;
	KeyChars[81][0]=81+32;
	KeyChars[80][2]=80;
	KeyChars[80][0]=80+32;
	KeyChars[66][2]=66;
	KeyChars[66][0]=66+32;
	KeyChars[89][2]=89;
	KeyChars[89][0]=89+32;
	KeyChars[71][2]=71;
	KeyChars[71][0]=71+32;

	KeyChars[186][2]=58;
	KeyChars[186][0]=59;
	KeyChars[87][2]=87;
	KeyChars[87][0]=87+32;
	KeyChars[75][2]=75;
	KeyChars[75][0]=75+32;
	KeyChars[70][2]=70;
	KeyChars[70][0]=70+32;
	KeyChars[79][2]=79;	
	KeyChars[79][0]=79+32;
	KeyChars[85][2]=85;
	KeyChars[85][0]=85+32;

	KeyChars[190][2]=62;
	KeyChars[190][0]=46;
	KeyChars[189][2]=95;
	KeyChars[189][0]=45;
	KeyChars[191][2]=63;
	KeyChars[191][0]=47;
	KeyChars[74][2]=74;
	KeyChars[74][0]=106;
	KeyChars[76][2]=76;
	KeyChars[76][0]=76+32;
	KeyChars[68][2]=68;
	KeyChars[68][0]=68+32;

	/* numbers */
	KeyChars[48][2]=41;
	KeyChars[48][0]=48;
	KeyChars[49][2]=33;
	KeyChars[49][0]=49;
	KeyChars[50][2]=64;
	KeyChars[50][0]=50;
	KeyChars[51][2]=35;
	KeyChars[51][0]=51;
	KeyChars[52][2]=36;
	KeyChars[52][0]=52;
	KeyChars[53][2]=37;
	KeyChars[53][0]=53;
	KeyChars[54][2]=94;
	KeyChars[54][0]=54;
	KeyChars[55][2]=38;
	KeyChars[55][0]=55;
	KeyChars[56][2]=42;
	KeyChars[56][0]=56;
	KeyChars[57][2]=40;
	KeyChars[57][0]=57;

	/* numpad */
	//KeyChars[107][0]=43;
	//KeyChars[106][0]=42;
	//KeyChars[109][0]=45;
	//KeyChars[111][0]=47;


	/* Function */

	/* control */

	return 0;
}


int p_spGetControlKeys(unsigned int butpress1, unsigned int butpress2)
{
	/* determines whether or not a two-button combination
	contains a shift option and returns the shift option
	a.k.a. modifiers if true */

	/* to do: allow for combinations of control keys */

	unsigned int mainbut;
	unsigned int shiftbut;
	int shift = 0;
	int control = 0;
	int alt = 0;
	//int num = 0;
	int special = 0;
	
	mainbut = butpress1 & butpress2;
	
	if ((!(mainbut==butpress2))&(!(butpress2==0)))
	/* contains shift-option */
	{
		/* TO DO: parse multiple button combos */
		/* get shift value */
		shiftbut = butpress2 ^ butpress1;
		switch (shiftbut)
		{
		case PSP_CTRL_DOWN:
			special=8;
			break;
		case PSP_CTRL_CROSS:
			special=8;
			break;

		case PSP_CTRL_RIGHT:
			alt=4;
			break;
		case PSP_CTRL_CIRCLE:
			alt=4;
			break;
		case PSP_CTRL_SQUARE:
			control=1;
			break;
		case PSP_CTRL_LEFT:
			control=1;
			break;
		case PSP_CTRL_TRIANGLE:
			shift=2;
			break;
		case PSP_CTRL_UP:
			shift=2;
			break;
		}
	}
	return (alt+control+shift+special+g_shift_state);
}

int p_spGetKeyId(unsigned int butpress1, unsigned int butpress2)
{
	/* returns a unique id for a p-sprint supported button
	or button combination */
	int i = 0;
	int keyIds[7];
	for(i=0;i<7;i++)
	{
		keyIds[i]=0;
	}
	switch(butpress1)
	{
	case PSP_CTRL_LTRIGGER:		
		return 41;
		break;
	case PSP_CTRL_RTRIGGER:		
		return 42;
		break;
	case PSP_CTRL_START:
		return 39;
		break;
	case PSP_CTRL_SELECT:
		return 40;
		break;
	case PSP_CTRL_DOWN:
		return 1;
		break;
	case PSP_CTRL_CROSS:
		return 2;
		break;
	case PSP_CTRL_LEFT:
		keyIds[0]=3;
		keyIds[1]=4;
		keyIds[2]=5;
		keyIds[3]=6;
		keyIds[4]=7;
		keyIds[5]=8;
		keyIds[6]=63;
		break;
	case PSP_CTRL_UP:
		keyIds[0]=9;
		keyIds[1]=10;
		keyIds[2]=11;
		keyIds[3]=12;
		keyIds[4]=13;
		keyIds[5]=14;
		keyIds[6]=62;
		break;
	case PSP_CTRL_RIGHT:
		keyIds[0]=15;
		keyIds[1]=16;
		keyIds[2]=17;
		keyIds[3]=18;
		keyIds[4]=19;
		keyIds[5]=20;
		keyIds[6]=61;
		break;
	case PSP_CTRL_SQUARE:
		keyIds[0]=21;
		keyIds[1]=22;
		keyIds[2]=23;
		keyIds[3]=24;
		keyIds[4]=25;
		keyIds[5]=26;
		keyIds[6]=60;
		break;
	case PSP_CTRL_TRIANGLE:
		keyIds[0]=27;
		keyIds[1]=28;
		keyIds[2]=29;
		keyIds[3]=30;
		keyIds[4]=31;
		keyIds[5]=32;
		keyIds[6]=59;
		break;
	case PSP_CTRL_CIRCLE:
		keyIds[0]=33;
		keyIds[1]=34;
		keyIds[2]=35;
		keyIds[3]=36;
		keyIds[4]=37;
		keyIds[5]=38;
		keyIds[6]=58;
		break;
	}

	switch(butpress2)
	{
	case PSP_CTRL_DOWN:
		return keyIds[6];
		break;
	case PSP_CTRL_CROSS:
		return keyIds[6];
		break;
	case PSP_CTRL_LEFT:
		return keyIds[0];
		break;
	case PSP_CTRL_UP:
		return keyIds[1];
		break;
	case PSP_CTRL_RIGHT:
		return keyIds[2];
		break;
	case PSP_CTRL_SQUARE:
		return keyIds[3];
		break;
	case PSP_CTRL_TRIANGLE:
		return keyIds[4];
		break;
	case PSP_CTRL_CIRCLE:
		return keyIds[5];
		break;
	}
	return 0;
}



int p_spGroupSelect(unsigned int butpress1, unsigned int butpress2)
{
	/* looks if a two button combo renders a select group 
	code and sets the group code if a valid one is found*/

	int keyId;
	keyId = p_spGetKeyId(butpress1,butpress2);
	switch(keyId)
	{
	case 63:
		if(g_active_group==1)
		{
			g_active_group=P_SP_KEYGROUP_DEFAULT;
		}
		else
		{
			g_active_group=1;
		}
		return 1;
		break;
	case 62:
		if(g_active_group==2)
		{
			g_active_group=P_SP_KEYGROUP_DEFAULT;
		}
		else
		{
			g_active_group=2;
		}
		return 1;
		break;
	case 61:
		if(g_active_group==3)
		{
			g_active_group=P_SP_KEYGROUP_DEFAULT;
		}
		else
		{
			g_active_group=3;
		}
		return 1;

		break;
	}
	return 0;
}



int p_sp_init(void)
{	
	/* init data structures */
	p_spSetupKeyCodes();
	p_spSetupKeyChars();



	g_init_done = 1;
	return 1;
}
int p_spReadKeyEx(struct p_sp_Key * newKey, unsigned int Buttons)
{
	/* main p_sprint function: 

	non-blocking routine that sets a key structure containing
	a keyid, PC compatible keyCode, PC compatile keyChar, 
	PC compatible modifier state (shift,control, alt) and
	the key group that was selected by the user.

	Function returns true when a key is found, else false. */

	int keyId = 0;
	int keyCode = 0;
	int keyChar = 0;
	int keyFound = 0;
	unsigned int cur_btnstate = 0;
	
	if(!g_init_done)
	{
		p_sp_init();
	}

	//SceCtrlData pad;
	//sceCtrlReadBufferPositive(&pad, 1);
	cur_btnstate = Buttons;
	if(cur_btnstate==0)
	{
		/* reset certain variables,
		set flag that zero state has been found */
		g_keyrep_zsf = 1;
		
		g_keyrep_first_hold = g_keyrep_first;
		g_keyrep_counter = 0;

	}
	/* a zero button state never repeats, and for a repeat
	obviously the current and previous buttons need to be
	identical */
	
	if(cur_btnstate==g_prev_btnstate)
	{
		if(!g_keyrep_zsf)
		{
			//if(g_prevkey_set)
			//{
				g_keyrep_counter++;
				if(g_keyrep_counter>(g_keyrep_first_hold+g_keyrep_rate))
				{
					/* clear repeat threshhold */
					g_keyrep_first_hold=0;
					g_keyrep_counter = 0;
					keyId = g_prev_Key.keyid;
					keyCode = g_prev_Key.keycode;
					keyChar = g_prev_Key.keychar;
					g_active_group = g_prev_Key.keygroup;
					g_shift_state = g_prev_Key.modifiers;
					keyFound = 1;
				}
			//}
		}
	}
	else
	{
		/* handle button state-change */
		if(cur_btnstate)
		{
			if(g_prev_btnstate)
			{
				//if(!g_shift_state)
				//{
					g_shift_state = p_spGetControlKeys(g_prev_btnstate,cur_btnstate);
					if(g_shift_state)
					{
						/* a key combo is found, check if it is a group select special */
						if(p_spGroupSelect(g_prev_btnstate,(cur_btnstate ^ g_prev_btnstate) & cur_btnstate))
						{
							/* a group select special, so no regular shift value */
							g_shift_state = 0;
							g_prev_btn = 0;
							//keyId = 64-g_active_group;
						}
						
					}
				//}
				//else
				//{
				//	/* TO DO: Handle combined shift_states */

				//}
			}
			else
			{
				if(g_prev_btn)
				{
					/* found 2nd button press, get keyCode */
					keyId = p_spGetKeyId(g_prev_btn, cur_btnstate);
					keyCode = KeyCodes[keyId][g_active_group];
					keyChar = KeyChars[keyCode][g_shift_state];
					keyFound = 1;
				}
				else
				{
					/* found first button press */
					/* check if buttonstate is a single-key that returns a value
					(e.g. 'x' for space or 'down' for backspace) */
					
					keyId = p_spGetKeyId(cur_btnstate,0);
					if(keyId)
					{
						keyFound = 1;		
						keyCode = KeyCodes[keyId][g_active_group];
						keyChar = KeyChars[keyCode][g_shift_state];
					}
					else
					{
						/* store button press in buffer */
						g_prev_btn = cur_btnstate;
						keyFound = 0;
						
					}
				}

			}
		}
		else
		{
			/* ignore button zero state */

			/* reset first keyrepeat threshold */
			g_keyrep_first_hold = g_keyrep_first;
			//g_prev_btnstate = 0;
			keyFound = 0;
		}
	}
	(*newKey).keyid = keyId;
	(*newKey).keycode = keyCode;
	(*newKey).keychar = keyChar;
	(*newKey).keygroup = g_active_group;
	(*newKey).modifiers = g_shift_state;

	g_prev_btnstate = cur_btnstate;

	if(keyFound)
	{
		g_prevkey_set = 1;
		g_keyrep_zsf = 0;

		g_prev_Key.keyid = keyId;
		g_prev_Key.keycode = keyCode;
		g_prev_Key.keychar = keyChar;
		g_prev_Key.keygroup = g_active_group;
		g_prev_Key.modifiers = g_shift_state;
		g_prev_btn = 0;
		g_shift_state = 0;
		return 1;
	}
	else
	{
		g_prevkey_set = 0;
		return 0;
	}
}

char p_spgetKeyCodeFromKeyId(int keyId, int keyGroup)
{
	return KeyCodes[keyId][keyGroup];
}

char p_spgetKeyCharFromKeyCode(int keyCode, int modifiers)
{
	return KeyChars[keyCode][modifiers];
}


int p_spSetActiveGroup(int i_active_group)
{
	/* allows you to programmatically set the key group */
	g_active_group = i_active_group;
	return 1;
}

int p_sp_GetActiveGroup()
{
	return g_active_group;
}


