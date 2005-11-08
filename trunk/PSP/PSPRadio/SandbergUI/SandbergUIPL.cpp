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

#include <Tools.h>

#include "SandbergUI.h"

#define		LIST_COUNT		5
#define		LIST_MARGIN		(((LIST_COUNT-1) / 2) + 1)

#define		FRAME_WIDTH		176
#define		FRAME_HEIGHT		 80
#define		LIST_FRAME_X		  6
#define		LIST_FRAME_Y		188
#define		LIST_TEXT_X		  1
#define		LIST_TEXT_Y		 12
#define		ENTRY_FRAME_X		(480-LIST_FRAME_X-FRAME_WIDTH)
#define		ENTRY_FRAME_Y		188
#define		ENTRY_TEXT_X		 38
#define		ENTRY_TEXT_Y		 12
#define		MAX_CHARS		((FRAME_WIDTH/8)-1)

#define		SELECT_MIN_X		LIST_FRAME_X
#define		SELECT_MAX_X		ENTRY_FRAME_X
#define		SELECT_Y		LIST_FRAME_Y

static CSandbergUI::TexCoord __attribute__((aligned(16))) meta_frame =  	{  64,  92, 416, 176};
static CSandbergUI::TexCoord __attribute__((aligned(16))) list_frame =  	{ LIST_FRAME_X, LIST_FRAME_Y, LIST_FRAME_X+FRAME_WIDTH,LIST_FRAME_Y+FRAME_HEIGHT};
static CSandbergUI::TexCoord __attribute__((aligned(16))) entry_frame =  	{ ENTRY_FRAME_X, ENTRY_FRAME_Y, ENTRY_FRAME_X+FRAME_WIDTH,ENTRY_FRAME_Y+FRAME_HEIGHT};

#define SIN_COUNT       64

static float __attribute__((aligned(16))) sintable[] =
{
	0.000000f,0.000622f,0.002485f,0.005585f,0.009914f,0.015461f,0.022214f,0.030154f,
	0.039262f,0.049516f,0.060889f,0.073355f,0.086881f,0.101434f,0.116978f,0.133474f,
	0.150882f,0.169157f,0.188255f,0.208128f,0.228727f,0.250000f,0.271895f,0.294356f,
	0.317330f,0.340757f,0.364580f,0.388740f,0.413176f,0.437828f,0.462635f,0.487535f,
	0.512465f,0.537365f,0.562172f,0.586824f,0.611260f,0.635420f,0.659243f,0.682671f,
	0.705644f,0.728105f,0.750000f,0.771273f,0.791872f,0.811745f,0.830843f,0.849118f,
	0.866526f,0.883022f,0.898566f,0.913119f,0.926645f,0.939111f,0.950484f,0.960738f,
	0.969846f,0.977786f,0.984539f,0.990086f,0.994415f,0.997515f,0.999378f,1.000000f,
};

void CSandbergUI::RenderPL(void)
{
	static int	rotate = 0;

	RenderFrame(meta_frame, 0xFFFFFFFF);
	RenderFrame(list_frame, 0xFFFFFFFF);
	RenderFrame(entry_frame, 0xFFFFFFFF);

	RenderActiveList();

	RenderOptions(RENDER_METADATA);

	rotate++;
}

