/* 
	Texture cache manager for the Sony PSP.
	Copyright (C) 2005 Jesper Sandberg


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
#include <Logging.h>

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspge.h>

#include "jsaTextureCache.h"


bool jsaTextureCache::jsaTCacheStoreTexture(int ID, jsaTextureInfo *texture_info, void *tbuffer)
{
	bool		ret_value = false;
	unsigned long	texture_address;
	jsaTextureItem	Texture;
	unsigned long	tsize;
	float		bytes_pr_pixel;

	bytes_pr_pixel	= jsaTCacheTexturePixelSize(texture_info->format);
	tsize		= (unsigned long)(bytes_pr_pixel * texture_info->width * texture_info->height);

	texture_address = (unsigned long)jsaVRAMManager::jsaVRAMManagerMalloc(tsize);

	if (texture_address)
	{
		/* Add texture to list */
		Texture.ID	= ID;
		Texture.format	= texture_info->format;
		Texture.width	= texture_info->width;
		Texture.height	= texture_info->height;
		Texture.offset	= texture_address;
		Texture.swizzle	= texture_info->swizzle;
		m_TextureList.push_back(Texture);

		/* Upload texture to VRAM */
		if (texture_info->swizzle)
		{
			jsaTCacheSwizzleUpload((unsigned char *)texture_address, (unsigned char *)tbuffer, (int)(texture_info->width * bytes_pr_pixel), texture_info->height);
		}
		else
		{
			memcpy((void *)texture_address, tbuffer, tsize);
		}
		ret_value 	= true;
	}
	return ret_value;
}

bool jsaTextureCache::jsaTCacheSetTexture(int ID)
{
	list<jsaTextureItem>::iterator 	TextureIterator;
	bool				found = false;

	if (m_TextureList.size() > 0)
	{
		for (TextureIterator = m_TextureList.begin() ; TextureIterator != m_TextureList.end() ; TextureIterator++)
		{
			if ((*TextureIterator).ID == ID)
			{
				/* setup texture */
				if ((*TextureIterator).swizzle)
				{
					sceGuTexMode((*TextureIterator).format,0,0,GU_TRUE);
				}
				else
				{
					sceGuTexMode((*TextureIterator).format,0,0,GU_FALSE);
				}
				sceGuTexImage(0,(*TextureIterator).width, (*TextureIterator).height,(*TextureIterator).width, (void *)((*TextureIterator).offset));
				found = true;
			}
		}
	}
	return found;
}

float jsaTextureCache::jsaTCacheTexturePixelSize(int format)
{
	float	size;

	switch (format)
		{
		case	GU_PSM_5650:
			size = 2;
			break;
		case	GU_PSM_5551:
			size = 2;
			break;
		case	GU_PSM_4444:
			size = 2;
			break;
		case	GU_PSM_8888:
			size = 4;
			break;
		case	GU_PSM_T4:
			size = 0.5;
			break;
		case	GU_PSM_T8:
			size = 1;
			break;
		default:
			size = 0;
			break;
		}

	return size;
}

/* This code is originally done by chp from ps2dev.org. */
void jsaTextureCache::jsaTCacheSwizzleUpload(unsigned char *dest, unsigned char *source, int width, int height)
{
	int i,j;
	int rowblocks = (width / 16);
 
	for (j = 0; j < height; ++j)
	{
		for (i = 0; i < width; ++i)
		{
			unsigned int blockx = i / 16;
			unsigned int blocky = j / 8;
 
			unsigned int x = (i - blockx*16);
			unsigned int y = (j - blocky*8);
			unsigned int block_index = blockx + ((blocky) * rowblocks);
			unsigned int block_address = block_index * 16 * 8;
 
			dest[block_address + x + y * 16] = source[i+j*width];
		}
	}
}
