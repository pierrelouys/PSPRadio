/* 
	PSPRadio / Music streaming client for the PSP. (Initial Release: Sept. 2005)
	PSPRadio Copyright (C) 2005 Rafael Cabezas a.k.a. Raf
	SandbergUI Copyright (C) 2005 Jesper Sandberg

	
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
#include <list>
#include <PSPApp.h>
#include <PSPSound.h>
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <malloc.h>
#include <math.h>
#include <stdarg.h>

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>

#include <pspgu.h>
#include <pspgum.h>

#include "SandbergUI.h"

#define FONT_WIDTH		8
#define	FONT_HEIGHT		16


static char __attribute__((aligned(16))) char_list[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
							'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5',
							'6', '7', '8', '9', ':', '.', ',', '-', '+', '(', ')', '[', ']', '?', '=', ';',
							'\\', '/', '<', '>', '!', '"', '#', '&', '$', '@', '{', '}', '*', '\'', '%', ' '};
static CSandbergUI::TexCoord __attribute__((aligned(16))) option_frame = { 8, 8, 480-8, 272-8};


void CSandbergUI::RenderOptionScreen(void)
{
	RenderFX();
	RenderFrame(option_frame, 0xFFFFFFFF);
	RenderOptionLogo();
	RenderOptions(RENDER_OPTIONS);
}

void CSandbergUI::RenderOptionLogo(void)
{
	sceGuEnable(GU_TEXTURE_2D);

	sceGuAlphaFunc(GU_GREATER,0,0xff);
	sceGuEnable(GU_ALPHA_TEST);
	sceGuBlendFunc(GU_ADD, GU_SRC_COLOR, GU_DST_COLOR, 0, 0);
	sceGuEnable(GU_BLEND);

	// setup texture
	(void)tcache.jsaTCacheSetTexture(TEX_OPTIONS);
	sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);

	struct Vertex* c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	c_vertices[0].u = 0; c_vertices[0].v = 0;
	c_vertices[0].x = 176; c_vertices[0].y = 8; c_vertices[0].z = 0;
	c_vertices[0].color = 0xFFFFFFFF;
	c_vertices[1].u = 128; c_vertices[1].v = 64;
	c_vertices[1].x = 304; c_vertices[1].y = 72; c_vertices[1].z = 0;
	c_vertices[1].color = 0xFFFFFFFF;
	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,c_vertices);

	sceGuDisable(GU_BLEND);
	sceGuDisable(GU_ALPHA_TEST);
	sceGuDisable(GU_TEXTURE_2D);
}

void CSandbergUI::UpdateOptionsScreen(list<CScreenHandler::Options> &OptionsList, list<CScreenHandler::Options>::iterator &CurrentOptionIterator)
{
	bool active_item;
	int y = 6;
	list<CScreenHandler::Options>::iterator OptionIterator;
	CScreenHandler::Options	Option;

	while(OptionsItems.size())
	{
		OptionsItems.pop_front();
	}

	if (OptionsList.size() > 0)
	{
		for (OptionIterator = OptionsList.begin() ; OptionIterator != OptionsList.end() ; OptionIterator++)
		{
			if (OptionIterator == CurrentOptionIterator)
			{
			active_item = true;
			}
			else
			{
			active_item = false;
			}
			Option = (*OptionIterator);
			StoreOption(y, active_item, Option.strName, Option.strStates, Option.iNumberOfStates, Option.iSelectedState, Option.iActiveState);
			y += 1;
		}
	}
}

void CSandbergUI::StoreOption(int y, bool active_item, char *strName, char *strStates[], int iNumberOfStates, int iSelectedState, int iActiveState)
{
	int 			x = 5;
	int 			color = 0xFFFFFFFF;
	StoredOptionItem	Option;

	Option.x = x;
	Option.y = y;
	if (active_item)
	{
		Option.color = 0xFFFFFFFF;
	}
	else
	{
		Option.color = 0xFF888888;
	}
	strcpy(Option.strText, strName);
	strupr(Option.strText);
	Option.ID = TEXT_OPTION;
	OptionsItems.push_back(Option);

	if (iNumberOfStates > 0)
	{
		x = 30;
		for (int iStates = 0; iStates < iNumberOfStates ; iStates++)
		{
			if (iStates+1 == iActiveState)
			{
				color = 0xFF0000FF;
			}
			else if (iStates+1 == iSelectedState) /** 1-based */
			{
				color = 0xFFFFFFFF;
			}
			else
			{
				color = 0xFF888888;
			}
			
			if ((iStates+1 == iActiveState) && (iStates+1 == iSelectedState))
			{
				color =  0xFF9090E3;
			}

			Option.x 	= x;
			Option.y 	= y;
			Option.color 	= color;
			strcpy(Option.strText, strStates[iStates]);
			strupr(Option.strText);
			Option.ID = TEXT_OPTION;
			OptionsItems.push_back(Option);

			x += strlen(strStates[iStates])+1;
		}
	}	
}

