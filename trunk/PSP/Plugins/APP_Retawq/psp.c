#include <pspsdk.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#ifdef USE_DANZEFF
#include <pspgu.h>
#endif /* USE_DANZEFF */
#include "psp.h"
#include <PSPRadio_Exports.h>
#include <danzeff.h>

#define printf pspDebugScreenPrintf

#include "pspcurses.h"

#define PSP_SCREEN_WIDTH 480
#define PSP_SCREEN_HEIGHT 272

volatile tBoolean g_PSPEnableRendering = truE;
volatile tBoolean g_PSPEnableInput = truE;
tBoolean g_InputMethod = falsE;

#ifdef USE_DANZEFF
static unsigned int __attribute__((aligned(16))) gu_list[8192];
static unsigned int __attribute__((aligned(16))) gu_backup[4*PSP_SCREEN_WIDTH*PSP_SCREEN_HEIGHT];
static int first_time = 1;
#endif /* USE_DANZEFF */

void PSPPutch(char ch)
{
	if (g_PSPEnableRendering == truE)
	{
		printf("%c", ch);
	}
}

int env_termsize(int *x, int *y)
{
	*x = (PSP_SCREEN_WIDTH / 7) - 1;
	*y = (PSP_SCREEN_HEIGHT / 8);

	return 1;
}

#ifdef USE_DANZEFF
void PSPInputRender(void)
{
	if (first_time)
		{
		sceGuInit();
		sceGuDisplay(GU_TRUE);
		}

	sceGuStart(GU_DIRECT,gu_list);
	sceGuDrawBuffer(GU_PSM_8888, (void*) 0, 512);
	sceGuDispBuffer(PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT, (void*) 0, 512);

	/* First time, init GU and copy the screen containing the text */
	if (first_time)
		{
		/* Init GU */
		sceGuOffset(2048 - (PSP_SCREEN_WIDTH/2),2048 - (PSP_SCREEN_HEIGHT/2));
		sceGuViewport(2048,2048,PSP_SCREEN_WIDTH,PSP_SCREEN_HEIGHT);
		sceGuScissor(0,0,PSP_SCREEN_WIDTH,PSP_SCREEN_HEIGHT);
		sceGuEnable(GU_SCISSOR_TEST);
		sceGuDisable(GU_DEPTH_TEST);
		sceGuDisable(GU_CULL_FACE);

		/* Make textscreen backup to use as texture */
		sceGuCopyImage(GU_PSM_8888, 0, 0, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT, PSP_SCREEN_WIDTH, (void*) 0x4000000, 0, 0, PSP_SCREEN_WIDTH, (void*) gu_backup);
		first_time = 0;
		}

	sceGuCopyImage(GU_PSM_8888, 0, 0, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT, PSP_SCREEN_WIDTH, (void*) gu_backup, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT, 512, (void*) 0x4000000);
	sceGuTexSync();
	danzeff_render();
	sceGuFinish();
	sceGuSync(0,0);
}

void ClearInputHandlerWindow()
{
}

void DrawInputHandlerWindow()
{
	if (danzeff_dirty())
	{
		PSPInputRender();
	}
}

void PSPInputHandlerStart()
{
	static int danzeff_x = PSP_SCREEN_WIDTH/2-(64*3/2), danzeff_y = PSP_SCREEN_HEIGHT/2-(64*3/2);

	danzeff_load_lite();
	danzeff_moveTo(danzeff_x, danzeff_y);
	if (danzeff_isinitialized())
	{
		//danzef_set_screen(sdl_SURFACE(dev));
		//danzeff_moveTo(danzeff_x, danzeff_y);
		PSPInputRender();
	}
	else
	{
		printf("Error loading danzeff OSK");
	}
}

void PSPInputHandlerEnd()
{
	danzeff_free();
	first_time = 1;
}


void PSPInputHandler_DisplayButtons()
{
	if (danzeff_dirty())
	{
		PSPInputRender();
	}
}

int PSPInputHandler(SceCtrlData pad, char *key)
{
	*key = danzeff_readInput(pad);
	if (*key)
	{
		switch (*key)
		{
			case '\n':
				*key = KEY_ENTER;
				break;
			case 8:
				*key = KEY_BACKSPACE;
				break;
		}
		PSPInputRender();
		return 1;
	}
	PSPInputRender();
	return 0;

}

#endif

