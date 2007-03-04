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
#include <pspge.h>
#include <stdarg.h>
#include <Screen.h>
#include <png.h>

u32 *third = NULL;

CScreen::CScreen(int width, int height, int pitch, int pixel_format)
{
	m_Width = width;
	m_Height = height;
	m_Pitch = pitch;
	m_PixelFormat = pixel_format;
	switch(m_PixelFormat)
	{
	case PSP_DISPLAY_PIXEL_FORMAT_565:
		FRAMESIZE = m_Pitch * m_Height * 2;
		break;
	case PSP_DISPLAY_PIXEL_FORMAT_5551:
		FRAMESIZE = m_Pitch * m_Height * 2;
		break;
	case PSP_DISPLAY_PIXEL_FORMAT_4444:
		FRAMESIZE = m_Pitch * m_Height * 2;
		break;
	case PSP_DISPLAY_PIXEL_FORMAT_8888:
		FRAMESIZE = m_Pitch * m_Height * 4;
		break;
	};
	
	bg_col = 0; 
	fg_col = 0xFFFFFFFF;
	init = false;
	m_TextMode = TEXTMODE_NORMAL;
	m_FontWidth = 7; 
	m_FontHeight = 8;
	SetFontSize(7, 8);

	Init();
}

CScreen::~CScreen()
{
	free (m_AllocatedBuffer[0]);
	free (m_AllocatedBuffer[1]);
}

void CScreen::Init()
{
	/* Place vram in uncached memory */
	//u32 *g_vram_base = (u32 *) (0x40000000 | (u32) sceGeEdramGetAddr());
	/* let's use cached memory better */
	u32 *g_vram_base = (u32 *) ((u32) sceGeEdramGetAddr());
	m_AllocatedBuffer[0] = (u32*)memalign(16, FRAMESIZE); //Used to store the background
	m_AllocatedBuffer[1] = NULL;

	sceDisplaySetMode(0, m_Width, m_Height);
	sceDisplaySetFrameBuf((void *) g_vram_base, 
		m_Pitch, m_PixelFormat, PSP_DISPLAY_SETBUF_NEXTFRAME);

	for (int i = 0; i < 4; i++)
	{
		m_Buffer[i] = (u32*)((char*)g_vram_base+FRAMESIZE*i);
	}
	//m_Buffer[2] = m_AllocatedBuffer[0];
	m_Buffer[3] = m_AllocatedBuffer[0];

	init = true;
}

void CScreen::SetFrameBuffer(int iBuffer)
{
	sceDisplaySetFrameBuf(m_Buffer[iBuffer], 
		m_Pitch, m_PixelFormat, PSP_DISPLAY_SETBUF_NEXTFRAME);
}

void CScreen::Plot(int iBuffer, int x, int y, int color)
{
	u32 *pixel = m_Buffer[iBuffer] + m_Pitch*y + x;
	//*pixel = color;//*pixel & 0xAAAAAAAA;//color;
	//*pixel = *pixel & 0xAAAAAAAA;//color;
	*pixel = *pixel | color;//0xAAAAAAAA;//color;
}

void CScreen::VertLine(int iBuffer, int x, int y1, int y2, int color)
{
	if (y1 < y2)
	{
		for (int y = y1<0?0:y1; y <= y2; y++)
		{
			Plot(iBuffer, x, y, color);
		}
	}
	else if (y2 < y1)
	{
		for (int y = y2<0?0:y2; y <= y1; y++)
		{
			Plot(iBuffer, x, y, color);
		}
	}
	else 
	{
		Plot(iBuffer, x, y1, color);
	}
	
}

void CScreen::CopyFromToBuffer(int iBufferFrom, int iBufferTo)
{
	memcpy(m_Buffer[iBufferTo], m_Buffer[iBufferFrom], FRAMESIZE);
}

int CScreen::Peek(int iBuffer, int x, int y)
{
	u32 *pixel = m_Buffer[iBuffer] + m_Pitch*y + x;
	return *pixel;
}

void CScreen::Rectangle(int iBuffer, int x1, int y1, int x2, int y2, int color)
{
	for (int x = x1;x <= x2; x++)
	{
		for (int y = y1;y <= y2; y++)
		{
			Plot(iBuffer, x, y, color);
		}
	}
}

void CScreen::SetFontSize(int iWidth, int iHeight) 
{ 
	m_FontWidth = iWidth; 
	m_FontHeight = iHeight; 	
}

/* baseado nas libs do Duke... */

void BlitImage(u32 x1, u32 y1, u32 x2, u32 y2);

/** PNG Stuff **/
/** From pspsdk's libpng example. (Copyright (c) 2005 Frank Buss <fb@frank-buss.de> (aka Shine)) */
void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
	// ignore PNG warnings
}

