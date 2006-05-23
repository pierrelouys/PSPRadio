/*
	Random number generator for the Sony PSP.
	Copyright (C) 2006 Jesper Sandberg


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
#include <limits.h>
#include <pspkernel.h>
#include <psputils.h>

#include <stdio.h>

#include "jsaRand.h"

/* MT context */
static SceKernelUtilsMt19937Context ctx;

void jsaRandInit(void)
{
	sceKernelUtilsMt19937Init(&ctx, time(NULL));
}

u32 jsaRand(void)
{
u32	rand_val;

	rand_val = sceKernelUtilsMt19937UInt(&ctx);

	return rand_val;
}

int jsaRandRange(int min, int max)
{
u32	rand_val;

	rand_val = sceKernelUtilsMt19937UInt(&ctx);

	rand_val %= (max-min);
	rand_val += min;

	return rand_val;
}

float jsaRandRangef(float min, float max)
{
float	rand_val;

	/* Create a number between 0 and 1 */
	rand_val = (float)sceKernelUtilsMt19937UInt(&ctx);
	rand_val /=  (float)INT_MAX * 2;

	/* Scale and offset */
	rand_val *= (max-min);
	rand_val += min;

	return rand_val;
}
