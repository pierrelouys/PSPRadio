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


jsaTextureCache::jsaTextureCache()
{
	m_vram_start	= (unsigned long)sceGeEdramGetAddr();
	m_vram_size	= (unsigned long)sceGeEdramGetSize();
	m_vram_free	= 0;
	m_vram_offset	= 0;
	m_systemoffset	= 0;
	m_initialized	= false;
}

jsaTextureCache::~jsaTextureCache()
{
}

void jsaTextureCache::jsaTCacheInit(unsigned long buffersize)
{
	m_systemoffset	= buffersize;
	m_vram_free	= m_vram_size  - m_systemoffset;
	m_vram_offset	= m_vram_start + m_systemoffset;
	m_initialized	= true;
}

bool jsaTextureCache::jsaTCacheStoreTexture(int ID, jsaTextureInfo *texture_info, void *tbuffer)
{
	bool		ret_value = false;
	jsaTextureItem	Texture;
	unsigned long	tsize  = jsaTCacheTextureSize(texture_info->format, texture_info->width, texture_info->height);

	if (tsize <= m_vram_free)
	{
		/* Add texture to list */
		Texture.ID	= ID;
		Texture.format	= texture_info->format;
		Texture.width	= texture_info->width;
		Texture.height	= texture_info->height;
		Texture.offset	= m_vram_offset;
		m_TextureList.push_back(Texture);

		/* Upload texture to VRAM */
/*		Why doesn't this work ???
		sceGuCopyImage(	texture_info->format,
				texture_info->x,
				texture_info->y,
				texture_info->width,
				texture_info->height,
				texture_info->source_width,
				tbuffer,
				0,
				0,
				texture_info->width,
				(void*)(m_vram_offset));
*/
		memcpy((void *)m_vram_offset, tbuffer, tsize);
//		sceGuTexFlush();
//		sceGuTexSync();

		/* Update internal VRAM pointers */
		m_vram_offset	+= tsize;
		m_vram_free 	-= tsize;
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
				sceGuTexMode((*TextureIterator).format,0,0,0);
				sceGuTexImage(0,(*TextureIterator).width, (*TextureIterator).height,(*TextureIterator).width, (void *)((*TextureIterator).offset));
				found = true;
			}
		}
	}
	return found;
}

unsigned long jsaTextureCache::jsaTCacheTextureSize(int format, int width, int height)
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

	return (int)(size*width*height);
}