/* Load an image and show it to screen */
void CScreen::LoadBuffer(int iBuffer, const char* filename)
{
	u32 *vram32;
	u16 *vram16;
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	size_t x, y;
	u32* line;
	FILE *fp;

	if ((fp = fopen(filename, "rb")) == NULL) return;
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fclose(fp);
		return;
	}
	png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) NULL, user_warning_fn);
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return;
	}
	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, sig_read);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, int_p_NULL, int_p_NULL);
	png_set_strip_16(png_ptr);
	png_set_packing(png_ptr);
	if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_gray_1_2_4_to_8(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
	png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	line = (u32*) malloc(width * 32);//4);
	if (!line) {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return;
	}
	vram16 = (u16*) m_Buffer[iBuffer];
	vram32 = m_Buffer[iBuffer];

	for (y = 0; y < height; y++) {
		png_read_row(png_ptr, (u8*) line, png_bytep_NULL);
		for (x = 0; x < width; x++) {
			u32 color32 = line[x];
			u16 color16;
			int r = color32 & 0xff;
			int g = (color32 >> 8) & 0xff;
			int b = (color32 >> 16) & 0xff;
			switch (m_PixelFormat) {
				case PSP_DISPLAY_PIXEL_FORMAT_565:
					color16 = (r >> 3) | ((g >> 2) << 5) | ((b >> 3) << 11);
					vram16[x + y * m_Pitch] = color16;
					break;
				case PSP_DISPLAY_PIXEL_FORMAT_5551:
					color16 = (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10);
					vram16[x + y * m_Pitch] = color16;
					break;
				case PSP_DISPLAY_PIXEL_FORMAT_4444:
					color16 = (r >> 4) | ((g >> 4) << 4) | ((b >> 4) << 8);
					vram16[x + y * m_Pitch] = color16;
					break;
				case PSP_DISPLAY_PIXEL_FORMAT_8888:
					color32 = r | (g << 8) | (b << 16);
					vram32[x + y * m_Pitch] = color32;
					break;
			}
		}
	}
	free(line);
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
	fclose(fp);
}
/** PNG STUFF */

void CScreen::CopyRectangle(int iFromBuffer, int iDestBuffer, u32 x1, u32 y1, u32 x2, u32 y2)
{
	u32 *src = m_Buffer[iFromBuffer] + x1 + (y1*m_Pitch);
	u32 *dst = m_Buffer[iDestBuffer] + x1 + (y1*m_Pitch);
	int xlen_in_bytes = (x2 - x1)*4;
	int ylen = y2 - y1;

	for (int y = 0; y < ylen; y++)
	{
		memcpy(dst + y*m_Pitch, src + y*m_Pitch, xlen_in_bytes);
	}
}

void CScreen::SetBackColor(u32 color)
{
   bg_col = color;
}

void CScreen::SetTextColor(u32 color)
{
   fg_col = color;
}

void CScreen::Effect(int iBuffer)
{
	u32 *pixel = NULL;
	for (int y = 0; y < 100; y++)
	{
		for (int x = 0; x < 100 /*m_Pitch*/; x++)
		{
			pixel = m_Buffer[iBuffer] + m_Pitch*y + x;
			*pixel = *pixel * 2;
		}
	}
	
}

extern u8 msx[];

void CScreen::PutChar(int iBuffer, int x, int y, u32 color, u8 ch)
{
	int 	i,j;
	u8	*font;
	u32 *vram_ptr;
	
	if(false == init)
	{
	   return;
	}
	
	u32 *vram = m_Buffer[iBuffer] + x + (y * m_Pitch);
	
	
	font = &msx[ (int)ch * 8];
	for (i=0; i < 8; i++, font++)
	{
		vram_ptr  = vram;
		for (j=0; j < 8; j++)
		{
			if ((*font & (128 >> j)))
				*vram_ptr++ = color;
			else
				vram_ptr++;
		}
		vram += m_Pitch;
	}
}

void CScreen::PutCharWithOutline(int iBuffer, int x, int y, u32 bg_color, u32 fg_color, u8 ch)
{
	/** y */	
	PutChar(iBuffer, x,   y, bg_color, ch);
	PutChar(iBuffer, x+1, y, bg_color, ch);	
	PutChar(iBuffer, x+2, y, bg_color, ch);
	/** y + 2 */
	PutChar(iBuffer, x,   y+2, bg_color, ch);
	PutChar(iBuffer, x+1, y+2, bg_color, ch);	
	PutChar(iBuffer, x+2, y+2, bg_color, ch);
	/** y + 1 */
	PutChar(iBuffer, x,   y+1, bg_color, ch);
	PutChar(iBuffer, x+2, y+1, bg_color, ch);
	PutChar(iBuffer, x+1, y+1, fg_color, ch);	
}

void CScreen::PutCharWithShadow(int iBuffer, int x, int y, u32 bg_color, u32 fg_color, u8 ch)
{
	/** x+1,y+1 */
	PutChar(iBuffer, x+1, y+1, bg_color, ch);
	/** x,y */
	PutChar(iBuffer, x, y, fg_color, ch);	
}


void CScreen::PrintText(int iBuffer, int pixel_x, int pixel_y, int color, char *string)
{
	int i = 0;
	char c;

	for (;;i++)
	{
		c = string[i];
		if (0 == c || pixel_x > m_Width || pixel_y > m_Height)
			break;
			
		switch (c)
		{
			case '\n':
			case '\t':
				PutChar(iBuffer, pixel_x, pixel_y, color, ' ' );
				pixel_x+=m_FontWidth;
				break;
			default: 
				switch(m_TextMode)
				{
					case TEXTMODE_NORMAL:
						PutChar(iBuffer, pixel_x, pixel_y, color, c );
						break;
					case TEXTMODE_OUTLINED:
						PutCharWithOutline(iBuffer, pixel_x, pixel_y, 0, color, c );
						break;
					case TEXTMODE_SHADOWED:
						PutCharWithShadow(iBuffer, pixel_x, pixel_y, 0, color, c );
						break;
					default:
						PutChar(iBuffer, pixel_x, pixel_y, color, c );
						break;
				}
				pixel_x+=m_FontWidth;
		}
	}
}