#ifdef USE_FORMER_INPUT_METHOD
typedef enum
{
	PSPIHM_NORMAL_1,
	PSPIHM_NORMAL_2,
	PSPIHM_NORMAL_3,
	PSPIHM_NORMAL_4,

	PSPIHM_MAX

} pspih_modes;

pspih_modes g_Mode;
int g_Modif = 0;
tBoolean g_InputMethod = falsE;

static char InputTable[PSPIHM_MAX][1][16];

void PSPInputHandler_DisplayButtons();

void ClearInputHandlerWindow()
{
	int i;
	for (i = 10; i <= 26; i++)
	{
		pspDebugScreenSetXY(14,i);
		pspDebugScreenPrintf("                                                     ");
	}
}

void DrawInputHandlerWindow()
{
	int i;
	ClearInputHandlerWindow();
	pspDebugScreenSetXY(10,11);
	pspDebugScreenPrintf    ("+--------------------------------------+");
	for (i = 12; i <= 25; i++)
	{
		pspDebugScreenSetXY(10,i);
		pspDebugScreenPrintf("|                                      |");
	}
	pspDebugScreenSetXY(10,26);
	pspDebugScreenPrintf    ("+--------------------------------------+");



}

void PSPInputHandlerStart()
{
	char string[17];
	sprintf(string, "QWERT. ,YUIOP@%c_", KEY_BACKSPACE);
	memcpy(InputTable[PSPIHM_NORMAL_1][0], string, 16);

	sprintf(string, "ASDFG*(-HJKL;+)-");
	memcpy(InputTable[PSPIHM_NORMAL_2][0], string, 16);

	sprintf(string, "ZXCVB=:;NM\"'?</>");
	memcpy(InputTable[PSPIHM_NORMAL_3][0], string, 16);

	sprintf(string, "01234!#$56789%%^&");
	memcpy(InputTable[PSPIHM_NORMAL_4][0], string, 16);

	g_Mode = PSPIHM_NORMAL_1;

	DrawInputHandlerWindow();

	PSPInputHandler_DisplayButtons();
						/*12345678901234567890*/
	pspDebugScreenSetXY(16,12);
	pspDebugScreenPrintf("INPUT MODE (R+START To Exit)");

}

void PSPInputHandlerEnd()
{
	ClearInputHandlerWindow();
}


void PrintAxis(int x, int y, char *str);

void PSPInputHandler_DisplayButtons()
{
//	pspDebugScreenSetXY(0,13);
//	pspDebugScreenPrintf("                                                     ");

	PrintAxis(15,16, InputTable[g_Mode][0]);
	PrintAxis(35,16, InputTable[g_Mode][0]+8);
}

char *key2str(char key, char *str)
{
	if (key == ' ')
	{
		strcpy(str, "Space");
	}
	else if (key == KEY_BACKSPACE)
	{
		strcpy(str, "BackSp");
	}
	else
	{
		str[0] = key;
		str[1] = 0;
	}

	return str;
}

void PrintAxis(int x, int y, char *str)
{
	char str1[16], str2[16];

	pspDebugScreenSetXY(x, y++);
	pspDebugScreenPrintf("    %s    ", key2str(str[2], str1));
	y++;
	pspDebugScreenSetXY(x, y++);
	pspDebugScreenPrintf(" %s     %s ", key2str(str[1], str1), key2str(str[3], str2));

	pspDebugScreenSetXY(x, y++);
	pspDebugScreenPrintf("    |    ");

	pspDebugScreenSetXY(x, y++);
	pspDebugScreenPrintf("%s  -+-  %s", key2str(str[0], str1), key2str(str[4], str2));

	pspDebugScreenSetXY(x, y++);
	pspDebugScreenPrintf("    |    ");

	pspDebugScreenSetXY(x, y++);
	pspDebugScreenPrintf(" %s     %s ", key2str(str[5], str1), key2str(str[7], str2));

	y++;
	pspDebugScreenSetXY(x, y);
						/*1234567*/
	pspDebugScreenPrintf("         ");
	key2str(str[6], str1);
	pspDebugScreenSetXY(x+(4-strlen(str1)/2), y);
	pspDebugScreenPrintf(str1);
}

