/*
 * Raf: Based on scr_printf.c from pspsdk:
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * scr_printf.c - Debug screen functions.
 *
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 * Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
 * Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
 *
 * $Id: scr_printf.c 806 2005-07-31 11:47:10Z stefan $
 */
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <malloc.h>
#include <psptypes.h>
#include <pspkernel.h>
#include <pspdisplay.h>
#include <ScFont_base.h>

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

extern u8 msx[];

CSFont::CSFont()
{
	bg_col = 0; 
	fg_col = 0xFFFFFFFF;
	m_TextMode = TEXTMODE_NORMAL;
	SetFontSize(7, 8);
	SetSurface(NULL,0,0,0,0);
}

CSFont::~CSFont()
{
}

void CSFont::SetFontSize(int iWidth, int iHeight) 
{ 
	m_FontWidth = iWidth; 
	m_FontHeight = iHeight; 	
}

void CSFont::SetBackColor(u32 color)
{
	bg_col = color;
}

void CSFont::SetTextColor(u32 color)
{
	fg_col = color;
}

void CSFont::PutChar(u32 x, u32 y, u32 color, u8 ch)
{
	int 	i,j;
	u8	*font;
	u32 *vram_ptr32;
	u16 *vram_ptr16;
	
	u32 *vram32 = ((u32*)m_sfc_addr + x + (y * m_sfc_width));
	u16 *vram16 = ((u16*)m_sfc_addr + x + (y * m_sfc_width));
	
	font = &msx[ (int)ch * 8];

	if (m_sfc_Bpp == 4)
	{
		for (i=0; i < 8; i++, font++)
		{
			vram_ptr32  = vram32;
			for (j=0; j < 8; j++)
			{
				if ((*font & (128 >> j)))
					*vram_ptr32++ = color;
				else
					vram_ptr32++;
			}
			vram32 += m_sfc_width;
		}
	}
	else
	{
		for (i=0; i < 8; i++, font++)
		{
			vram_ptr16  = vram16;
			for (j=0; j < 8; j++)
			{
				if ((*font & (128 >> j)))
					*vram_ptr16++ = color;
				else
					vram_ptr16++;
			}
			vram16 += m_sfc_width;
		}
	}
}

void CSFont::PutCharWithOutline(u32 x, u32 y, u32 bg_color, u32 fg_color, u8 ch)
{
	/** y */	
	PutChar(x,   y, bg_color, ch);
	PutChar(x+1, y, bg_color, ch);	
	PutChar(x+2, y, bg_color, ch);
	/** y + 2 */
	PutChar(x,   y+2, bg_color, ch);
	PutChar(x+1, y+2, bg_color, ch);	
	PutChar(x+2, y+2, bg_color, ch);
	/** y + 1 */
	PutChar(x,   y+1, bg_color, ch);
	PutChar(x+2, y+1, bg_color, ch);
	PutChar(x+1, y+1, fg_color, ch);	
}

void CSFont::PutCharWithShadow(u32 x, u32 y, u32 bg_color, u32 fg_color, u8 ch)
{
	/** x+1,y+1 */
	PutChar(x+1, y+1, bg_color, ch);
	/** x,y */
	PutChar(x, y, fg_color, ch);	
}


void CSFont::PrintText(u32 pixel_x, u32 pixel_y, u32 color, const char *string)
{
	int i = 0;
	char c;

	if(m_sfc_addr == NULL)
	{
	   return;
	}
	
	for (;;i++)
	{
		c = string[i];
		if (0 == c || pixel_x > m_sfc_width || pixel_y > m_sfc_height)
			break;
			
		switch (c)
		{
			case '\n':
			case '\t':
				PutChar(pixel_x, pixel_y, color, ' ' );
				pixel_x+=m_FontWidth;
				break;
			default: 
				switch(m_TextMode)
				{
					case TEXTMODE_NORMAL:
						PutChar(pixel_x, pixel_y, color, c );
						break;
					case TEXTMODE_OUTLINED:
						PutCharWithOutline(pixel_x, pixel_y, 0, color, c );
						break;
					case TEXTMODE_SHADOWED:
						PutCharWithShadow(pixel_x, pixel_y, 0, color, c );
						break;
					default:
						PutChar(pixel_x, pixel_y, color, c );
						break;
				}
				pixel_x+=m_FontWidth;
		}
	}
}
