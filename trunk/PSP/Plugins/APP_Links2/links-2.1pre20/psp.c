#include <stdio.h>
#include <pspsdk.h>
#include <pspctrl.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include "psp.h"
#include <PSPRadio_Exports.h>

#define printf pspDebugScreenPrintf

#define PSP_SCREEN_WIDTH 480
#define PSP_SCREEN_HEIGHT 272

volatile tBoolean g_PSPEnableRendering = truE;
volatile tBoolean g_PSPEnableInput = truE;

int env_termsize(int *x, int *y)
{
	*x = PSP_SCREEN_WIDTH / 7;
	*y = PSP_SCREEN_HEIGHT / 8;

	return 0;
}
