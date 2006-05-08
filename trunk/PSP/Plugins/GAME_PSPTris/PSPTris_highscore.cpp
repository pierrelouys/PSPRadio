/*
	PSPTris - The game - Highscore handling
	Copyright (C) 2006  Jesper Sandberg

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
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <pspkernel.h>

#include "PSPTris.h"
#include "PSPTris_highscore.h"
#include "PSPTris_game.h"

#define		HIGHSCORE_COUNT		10

static highscore_str	highscore[] =	{
										/* For classic */
										{10000, "PSP",  1},
										{ 9000, "PSP",  2},
										{ 8000, "PSP",  3},
										{ 7000, "PSP",  4},
										{ 6000, "PSP",  5},
										{ 5000, "PSP",  6},
										{ 4000, "PSP",  7},
										{ 3000, "PSP",  8},
										{ 2000, "PSP",  9},
										{ 1000, "PSP", 10},
										/* For classic */
										{50000, "PSP",  1},
										{45000, "PSP",  2},
										{40000, "PSP",  3},
										{35000, "PSP",  4},
										{30000, "PSP",  5},
										{25000, "PSP",  6},
										{20000, "PSP",  7},
										{15000, "PSP",  8},
										{10000, "PSP",  9},
										{ 5000, "PSP", 10},
										};

static highscore_str	*classic_hs;
static highscore_str	*color_hs;

static FILE				*fhandle;

void PSPTris_highscore_init(void)
{
	fhandle = fopen("./highscore.dat", "a+");
	if (fhandle != NULL)
		{
		int bytes;

		(void)fseek(fhandle, 0, SEEK_END);
		int filesize = ftell(fhandle);
		(void)fseek(fhandle, 0, SEEK_SET);

		if (filesize != -1)
			{
			if (filesize > 0)
				{
				bytes = fread(highscore, sizeof(highscore), 1, fhandle);
				if (bytes != 1)
					{
					printf("Wrong filesize for highscore.dat!! (%d)\n", bytes);
					}
				}
			else
				{
				bytes = fwrite(highscore, sizeof(highscore), 1, fhandle);
				if (bytes != 1)
					{
					printf("Couldn't write highscore.dat!!\n");
					}
				}
			}
		fclose(fhandle);
		}
	else
		{
		printf("Couldn't open or create highscore.dat!!!\n");
		}

	classic_hs	= highscore;
	color_hs	= highscore+10;
}

u32 PSPTris_highscore_check(u32 score, u32 game_type)
{
highscore_str *table = highscore + game_type * HIGHSCORE_COUNT;

	for (u32 i = 0 ; i < HIGHSCORE_COUNT ; i++)
		{
		if (score > table[i].score)
			{
			return i+1;
			}
		}
	return 0;
}

highscore_str *PSPTris_highscore_get(u32 rank, u32 game_type)
{
highscore_str *table = highscore + game_type * HIGHSCORE_COUNT;

	return &table[rank-1];
}

void PSPTris_highscore_store(highscore_str *new_highscore, u32 game_type)
{
highscore_str *table = highscore + game_type * HIGHSCORE_COUNT;

	/* shift down the scores below this one */
	for (u32 i = HIGHSCORE_COUNT - 2 ; i >= new_highscore->rank - 1; i--)
		{
		memcpy(&table[i+1], &table[i], sizeof(highscore_str));
		}
	memcpy(&table[new_highscore->rank - 1], new_highscore, sizeof(highscore_str));

	/* save the current highscore list to highscore.dat */
	fhandle = fopen("./highscore.dat", "w");

	if (fhandle != NULL)
		{
		int bytes;

		bytes = fwrite(highscore, sizeof(highscore), 1, fhandle);
		if (bytes != 1)
			{
			printf("Couldn't write highscore.dat!!\n");
			}
		fclose(fhandle);
		}
	else
		{
		printf("Couldn't write highscore.dat!!!\n");
		}
}
