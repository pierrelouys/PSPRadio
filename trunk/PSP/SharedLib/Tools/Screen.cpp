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

CScreen::CScreen()
{
	
	X = 0; 
	Y = 0; 
	bg_col = 0; 
	fg_col = 0xFFFFFFFF;
	g_vram_base = (u32 *)0x04000000;
	init = false;
	m_strImage = NULL;
	m_ImageBuffer = NULL;
	m_TextMode = TEXTMODE_NORMAL;
	m_FontWidth = 7; 
	m_FontHeight = 8;
	MX = 68;
	MY = 34;
	SetFontSize(7, 8);
	
	Init();
}

CScreen::~CScreen()
{
	if (m_strImage)
	{
		free (m_strImage), m_strImage = NULL;
	}
	
	if (m_ImageBuffer)
	{
		free (m_ImageBuffer), m_ImageBuffer = NULL;
	}

}

void CScreen::SetFontSize(int iWidth, int iHeight) 
{ 
	m_FontWidth = iWidth; 
	m_FontHeight = iHeight; 	
	MX = GetNumberOfTextColumns();//SCREEN_MAX_X;
	MY = GetNumberOfTextRows();//SCREEN_MAX_Y;
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
void CScreen::LoadImage(const char* filename, u32 *ImageBuffer)
{
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
	for (y = 0; y < height; y++) 
	{
		png_read_row(png_ptr, (u8*) line, png_bytep_NULL);
		for (x = 0; x < width; x++) 
		{
			ImageBuffer[y*width+x] = line[x];
		}
	}
	free(line), line = NULL;
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
	fclose(fp);
}
/** PNG STUFF */

void CScreen::clear_screen(u32 color)
{
    int x;
    u32 *vram = g_vram_base;

    for(x = 0; x < (PSP_LINE_SIZE * PSP_SCREEN_HEIGHT); x++)
    {
		*vram++ = color; 
    }
}

void CScreen::Init()
{
	X = Y = 0;
	/* Place vram in uncached memory */
	g_vram_base = (u32 *) (0x40000000 | (u32) sceGeEdramGetAddr());
	sceDisplaySetMode(0, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
	sceDisplaySetFrameBuf((void *) g_vram_base, PSP_LINE_SIZE, PSP_PIXEL_FORMAT, 1);
	//clear_screen(bg_col);
	FRAMESIZE = PSP_LINE_SIZE * PSP_SCREEN_HEIGHT * 4;
	init = true;
}

void CScreen::DrawBackground(int iBuffer, u32 x1, u32 y1, u32 x2, u32 y2)
{
	u16* vram16;
	int bufferwidth;
	int pixelformat;
	u32 x,y;
	u32 *vram32 = NULL;
	//int unknown;
	//sceDisplayWaitVblankStart();  // if framebuf was set with PSP_DISPLAY_SETBUF_NEXTFRAME, wait until it is changed
	//sceDisplayGetFrameBuf((void**)&vram32, &bufferwidth, &pixelformat, &unknown); 
	bufferwidth = PSP_LINE_SIZE;
	pixelformat = PSP_PIXEL_FORMAT;
	//vram32 = g_vram_base;//
	vram32 = (u32*)((char*)g_vram_base+FRAMESIZE*iBuffer);
	
	vram16 = (u16*) vram32;
	for (y = y1; y < y2; y++) {
		for (x = x1; x < x2; x++) {
			u32 color32 = m_ImageBuffer[y*PSP_SCREEN_WIDTH+x];
			u16 color16;
			int r = color32 & 0xff;
			int g = (color32 >> 8) & 0xff;
			int b = (color32 >> 16) & 0xff;
			switch (pixelformat) {
				case PSP_DISPLAY_PIXEL_FORMAT_565:
					color16 = (r >> 3) | ((g >> 2) << 5) | ((b >> 3) << 11);
					vram16[x + y * bufferwidth] = color16;
					break;
				case PSP_DISPLAY_PIXEL_FORMAT_5551:
					color16 = (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10);
					vram16[x + y * bufferwidth] = color16;
					break;
				case PSP_DISPLAY_PIXEL_FORMAT_4444:
					color16 = (r >> 4) | ((g >> 4) << 4) | ((b >> 4) << 8);
					vram16[x + y * bufferwidth] = color16;
					break;
				case PSP_DISPLAY_PIXEL_FORMAT_8888:
					color32 = r | (g << 8) | (b << 16);
					vram32[x + y * bufferwidth] = color32;
					break;
			}
		}
	}
}

void CScreen::SetFrameBuffer(int iBuffer)
{
	sceDisplaySetFrameBuf((u32*)((char*)g_vram_base+FRAMESIZE*iBuffer), PSP_LINE_SIZE, PSP_PIXEL_FORMAT, 1);
}


void CScreen::SetBackgroundImage(char *strImage)
{
	
	if (strImage)
	{
		if (m_ImageBuffer == NULL)
		{
			m_ImageBuffer = (u32*)malloc(PSP_SCREEN_WIDTH*PSP_SCREEN_WIDTH*sizeof(u32));
		}
		
		if (m_ImageBuffer)
		{
			if (m_strImage)
			{
				free(m_strImage), m_strImage = NULL;
			}
			m_strImage = strdup(strImage);
			if (m_strImage)
			{
				LoadImage(m_strImage, m_ImageBuffer);
			}
		}
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
	u32 *vram = (u32*)((char*)g_vram_base+FRAMESIZE*iBuffer);
	u32 *pixel = NULL;
	for (int y = 0; y < 100; y++)
	{
		for (int x = 0; x < 100 /*PSP_LINE_SIZE*/; x++)
		{
			pixel = vram + PSP_LINE_SIZE*y + x;
			*pixel = *pixel * 2;
		}
	}
	
}

void CScreen::Plot(int iBuffer, int x, int y, int color)
{
	u32 *vram = (u32*)((char*)g_vram_base+FRAMESIZE*iBuffer);
	u32 *pixel = vram + PSP_LINE_SIZE*y + x;
	//*pixel = color;//*pixel & 0xAAAAAAAA;//color;
	*pixel = *pixel & 0xAAAAAAAA;//color;
}

int CScreen::Peek(int iBuffer, int x, int y)
{
	u32 *vram = (u32*)((char*)g_vram_base+FRAMESIZE*iBuffer);
	u32 *pixel = vram + PSP_LINE_SIZE*y + x;
	return *pixel;
}

void CScreen::Line(int iBuffer, int x1, int y1, int x2, int y2)
{
	for (int x = x1;x < x2; x++)
	{
		for (int y = y1;y < y2; y++)
		{
			Plot(iBuffer, x, y, 0xFFFFFFFF);
		}
	}
}

extern u8 msx[];

void CScreen::PutChar(int iBuffer, int x, int y, u32 color, u8 ch, bool do_background)
{
	int 	i,j;
	u8	*font;
	u32  pixel;
	u32 *vram_ptr;
	u32 *vram;
	
	if(false == init)
	{
	   return;
	}
	
	vram = (u32*)((char*)g_vram_base+FRAMESIZE*iBuffer) + x;
	vram += (y * PSP_LINE_SIZE);
	
	font = &msx[ (int)ch * 8];
	for (i=0; i < 8; i++, font++)
	{
		vram_ptr  = vram;
		for (j=0; j < 8; j++)
		{
			if (m_ImageBuffer)
			{
				if ((*font & (128 >> j)))
					*vram_ptr++ = color;
				else
				{
					if (true == do_background)
					{
						*vram_ptr++ = m_ImageBuffer[(y+i)*PSP_SCREEN_WIDTH+(x+j)]; /** If bg image buffer exists, we use that as bg */
					}
					else
					{
						vram_ptr++;
					}
				}
			}
			else
			{
				if ((*font & (128 >> j)))
					pixel = color;
				else
					pixel = bg_col;
				*vram_ptr++ = pixel; 
			}
			
		}
		vram += PSP_LINE_SIZE;
	}
}

void CScreen::PutCharWithOutline(int iBuffer, int x, int y, u32 bg_color, u32 fg_color, u8 ch)
{
	/** y */	
	PutChar(iBuffer, x,   y, bg_color, ch, false);
	PutChar(iBuffer, x+1, y, bg_color, ch, false);	
	PutChar(iBuffer, x+2, y, bg_color, ch, false);
	/** y + 2 */
	PutChar(iBuffer, x,   y+2, bg_color, ch, false);
	PutChar(iBuffer, x+1, y+2, bg_color, ch, false);	
	PutChar(iBuffer, x+2, y+2, bg_color, ch, false);
	/** y + 1 */
	PutChar(iBuffer, x,   y+1, bg_color, ch, false);
	PutChar(iBuffer, x+2, y+1, bg_color, ch, false);
	PutChar(iBuffer, x+1, y+1, fg_color, ch, false);	
}

void CScreen::PutCharWithShadow(int iBuffer, int x, int y, u32 bg_color, u32 fg_color, u8 ch)
{
	/** x+1,y+1 */
	PutChar(iBuffer, x+1, y+1, bg_color, ch, false);
	/** x,y */
	PutChar(iBuffer, x, y, fg_color, ch, false);	
}


void CScreen::PrintText(int iBuffer, int pixel_x, int pixel_y, int color, char *string)
{
	int i = 0;
	char c;

	for (;;i++)
	{
		c = string[i];
		if (0 == c || pixel_x > PSP_SCREEN_WIDTH || pixel_y > PSP_SCREEN_HEIGHT)
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
				}
				pixel_x+=m_FontWidth;
		}
	}
}

void CScreen::SetXY(int x, int y)
{
	if( x<MX && x>=0 ) X=x;
	if( y<MY && y>=0 ) Y=y;
}

int CScreen::GetX()
{
	return X;
}

int CScreen::GetY()
{
	return Y;
}

void CScreen::ShowBackgroundPng(u32 x1, u32 y1, u32 x2, u32 y2)
{
	u16* vram16;
	int bufferwidth;
	int pixelformat;
	u32 x,y;
	u32 *vram32 = NULL;
	int unknown;
	sceDisplayWaitVblankStart();  // if framebuf was set with PSP_DISPLAY_SETBUF_NEXTFRAME, wait until it is changed
	sceDisplayGetFrameBuf((void**)&vram32, &bufferwidth, &pixelformat, &unknown); 
	//vram32 = g_vram_base;//
	vram16 = (u16*) vram32;
	for (y = y1; y < y2; y++) {
		for (x = x1; x < x2; x++) {
			u32 color32 = m_ImageBuffer[y*PSP_SCREEN_WIDTH+x];
			u16 color16;
			int r = color32 & 0xff;
			int g = (color32 >> 8) & 0xff;
			int b = (color32 >> 16) & 0xff;
			switch (pixelformat) {
				case PSP_DISPLAY_PIXEL_FORMAT_565:
					color16 = (r >> 3) | ((g >> 2) << 5) | ((b >> 3) << 11);
					vram16[x + y * bufferwidth] = color16;
					break;
				case PSP_DISPLAY_PIXEL_FORMAT_5551:
					color16 = (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10);
					vram16[x + y * bufferwidth] = color16;
					break;
				case PSP_DISPLAY_PIXEL_FORMAT_4444:
					color16 = (r >> 4) | ((g >> 4) << 4) | ((b >> 4) << 8);
					vram16[x + y * bufferwidth] = color16;
					break;
				case PSP_DISPLAY_PIXEL_FORMAT_8888:
					color32 = r | (g << 8) | (b << 16);
					vram32[x + y * bufferwidth] = color32;
					break;
			}
		}
	}
}

void CScreen::Clear()
{
#if 0	
	int y;

	if(!init)
	{
		return;
	}

	if (m_strImage && m_ImageBuffer)
	{
		ShowBackgroundPng(0,0, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
		SetXY(0,0);
	}
	else
	{
		for(y=0;y<MY;y++)
			ClearLine(y);
		SetXY(0,0);
		clear_screen(bg_col);
	}
#endif
}