void CSandbergUI::RenderOptions(int render_option)
{
	StoredOptionItem			Option;
	list<StoredOptionItem>::iterator 	OptionIterator;
	char 					strText[MAX_OPTION_LENGTH];

	sceGuEnable(GU_TEXTURE_2D);

	sceGuAlphaFunc(GU_GREATER,0,0xff);
	sceGuEnable(GU_ALPHA_TEST);
	sceGuDepthFunc(GU_ALWAYS);

	// setup texture
	(void)tcache.jsaTCacheSetTexture(TEX_FONT_SMALL);
	sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);

	if (OptionsItems.size() > 0)
	{
		for (OptionIterator = OptionsItems.begin() ; OptionIterator != OptionsItems.end() ; OptionIterator++)
		{
			int 			strsize;
			int 			sx, sy;
			struct TexCoord		texture_offset;

			Option = (*OptionIterator);

			if (((render_option == RENDER_OPTIONS)  && (Option.ID == TEXT_OPTION)) ||
			    ((render_option == RENDER_METADATA) && (Option.ID != TEXT_OPTION)))
			{
				sprintf(strText, Option.strText);
				strsize = strlen(strText);
				sx = Option.x * FONT_WIDTH;
				sy = Option.y * FONT_HEIGHT;

				struct Vertex* c_vertices = (struct Vertex*)sceGuGetMemory(strsize * 2 * sizeof(struct Vertex));

				for (int i = 0, index = 0 ; i < strsize ; i++)
				{
					char	letter = strText[i];
					FindSmallFontTexture(letter, &texture_offset);

					c_vertices[index+0].u 		= texture_offset.x1;
					c_vertices[index+0].v 		= texture_offset.y1;
					c_vertices[index+0].x 		= sx;
					c_vertices[index+0].y 		= sy;
					c_vertices[index+0].z 		= 0;
					c_vertices[index+0].color 	= Option.color;

					c_vertices[index+1].u 		= texture_offset.x2;
					c_vertices[index+1].v 		= texture_offset.y2;
					c_vertices[index+1].x 		= sx + FONT_WIDTH;
					c_vertices[index+1].y 		= sy + FONT_HEIGHT;
					c_vertices[index+1].z 		= 0;
					c_vertices[index+1].color 	= Option.color;

					sx 	+= FONT_WIDTH;
					index 	+= 2;
				}
				sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,strsize * 2,0,c_vertices);
			}
		}
	}

	sceGuDepthFunc(GU_GEQUAL);
	sceGuDisable(GU_ALPHA_TEST);
	sceGuDisable(GU_TEXTURE_2D);
}

void CSandbergUI::FindSmallFontTexture(char index, struct TexCoord *texture_offset)
{
	int maxsize = sizeof(::char_list);
	bool found_char = false;
	
	texture_offset->x1 = 0;
	texture_offset->y1 = 0;
	texture_offset->x2 = FONT_WIDTH;
	texture_offset->y2 = FONT_HEIGHT;

	for (int i = 0 ; i < maxsize ; i++)
	{
		if (::char_list[i] == index)
		{
		found_char = true;
		break;
		}
		texture_offset->x1 += FONT_WIDTH;
		texture_offset->x2 += FONT_WIDTH;
	}

	/* If we didn't find a matching character then return the index of the last char (space) */
	if (!found_char)
	{
		texture_offset->x1 -= FONT_WIDTH;
		texture_offset->x2 -= FONT_WIDTH;
	}
}
