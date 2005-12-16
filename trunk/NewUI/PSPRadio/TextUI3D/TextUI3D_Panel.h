/* 
	PSPRadio / Music streaming client for the PSP. (Initial Release: Sept. 2005)
	PSPRadio Copyright (C) 2005 Rafael Cabezas a.k.a. Raf
	TextUI3D Copyright (C) 2005 Jesper Sandberg & Raf

	
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

#ifndef _TEXTUI3D_PANEL_
#define _TEXTUI3D_PANEL_

using namespace std;


class CTextUI3D_Panel
{

public:
	typedef struct FrameTextures
		{
		int	width, height;
		int	corner_ul;
		int	corner_ur;
		int	corner_ll;
		int	corner_lr;
		int	frame_t;
		int	frame_b;
		int	frame_l;
		int	frame_r;
		int	fill;
		};

public:
	CTextUI3D_Panel();
	~CTextUI3D_Panel();

	void SetPosition(int x, int y, int z);
	void SetSize(int width, int height);
	void SetFrameTexture(FrameTextures &textures);

private:

	typedef struct Vertex
	{
		float u, v;
		unsigned int color;
		float x,y,z;
	};

private:
	void UpdateVertexArray();

private:
	int				m_xpos, m_ypos, m_zpos;
	int				m_width, m_height;

	FrameTextures	m_frametextures;
	Vertex			*m_vertex_array;
};

#endif
