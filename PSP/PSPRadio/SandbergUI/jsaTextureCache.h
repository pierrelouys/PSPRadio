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
#ifndef _JSATEXTURECACHE_
#define _JSATEXTURECACHE_

#include <list>
using namespace std;

class jsaTextureCache
{
public:
	typedef struct jsaTextureInfo
	{
		int	format;
		int	x;
		int	y;
		int	width;
		int	height;
		int	source_width;
	};

public:
	jsaTextureCache();
	~jsaTextureCache();

	void jsaTCacheInit(unsigned long buffersize);
	bool jsaTCacheStoreTexture(int ID, jsaTextureInfo *texture_info, void *tbuffer);
	bool jsaTCacheSetTexture(int ID);

private:
	struct jsaTextureItem
	{
		int		ID;		/* The ID for the texture. Used for reference later	*/
		int		format;		/* The format of the texture (i.e. GU_PSM_8888)		*/
		int		width;		/* Width of the texture.				*/
		int		height;		/* Height of the texture.				*/
		unsigned long	offset;		/* Offset into VRAM where the texture is loaded.	*/
	};

private:
	unsigned long jsaTCacheTextureSize(int format, int width, int height);

private:
	list<jsaTextureItem> m_TextureList;

	bool m_initialized;
	unsigned long m_vram_start;
	unsigned long m_vram_size;
	unsigned long m_vram_free;
	unsigned long m_vram_offset;
	unsigned long m_systemoffset;
};

#endif