void CSandbergUI::RenderFrame(TexCoord &area, unsigned int tex_color)
{
	sceGuEnable(GU_TEXTURE_2D);
	// setup texture
	(void)tcache.jsaTCacheSetTexture(TEX_PLATE);
	sceGuTexFunc(GU_TFX_BLEND,GU_TCC_RGBA);
	sceGuTexEnvColor(0xFF000000);
	sceGuDepthFunc(GU_ALWAYS);
	sceGuBlendFunc(GU_ADD, GU_SRC_COLOR, GU_DST_COLOR, 0, 0);
	sceGuEnable(GU_BLEND);

	struct Vertex* c_vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	c_vertices[0].u = 0; c_vertices[0].v = 0;
	c_vertices[0].x = area.x1; c_vertices[0].y = area.y1; c_vertices[0].z = 0;
	c_vertices[0].color = tex_color;
	c_vertices[1].u = 64; c_vertices[1].v = 64;
	c_vertices[1].x = area.x2; c_vertices[1].y = area.y2; c_vertices[1].z = 0;
	c_vertices[1].color = tex_color;
	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,c_vertices);

	sceGuDisable(GU_BLEND);
	sceGuDisable(GU_TEXTURE_2D);

	struct Vertex* l_vertices = (struct Vertex*)sceGuGetMemory(5 * sizeof(struct Vertex));
	l_vertices[0].x = area.x1;	l_vertices[0].y = area.y1; 	l_vertices[0].color = 0xFFFFFFFF;
	l_vertices[1].x = area.x2;	l_vertices[1].y = area.y1; 	l_vertices[1].color = 0xFFFFFFFF;
	l_vertices[2].x = area.x2;	l_vertices[2].y = area.y2; 	l_vertices[2].color = 0xFFFFFFFF;
	l_vertices[3].x = area.x1;	l_vertices[3].y = area.y2; 	l_vertices[3].color = 0xFFFFFFFF;
	l_vertices[4].x = area.x1;	l_vertices[4].y = area.y1; 	l_vertices[4].color = 0xFFFFFFFF;

	sceGuDrawArray(GU_LINE_STRIP,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,5,0,l_vertices);

	sceGuDepthFunc(GU_GEQUAL);
}

void CSandbergUI::DisplayContainers(CMetaDataContainer *Container)
{
	int 		list_cnt, render_cnt;
	int		current = 1;
	int		i = 0;
	unsigned int	color;
	int		y = LIST_TEXT_Y;
	int		first_entry;
	char 	*strTemp = (char *)malloc (MAXPATHLEN);

	map< string, list<MetaData>* >::iterator ListIterator;
	map< string, list<MetaData>* >::iterator *CurrentElement = Container->GetCurrentContainerIterator();
	map< string, list<MetaData>* > *List = Container->GetContainerList();

	list_cnt = List->size();

	/*  Find number of current element */
	if (list_cnt > 0)
	{
		for (ListIterator = List->begin() ; ListIterator != List->end() ; ListIterator++, current++)
		{
			if (ListIterator == *CurrentElement)
			{
			break;
			}
		}

		/* Calculate start element in list */
		first_entry = FindFirstEntry(list_cnt, current);

		/* Find number of elements to show */
		render_cnt = (list_cnt > LIST_COUNT) ? LIST_COUNT : list_cnt;

		/* Go to start element */
		for (ListIterator = List->begin() ; i < first_entry ; i++, ListIterator++);

		for (i = 0 ; i < LIST_COUNT ; i++)
		{
			if (i < render_cnt)
			{
				if (ListIterator == *CurrentElement)
				{
					color = 0xFFFFFFFF;
				}
				else
				{
					color = 0xFF444444;
				}
				strncpy(strTemp, ListIterator->first.c_str(), MAXPATHLEN);
				strTemp[MAXPATHLEN - 1] = 0;
				if (strlen(strTemp) > 4 && memcmp(strTemp, "ms0:", 4) == 0)
				{
					char *pStrTemp = basename(strTemp);
					pStrTemp[MAX_CHARS] = 0;
					UpdateTextItem(TEXT_PL_LIST1 +  i, LIST_TEXT_X, y++, pStrTemp, color);
				}
				ListIterator++;
			}
			else
			{
				/* Add dummy elements for cleaning the list */
				strTemp[0] = ' ';
				strTemp[1] = 0;
				color = 0xFFFFFFFF;
				UpdateTextItem(TEXT_PL_LIST1 +  i, LIST_TEXT_X, y++, strTemp, color);
			}
		}
	}
	free (strTemp), strTemp = NULL;
}