/** Button combination to key index map (BKM) */
const static int BKM[16/*indexes*/][8/*buttons*/] =
{
/*      L   R   U   D   SQ  CI  TR  CR   */
	{	1,	0,	0,	0,	0,	0,	0,	0 }, // 0  (left)
	{	1,	0,	1,	0,	0,	0,	0,	0 }, // 1  (left and up)
	{	0,	0,	1,	0,	0,	0,	0,	0 }, // 2  (up)
	{	0,	1,	1,	0,	0,	0,	0,	0 }, // 3  (up and right)
	{	0,	1,	0,	0,	0,	0,	0,	0 }, // 4  (right)
	{	1,	0,	0,	1,	0,	0,	0,	0 }, // 5  (left and down)
	{	0,	0,	0,	1,	0,	0,	0,	0 }, // 6  (down)
	{	0,	1,	0,	1,	0,	0,	0,	0 }, // 7  (down and right)

/*      L   R   U   D   SQ  CI  TR  CR   */
	{	0,	0,	0,	0,	1,	0,	0,	0 }, // 8  (sq)
	{	0,	0,	0,	0,	1,	0,	1,	0 }, // 9  (sq and tr)
	{	0,	0,	0,	0,	0,	0,	1,	0 }, // 10 (tr)
	{	0,	0,	0,	0,	0,	1,	1,	0 }, // 11 (tr and ci)
	{	0,	0,	0,	0,	0,	1,	0,	0 }, // 12 (ci)
	{	0,	0,	0,	0,	1,	0,	0,	1 }, // 13 (sq and cr)
	{	0,	0,	0,	0,	0,	0,	0,	1 }, // 14 (cr)
	{	0,	0,	0,	0,	0,	1,	0,	1 }, // 15 (cr and ci)

};

int PSPInputHandler(SceCtrlData pad, char *key)
{
	int retval = 0;
	int index = 0;
	static int old_g_mode = PSPIHM_NORMAL_1;
	int nl_b = pad.Buttons;
	static int bm = 0;
	SceCtrlLatch latch;

	*key = 0;
	retval = 1;

 	if ( !(nl_b & PSP_CTRL_LTRIGGER) && !(nl_b & PSP_CTRL_RTRIGGER) )
	{
		g_Mode = PSPIHM_NORMAL_1;
	}
	else if ( (nl_b & PSP_CTRL_LTRIGGER) && !(nl_b & PSP_CTRL_RTRIGGER) )
	{
		g_Mode = PSPIHM_NORMAL_2;
	}
	else if ( !(nl_b & PSP_CTRL_LTRIGGER) && (nl_b & PSP_CTRL_RTRIGGER) )
	{
		g_Mode = PSPIHM_NORMAL_3;
	}
	else if ( (nl_b & PSP_CTRL_LTRIGGER) && (nl_b & PSP_CTRL_RTRIGGER) )
	{
		g_Mode = PSPIHM_NORMAL_4;
	}

	if (g_Mode != old_g_mode)
	{
		PSPInputHandler_DisplayButtons();
		old_g_mode = g_Mode;
	}

	sceCtrlReadLatch(&latch);
	if (latch.uiMake)
	{
		// Button Pressed
		bm = latch.uiPress;
	}
	else if (latch.uiBreak)
	{
		//printf("break! ");
		for (index = 0; index < 16 ; index++)
		{
			//ModuleLog(LOG_LOWLEVEL, "index=%d: bm=0x%x A=0x%x B=%d C=%d",
			//	index, bm, (bm & PSP_CTRL_LEFT), (bm & PSP_CTRL_LEFT)?1:0, BKM[index][0]);
			if ( (((bm & PSP_CTRL_LEFT)?1:0)	== BKM[index][0]) &&
				(((bm & PSP_CTRL_RIGHT)?1:0)	== BKM[index][1]) &&
				(((bm & PSP_CTRL_UP)?1:0)		== BKM[index][2]) &&
				(((bm & PSP_CTRL_DOWN)?1:0)		== BKM[index][3]) &&
				(((bm & PSP_CTRL_SQUARE)?1:0)	== BKM[index][4]) &&
				(((bm & PSP_CTRL_CIRCLE)?1:0)	== BKM[index][5]) &&
				(((bm & PSP_CTRL_TRIANGLE)?1:0)	== BKM[index][6]) &&
				(((bm & PSP_CTRL_CROSS)?1:0)	== BKM[index][7]) 	)
			{
				*key = InputTable[g_Mode][g_Modif][index];
				//printf("key = %c ", *key);
			}
		}
		bm = 0;
	}

	return retval;
}
#endif
