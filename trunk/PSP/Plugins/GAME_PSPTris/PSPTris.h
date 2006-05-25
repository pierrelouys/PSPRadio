/*
	PSPTris - The game
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

#ifndef _PSPTRIS_H_
#define _PSPTRIS_H_

#define APP_NAME		"PSPTRIS VER. 0.9a"

	struct NCVertex
	{
		float u, v;
		unsigned int color;
		float x,y,z;
	};

	struct NVertex
	{
		unsigned int color;
		float nx,ny,nz;
		float x,y,z;
	};

	struct CVertex
	{
		unsigned int color;
		float x,y,z;
	};

	struct Vertex
	{
		float x,y,z;
	};

	struct Face
	{
		unsigned short p1, p2, p3;
	};

#endif /* _PSPTRIS_H_ */