void CSandbergUI::DisplayElements(CMetaDataContainer *Container)
{
	int 		list_cnt, render_cnt;
	int		current = 1;
	int		i = 0;
	unsigned int	color;
	int		y = ENTRY_TEXT_Y;
	int		first_entry;
	char		strTemp[MAX_CHARS+1];

	list<MetaData>::iterator ListIterator;
	list<MetaData>::iterator *CurrentElement = Container->GetCurrentElementIterator();
	list<MetaData> *List = Container->GetElementList();

	list_cnt = List->size();

	/*  Find number of current element */
	if (list_cnt > 0)
	{
		for (ListIterator = List->begin() ; ListIterator != List->end() ; ListIterator++, current++)
		{
			if (ListIterator == *CurrentElement)
			{
			break;
			}
		}

		/* Calculate start element in list */
		first_entry = FindFirstEntry(list_cnt, current);

		/* Find number of elements to show */
		render_cnt = (list_cnt > LIST_COUNT) ? LIST_COUNT : list_cnt;

		/* Go to start element */
		for (ListIterator = List->begin() ; i < first_entry ; i++, ListIterator++);

		for (i = 0 ; i < LIST_COUNT ; i++)
		{
			if (i < render_cnt)
			{
				if (ListIterator == *CurrentElement)
				{
					color = 0xFFFFFFFF;
				}
				else
				{
					color = 0xFF444444;
				}

				if (strlen((*ListIterator).strTitle))
				{
					strncpy(strTemp, (*ListIterator).strTitle, MAX_CHARS);
					strTemp[MAX_CHARS] = 0;
				}
				else
				{
					strncpy(strTemp, (*ListIterator).strURI, MAX_CHARS);
					strTemp[MAX_CHARS] = 0;
				}
				ListIterator++;
			}
			else
			{
				/* Add dummy elements for cleaning the list */
				strTemp[0] = ' ';
				strTemp[1] = 0;
				color = 0xFFFFFFFF;
			}
			UpdateTextItem(TEXT_PL_ENTRY1 +  i, ENTRY_TEXT_X, y++, strTemp, color);
		}
	}

}

int CSandbergUI::FindFirstEntry(int list_cnt, int current)
{
	int		first_entry;

	/* Handle start of list */
	if ((list_cnt<=LIST_COUNT) || (current < LIST_MARGIN))
	{
		first_entry = 0;
	}
	/* Handle end of list */
	else if ((list_cnt > LIST_COUNT) && ((list_cnt - current) < LIST_MARGIN))
	{
		first_entry = list_cnt - LIST_COUNT;
	}
	/* Handle rest of list */
	else
	{
		first_entry = current - LIST_MARGIN;
	}
	
	return first_entry;
}

void CSandbergUI::OnCurrentContainerSideChange(CMetaDataContainer *Container)
{

	switch (Container->GetCurrentSide())
	{
		case	CMetaDataContainer::CONTAINER_SIDE_CONTAINERS:
			select_target = 0;
			break;
		case	CMetaDataContainer::CONTAINER_SIDE_ELEMENTS:
			select_target = SIN_COUNT-1;
			break;
		default:
			break;
	}
}

void CSandbergUI::RenderActiveList(void)
{
	float	start_x;

	sceGuDepthFunc(GU_ALWAYS);
	sceGuBlendFunc(GU_ADD, GU_SRC_COLOR, GU_DST_COLOR, 0, 0);
	sceGuEnable(GU_BLEND);

	/* if we are moving then find the new coord */
	if (select_state != select_target)
	{
		if (select_state < select_target)
		{
			select_state++;
		}
		else
		{
			select_state--;
		}
	}
	start_x = SELECT_MIN_X + ::sintable[select_state] * (SELECT_MAX_X-SELECT_MIN_X);

	struct SVertex* l_vertices = (struct SVertex*)sceGuGetMemory(2 * 3 * sizeof(struct SVertex));

	sceGuColor(0xFFFFFFFF);

	l_vertices[0].x = start_x;
	l_vertices[0].y = SELECT_Y;
	l_vertices[0].z = 0;

	l_vertices[1].x = start_x + FRAME_WIDTH;
	l_vertices[1].y = SELECT_Y;
	l_vertices[1].z = 0;

	l_vertices[2].x = start_x;
	l_vertices[2].y = SELECT_Y + FRAME_HEIGHT;
	l_vertices[2].z = 0;

	l_vertices[3].x = start_x;
	l_vertices[3].y = SELECT_Y + FRAME_HEIGHT;
	l_vertices[3].z = 0;

	l_vertices[4].x = start_x + FRAME_WIDTH;
	l_vertices[4].y = SELECT_Y;
	l_vertices[4].z = 0;

	l_vertices[5].x = start_x + FRAME_WIDTH;
	l_vertices[5].y = SELECT_Y + FRAME_HEIGHT;
	l_vertices[5].z = 0;

	sceGuDrawArray(GU_TRIANGLES,GU_VERTEX_32BITF|GU_TRANSFORM_2D,2*3,0,l_vertices);

	sceGuDisable(GU_BLEND);
	sceGuDepthFunc(GU_GEQUAL);
}
