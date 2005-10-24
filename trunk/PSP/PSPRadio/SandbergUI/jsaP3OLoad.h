/* 
	P3O Loader for the Sony PSP.
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
#ifndef _JSAP3OLOAD_
#define _JSAP3OLOAD_

#include <list>
using namespace std;

class jsaP3OLoad
{
public:

	enum jsaP3OErrors
	{
		JSAP3O_ERROR_OK,
		JSAP3O_ERROR_LOAD,
		JSAP3O_ERROR_VERSION,
		JSAP3O_ERROR_MEMORY,
	};

	typedef struct
	{
		unsigned int	color;
		float		nx;
		float		ny;
		float		nz;
		float		x;
		float		y;
		float		z;
	} jsaP3OVertex;

	typedef struct
	{
		unsigned short	p0;
		unsigned short	p1;
		unsigned short	p2;
	} jsaP3OFace;

	typedef struct jsaP3OInfo
	{
		int		vertice_count;
		int		face_count;
		jsaP3OVertex	*vertices;
		jsaP3OFace	*faces;
	};


public:
	jsaP3OLoad() {};
	~jsaP3OLoad() {};

	int jsaP3OLoadFile(char *objectfile, jsaP3OLoad::jsaP3OInfo *object);

private:
	typedef struct
	{
		unsigned int id;
		unsigned int vertices;
		unsigned int faces;
		unsigned int flags;
	} jsaP3OHeader;

};

#endif
